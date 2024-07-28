#include <std_include.hpp>
#include "gsc_funcs.hpp"
#include "gsc_custom.hpp"
#include "dvars.hpp"
#include "hashes.hpp"
#include "command.hpp"
#include "loader/component_loader.hpp"

#include "definitions/xassets.hpp"
#include "definitions/game.hpp"

#include <utilities/io.hpp>
#include <utilities/hook.hpp>

namespace mods {
	// GSC File magic (8 bytes)
	constexpr uint64_t gsc_magic = 0x36000A0D43534780;
	// Serious' GSIC File Magic (4 bytes)
	constexpr const char* gsic_magic = "GSIC";

	constexpr const char* mod_metadata_file = "metadata.json";
	std::filesystem::path mod_dir = "project-bo4/mods";

	namespace {
		struct raw_file
		{
			xassets::raw_file_header header{};

			std::string data{};

			auto* get_header()
			{
				header.buffer = data.data();
				header.size = (uint32_t)data.length();

				return &header;
			}
		};
		struct scriptparsetree
		{
			xassets::scriptparsetree_header header{};

			std::string data{};
			size_t gsic_header_size{};
			std::unordered_set<uint64_t> hooks{};
			gsc_custom::gsic_info gsic{};

			auto* get_header()
			{
				header.buffer = reinterpret_cast<game::GSC_OBJ*>(data.data());
				header.size = (uint32_t)data.length();

				for (gsc_custom::gsic_detour& detour : gsic.detours)
				{
					detour.fixup_function = header.buffer->magic + detour.fixup_offset;
				}

				return &header;
			}

			bool can_read_gsic(size_t bytes)
			{
				return data.length() >= gsic_header_size + bytes;
			}

			bool load_gsic()
			{
				byte* ptr = (byte*)data.data();

				if (!can_read_gsic(4) || memcmp(gsic_magic, ptr, 4))
				{
					return true; // not a gsic file
				}
				gsic_header_size += 4;

				if (!can_read_gsic(4))
				{
					logger::write(logger::LOG_TYPE_ERROR, "can't read gsic fields");
					return false;
				}
				int32_t fields = *reinterpret_cast<int32_t*>(ptr + gsic_header_size);
				gsic_header_size += 4;

				for (size_t i = 0; i < fields; i++)
				{
					if (!can_read_gsic(4))
					{
						logger::write(logger::LOG_TYPE_ERROR, "can't read gsic field type");
						return false;
					}

					int32_t field_type = *reinterpret_cast<int32_t*>(ptr + gsic_header_size);
					gsic_header_size += 4;

					switch (field_type)
					{
					case gsc_custom::gsic_field_type::GSIC_FIELD_DETOUR:
					{
						// detours
						if (!can_read_gsic(4))
						{
							logger::write(logger::LOG_TYPE_ERROR, "can't read gsic detours count");
							return false;
						}
						int32_t detour_count = *reinterpret_cast<int32_t*>(ptr + gsic_header_size);
						gsic_header_size += 4;


						if (!can_read_gsic(detour_count * 256ull))
						{
							logger::write(logger::LOG_TYPE_ERROR, "can't read detours");
							return false;
						}

						for (size_t j = 0; j < detour_count; j++)
						{
							gsc_custom::gsic_detour& detour = gsic.detours.emplace_back();

							detour.fixup_name = *reinterpret_cast<uint32_t*>(ptr + gsic_header_size);
							detour.replace_namespace = *reinterpret_cast<uint32_t*>(ptr + gsic_header_size + 4);
							detour.replace_function = *reinterpret_cast<uint32_t*>(ptr + gsic_header_size + 8);
							detour.fixup_offset = *reinterpret_cast<uint32_t*>(ptr + gsic_header_size + 12);
							detour.fixup_size = *reinterpret_cast<uint32_t*>(ptr + gsic_header_size + 16);
							detour.target_script = *reinterpret_cast<uint64_t*>(ptr + gsic_header_size + 20);

							logger::write(logger::LOG_TYPE_DEBUG, std::format(
								"read detour {:x} : namespace_{:x}<script_{:x}>::function_{:x} / offset={:x}+{:x}",
								detour.fixup_name, detour.replace_namespace, detour.target_script, detour.replace_function,
								detour.fixup_offset, detour.fixup_size
							));

							gsic_header_size += 256;
						}
					}
					break;
					default:
						logger::write(logger::LOG_TYPE_ERROR, "bad gsic field type {}", field_type);
						return false;
					}
				}

				// we need to remove the header to keep the alignment
				data = data.substr(gsic_header_size, data.length() - gsic_header_size);

				return true;
			}
		};
		struct lua_file
		{
			xassets::lua_file_header header{};

			std::unordered_set<uint64_t> hooks{};
			uint64_t noext_name{};
			std::unordered_set<uint64_t> hooks_post{};
			std::string data{};

			auto* get_header()
			{
				header.buffer = reinterpret_cast<byte*>(data.data());
				header.size = (uint32_t)data.length();

				return &header;
			}
		};
		struct string_table_file
		{
			xassets::stringtable_header header{};

			std::string data{};
			std::vector<xassets::stringtable_cell> cells{};

			auto* get_header()
			{
				header.values = cells.data();

				return &header;
			}
		};
		struct localize
		{
			xassets::localize_entry_header header{};

			std::string str{};

			auto* get_header()
			{
				header.string = str.data();

				return &header;
			}

		};
		struct cache_entry
		{
			game::BO4_AssetRef_t name{};
			xassets::BGCacheTypes type{};
			std::unordered_set<game::eModes> hooks_modes{};
			std::unordered_set<uint64_t> hooks_map{};
			std::unordered_set<uint64_t> hooks_gametype{};
		};


		class mod_storage
		{
		public:
			std::mutex load_mutex{};
			std::vector<char*> allocated_strings{};
			std::vector<scriptparsetree> gsc_files{};
			std::vector<raw_file> raw_files{};
			std::vector<lua_file> lua_files{};
			std::vector<string_table_file> csv_files{};
			std::vector<localize> localizes{};
			std::vector<cache_entry> cache_entries{};
			std::unordered_map<int64_t, int64_t> assets_redirects[xassets::ASSET_TYPE_COUNT] = {};
			std::vector<xassets::bg_cache_info_def> custom_cache_entries{};

			xassets::bg_cache_info custom_cache
			{
				.name
				{
					.hash = (int64_t)fnv1a::generate_hash("shield_cache") // 2c4f76fcf5cfbebd
				}
			};

			~mod_storage()
			{
				for (char* str : allocated_strings)
				{
					delete str;
				}
			}

			void clear()
			{
				// clear previously loaded files
				raw_files.clear();
				gsc_files.clear();
				lua_files.clear();
				csv_files.clear();
				localizes.clear();
				cache_entries.clear();

				for (auto& redirect : assets_redirects)
				{
					redirect.clear();
				}

				for (char* str : allocated_strings)
				{
					delete str;
				}
				allocated_strings.clear();
			}

			char* allocate_string(const std::string& string)
			{
				char* str = new char[string.length() + 1];
				memcpy(str, string.c_str(), string.length() + 1);

				allocated_strings.emplace_back(str);

				return str;
			}

			void sync_cache_entries()
			{
				std::lock_guard lg{ load_mutex };
				custom_cache.defCount = 0;
				custom_cache.def = nullptr;
				custom_cache_entries.clear();

				if (!cache_entries.size())
				{
					return; // nothing to sync
				}

				game::dvar_t* sv_mapname = dvars::find_dvar("sv_mapname");
				game::dvar_t* g_gametype = dvars::find_dvar("g_gametype");

				if (!sv_mapname || !g_gametype)
				{
					logger::write(logger::LOG_TYPE_ERROR, "Can't find bgcache dvars");
					return;
				}

				std::string mapname = dvars::get_value_string(sv_mapname, &sv_mapname->value->current);
				std::string gametype = dvars::get_value_string(g_gametype, &g_gametype->value->current);
				game::eModes mode = game::Com_SessionMode_GetMode();

				uint64_t mapname_hash = fnv1a::generate_hash(mapname.data());
				uint64_t gametype_hash = fnv1a::generate_hash(gametype.data());

				int count = 0;
				for (auto& entry : cache_entries)
				{
					if (
						entry.hooks_modes.find(mode) != entry.hooks_modes.end()
						|| entry.hooks_map.find(mapname_hash) != entry.hooks_map.end()
						|| entry.hooks_gametype.find(gametype_hash) != entry.hooks_gametype.end()
						)
					{
						auto& ref = custom_cache_entries.emplace_back();
						ref.type = entry.type;
						ref.name.hash = entry.name.hash;
						count++;
					}
				}
				custom_cache.def = custom_cache_entries.data();
				custom_cache.defCount = (int)custom_cache_entries.size();
				logger::write(logger::LOG_TYPE_DEBUG, "sync %d custom bgcache entries", count);
			}

			void* get_xasset(xassets::XAssetType type, uint64_t name)
			{
				std::lock_guard lg{ load_mutex };
				switch (type)
				{
				case xassets::ASSET_TYPE_SCRIPTPARSETREE:
				{
					auto it = std::find_if(gsc_files.begin(), gsc_files.end(), [name](const scriptparsetree& file) { return file.header.name == name; });

					if (it == gsc_files.end()) return nullptr;

					return it->get_header();
				}
				case xassets::ASSET_TYPE_RAWFILE:
				{
					auto it = std::find_if(raw_files.begin(), raw_files.end(), [name](const raw_file& file) { return file.header.name == name; });

					if (it == raw_files.end()) return nullptr;

					return it->get_header();
				}
				case xassets::ASSET_TYPE_LUAFILE:
				{
					auto it = std::find_if(lua_files.begin(), lua_files.end(), [name](const lua_file& file) { return file.header.name == name; });

					if (it == lua_files.end()) return nullptr;

					return it->get_header();
				}
				case xassets::ASSET_TYPE_STRINGTABLE:
				{
					auto it = std::find_if(csv_files.begin(), csv_files.end(), [name](const string_table_file& file) { return file.header.name == name; });

					if (it == csv_files.end()) return nullptr;

					return it->get_header();
				}
				case xassets::ASSET_TYPE_LOCALIZE_ENTRY:
				{
					auto it = std::find_if(localizes.begin(), localizes.end(), [name](const localize& file) { return file.header.name == name; });

					if (it == localizes.end()) return nullptr;

					return it->get_header();
				}
				default:
					return nullptr; // unknown resource type
				}
			}

			bool read_data_entry(rapidjson::Value& member, const char* mod_name, const std::filesystem::path& mod_path)
			{
				auto type = member.FindMember("type");

				if (type == member.MemberEnd() || !type->value.IsString())
				{
					logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a data member without a valid type", mod_name));
					return false;
				}

				const char* type_val = type->value.GetString();

				if (!_strcmpi("scriptparsetree", type_val))
				{
					auto name_mb = member.FindMember("name");
					auto path_mb = member.FindMember("path");

					if (
						name_mb == member.MemberEnd() || path_mb == member.MemberEnd()
						|| !name_mb->value.IsString() || !path_mb->value.IsString()
						)
					{
						logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad scriptparsetree def, missing/bad name or path", mod_name));
						return false;
					}

					scriptparsetree tmp{};
					std::filesystem::path path_cfg = path_mb->value.GetString();
					auto spt_path = path_cfg.is_absolute() ? path_cfg : (mod_path / path_cfg);
					tmp.header.name = fnv1a::generate_hash_pattern(name_mb->value.GetString());

					auto hooks = member.FindMember("hooks");

					if (hooks != member.MemberEnd())
					{
						// no hooks might not be an error, to replace a script for example

						if (!hooks->value.IsArray())
						{
							logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad scriptparsetree hook def, not an array for {}", mod_name, spt_path.string()));
							return false;
						}

						for (auto& hook : hooks->value.GetArray())
						{
							if (!hook.IsString())
							{
								logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad scriptparsetree hook def, not a string for {}", mod_name, spt_path.string()));
								return false;
							}

							tmp.hooks.insert(fnv1a::generate_hash_pattern(hook.GetString()));
						}
					}

					if (!utilities::io::read_file(spt_path.string(), &tmp.data))
					{
						logger::write(logger::LOG_TYPE_ERROR, std::format("can't read scriptparsetree {} for mod {}", spt_path.string(), mod_name));
						return false;
					}

					if (!tmp.load_gsic())
					{
						logger::write(logger::LOG_TYPE_ERROR, std::format("error when reading GSIC header of {} for mod {}", spt_path.string(), mod_name));
						return false;
					}

					if (tmp.gsic.detours.size())
					{
						logger::write(logger::LOG_TYPE_DEBUG, std::format("loaded {} detours", tmp.gsic.detours.size()));
					}

					if (tmp.data.length() < sizeof(game::GSC_OBJ) || *reinterpret_cast<uint64_t*>(tmp.data.data()) != gsc_magic)
					{
						logger::write(logger::LOG_TYPE_ERROR, std::format("bad scriptparsetree magic in {} for mod {}", spt_path.string(), mod_name));
						return false;
					}

					// after this point we assume that the GSC file is well formatted
					
					game::GSC_OBJ* script_obj = reinterpret_cast<game::GSC_OBJ*>(tmp.data.data());

					// fix compiler script name
					script_obj->name = tmp.header.name;

					// fix compiler custom namespace
					game::GSC_IMPORT_ITEM* imports = script_obj->get_imports();

					static uint32_t isprofilebuild_hash = gsc_funcs::canon_hash("IsProfileBuild");
					static uint32_t serious_custom_func_name_hash = gsc_funcs::canon_hash(gsc_funcs::serious_custom_func_name);
					for (size_t imp = 0; imp < script_obj->imports_count; imp++)
					{
						if (imports->name == isprofilebuild_hash && imports->param_count != 0)
						{
							// compiler:: calls, replace the call to our custom function
							imports->name = serious_custom_func_name_hash;
						}

						imports = reinterpret_cast<game::GSC_IMPORT_ITEM*>((uint32_t*)&imports[1] + imports->num_address);
					}

					logger::write(logger::LOG_TYPE_DEBUG, std::format("mod {}: loaded scriptparsetree {} -> {:x}", mod_name, spt_path.string(), tmp.header.name));
					gsc_files.emplace_back(tmp);
				}
				else if (!_strcmpi("rawfile", type_val))
				{
					auto name_mb = member.FindMember("name");
					auto path_mb = member.FindMember("path");

					if (
						name_mb == member.MemberEnd() || path_mb == member.MemberEnd()
						|| !name_mb->value.IsString() || !path_mb->value.IsString()
						)
					{
						logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad rawfile def, missing/bad name or path", mod_name));
						return false;
					}

					raw_file tmp{};
					std::filesystem::path path_cfg = path_mb->value.GetString();
					auto raw_file_path = path_cfg.is_absolute() ? path_cfg : (mod_path / path_cfg);
					tmp.header.name = fnv1a::generate_hash_pattern(name_mb->value.GetString());

					if (!utilities::io::read_file(raw_file_path.string(), &tmp.data))
					{
						logger::write(logger::LOG_TYPE_ERROR, std::format("can't read raw file {} for mod {}", raw_file_path.string(), mod_name));
						return false;
					}

					logger::write(logger::LOG_TYPE_DEBUG, std::format("mod {}: loaded raw file {} -> {:x}", mod_name, raw_file_path.string(), tmp.header.name));
					raw_files.emplace_back(tmp);
				}
				else if (!_strcmpi("localizeentry", type_val))
				{
					auto name_mb = member.FindMember("name");
					auto value_mb = member.FindMember("value");

					if (
						name_mb == member.MemberEnd() || value_mb == member.MemberEnd()
						|| !name_mb->value.IsString() || !value_mb->value.IsString()
						)
					{
						logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad localized entry def, missing/bad name or value", mod_name));
						return false;
					}

					localize tmp{};
					tmp.str = value_mb->value.GetString();
					tmp.header.name = fnv1a::generate_hash_pattern(name_mb->value.GetString());

					logger::write(logger::LOG_TYPE_DEBUG, std::format("mod {}: loaded localized entry {:x}", mod_name, tmp.header.name));
					localizes.emplace_back(tmp);
				}
				else if (!_strcmpi("luafile", type_val))
				{
					auto name_mb = member.FindMember("name");
					auto path_mb = member.FindMember("path");

					if (
						name_mb == member.MemberEnd() || path_mb == member.MemberEnd()
						|| !name_mb->value.IsString() || !path_mb->value.IsString()
						)
					{
						logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad luafile def, missing/bad name or path", mod_name));
						return false;
					}

					lua_file tmp{};
					std::filesystem::path path_cfg = path_mb->value.GetString();
					auto lua_file_path = path_cfg.is_absolute() ? path_cfg : (mod_path / path_cfg);
					// it injects the name without the .lua and load the name with the .lua, good luck to replace with an unknown hash!
					tmp.noext_name = fnv1a::generate_hash_pattern(name_mb->value.GetString());
					tmp.header.name = fnv1a::generate_hash(".lua", tmp.noext_name);

					auto hooks = member.FindMember("hooks_pre");

					if (hooks != member.MemberEnd())
					{
						// no hooks might not be an error, to replace a script for example

						if (!hooks->value.IsArray())
						{
							logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad luafile pre hook def, not an array for {}", mod_name, lua_file_path.string()));
							return false;
						}

						for (auto& hook : hooks->value.GetArray())
						{
							if (!hook.IsString())
							{
								logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad luafile pre hook def, not a string for {}", mod_name, lua_file_path.string()));
								return false;
							}

							tmp.hooks.insert(fnv1a::generate_hash_pattern(hook.GetString()));
						}
					}

					auto hooks_post = member.FindMember("hooks_post");
					
					if (hooks_post != member.MemberEnd())
					{
						if (!hooks_post->value.IsArray())
						{
							logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad luafile post hook def, not an array for {}", mod_name, lua_file_path.string()));
							return false;
						}

						for (auto& hook : hooks_post->value.GetArray())
						{
							if (!hook.IsString())
							{
								logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad luafile post hook def, not a string for {}", mod_name, lua_file_path.string()));
								return false;
							}

							tmp.hooks_post.insert(fnv1a::generate_hash_pattern(hook.GetString()));
						}
					}

					if (!utilities::io::read_file(lua_file_path.string(), &tmp.data))
					{
						logger::write(logger::LOG_TYPE_ERROR, std::format("can't read lua file {} for mod {}", lua_file_path.string(), mod_name));
						return false;
					}

					logger::write(logger::LOG_TYPE_DEBUG, std::format("mod {}: loaded lua file {} -> x64:{:x}.lua ({:x})", mod_name, lua_file_path.string(), tmp.noext_name, tmp.header.name));
					lua_files.emplace_back(tmp);
				}
				else if (!_strcmpi("stringtable", type_val))
				{
					auto name_mb = member.FindMember("name");
					auto path_mb = member.FindMember("path");

					if (
						name_mb == member.MemberEnd() || path_mb == member.MemberEnd()
						|| !name_mb->value.IsString() || !path_mb->value.IsString()
						)
					{
						logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad stringtable def, missing/bad name or path", mod_name));
						return false;
					}

					string_table_file tmp{};
					std::filesystem::path path_cfg = path_mb->value.GetString();
					auto stringtable_file_path = path_cfg.is_absolute() ? path_cfg : (mod_path / path_cfg);
					tmp.header.name = fnv1a::generate_hash_pattern(name_mb->value.GetString());

					if (!utilities::io::read_file(stringtable_file_path.string(), &tmp.data))
					{
						logger::write(logger::LOG_TYPE_ERROR, std::format("can't read stringtable file {} for mod {}", stringtable_file_path.string(), mod_name));
						return false;
					}

					rapidcsv::Document doc{};

					std::stringstream stream{ tmp.data };

					auto separator_mb = member.FindMember("separator");

					char sep = ',';

					if (separator_mb != member.MemberEnd())
					{
						if (!separator_mb->value.IsString())
						{
							logger::write(logger::LOG_TYPE_ERROR, std::format("bad separator type for stringtable file {} for mod {}", stringtable_file_path.string(), mod_name));
							return false;
						}
						const char* sepval = separator_mb->value.GetString();

						if (!sepval[0] || sepval[1])
						{
							logger::write(logger::LOG_TYPE_ERROR, std::format("bad separator for stringtable file {} for mod {}, a separator should contain only one character", stringtable_file_path.string(), mod_name));
							return false;
						}

						sep = *sepval;
					}

					doc.Load(stream, rapidcsv::LabelParams(-1, -1), rapidcsv::SeparatorParams(sep));

					size_t rows_count_tmp = doc.GetRowCount();
					tmp.header.rows_count = rows_count_tmp != 0 ? (int32_t)(rows_count_tmp - 1) : 0;
					tmp.header.columns_count = (int32_t)doc.GetColumnCount();

					std::vector<xassets::stringtable_cell_type> cell_types{};

					for (size_t i = 0; i < tmp.header.columns_count; i++)
					{
						// read cell types
						const std::string cell = doc.GetCell<std::string>(i, 0);

						xassets::stringtable_cell_type cell_type = xassets::STC_TYPE_STRING;
						if (cell == "undefined")
						{
							cell_type = xassets::STC_TYPE_UNDEFINED;
						}
						else if (cell == "string")
						{
							cell_type = xassets::STC_TYPE_STRING;
						}
						else if (cell == "int")
						{
							cell_type = xassets::STC_TYPE_INT;
						}
						else if (cell == "float")
						{
							cell_type = xassets::STC_TYPE_FLOAT;
						}
						else if (cell == "hash")
						{
							cell_type = xassets::STC_TYPE_HASHED2;
						}
						else if (cell == "hash7")
						{
							cell_type = xassets::STC_TYPE_HASHED7;
						}
						else if (cell == "hash8")
						{
							cell_type = xassets::STC_TYPE_HASHED8;
						}
						else if (cell == "bool")
						{
							cell_type = xassets::STC_TYPE_BOOL;
						}
						else
						{
							logger::write(logger::LOG_TYPE_ERROR, std::format("mod {} : can't read stringtable {} type of column {} : '{}'", mod_name, stringtable_file_path.string(), i, cell));
							return false;
						}

						cell_types.emplace_back(cell_type);
					}

					for (size_t row = 1; row <= tmp.header.rows_count; row++)
					{
						// read cells
						for (size_t column = 0; column < tmp.header.columns_count; column++)
						{
							xassets::stringtable_cell_type cell_type = cell_types[column];

							const std::string cell_str = doc.GetCell<std::string>(column, row);

							xassets::stringtable_cell& cell = tmp.cells.emplace_back();
							cell.type = cell_type;

							try
							{
								switch (cell_type)
								{
								case xassets::STC_TYPE_UNDEFINED:
									cell.value.int_value = 0;
									break;
								case xassets::STC_TYPE_BOOL:
									cell.value.bool_value = cell_str == "true";
									break;
								case xassets::STC_TYPE_HASHED2:
								case xassets::STC_TYPE_HASHED7:
								case xassets::STC_TYPE_HASHED8:
									cell.value.hash_value = fnv1a::generate_hash_pattern(cell_str.c_str());
									break;
								case xassets::STC_TYPE_INT:
									if (cell_str.starts_with("0x"))
									{
										cell.value.int_value = std::stoull(cell_str.substr(2), nullptr, 16);
									}
									else
									{
										cell.value.int_value = std::stoll(cell_str);
									}
									break;
								case xassets::STC_TYPE_FLOAT:
									cell.value.float_value = std::stof(cell_str);
									break;
								case xassets::STC_TYPE_STRING:
									cell.value.string_value = allocate_string(cell_str);
									break;
								}
							}
							catch (const std::invalid_argument& e)
							{
								logger::write(logger::LOG_TYPE_DEBUG, std::format("mod {}: error when loading stringtable file {} : {} [line {} col {}] '{}'", mod_name, stringtable_file_path.string(), e.what(), row, column, cell_str));
								return false;
							}
						}
					}
					
					logger::write(logger::LOG_TYPE_DEBUG, std::format("mod {}: loaded stringtable file {} -> {:x} ({}x{})", mod_name, stringtable_file_path.string(), tmp.header.name, tmp.header.columns_count, tmp.header.rows_count));
					csv_files.emplace_back(tmp);
				}
				else if (!_strcmpi("hashes", type_val))
				{
					auto path_mb = member.FindMember("path");

					if (path_mb == member.MemberEnd() || !path_mb->value.IsString())
					{
						logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a hashes storage without a path", mod_name));
						return false;
					}

					auto format_mb = member.FindMember("format");
					hashes::hashes_file_format format;

					if (format_mb == member.MemberEnd() || !format_mb->value.IsString() || (format = hashes::get_format_idx(format_mb->value.GetString())) == hashes::HFF_COUNT)
					{
						logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a hashes storage without a valid format", mod_name));
						return false;
					}

					std::filesystem::path path_cfg = path_mb->value.GetString();
					auto path = path_cfg.is_absolute() ? path_cfg : (mod_path / path_cfg);

					return hashes::load_file(path, format);
				}
				else
				{
					logger::write(logger::LOG_TYPE_ERROR, std::format("mod {} is load data member with an unknown type '{}'", mod_name, type_val));
					return false;
				}

				return true;
			}

			bool read_cache_entry(rapidjson::Value& member, const char* mod_name, const std::filesystem::path& mod_path)
			{
				auto type = member.FindMember("type");

				if (type == member.MemberEnd() || !type->value.IsString())
				{
					logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a cache member without a valid type", mod_name));
					return false;
				}

				auto name = member.FindMember("name");

				if (name == member.MemberEnd() || !name->value.IsString())
				{
					logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a cache member without a valid name", mod_name));
					return false;
				}

				const char* name_val = name->value.GetString();
				const char* type_val = type->value.GetString();

				xassets::BGCacheTypes bgtype = xassets::BG_Cache_GetTypeIndex(type_val);

				if (!bgtype)
				{
					logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a cache member with a bad type: {}", mod_name, type_val));
					return false;
				}

				cache_entry tmp{};

				tmp.name.hash = fnv1a::generate_hash_pattern(name_val);
				tmp.type = bgtype;

				auto hook_map = member.FindMember("map");
				auto hook_mode = member.FindMember("mode");
				auto hook_gametype = member.FindMember("gametype");

				if (hook_map != member.MemberEnd())
				{
					if (hook_map->value.IsArray())
					{
						auto data_array = hook_map->value.GetArray();

						for (rapidjson::Value& hookmember : data_array)
						{
							if (!hookmember.IsString())
							{
								logger::write(logger::LOG_TYPE_ERROR, std::format("mod {} is containing a cache member with a bad map hook", mod_name));
								continue;
							}
							tmp.hooks_map.insert(fnv1a::generate_hash(hookmember.GetString()));
						}
					}
					else if (hook_map->value.IsString())
					{
						tmp.hooks_map.insert(fnv1a::generate_hash(hook_map->value.GetString()));
					}
					else
					{
						logger::write(logger::LOG_TYPE_ERROR, std::format("mod {} is containing a cache member with a bad map hook", mod_name));
						return false;
					}
				}

				if (hook_mode != member.MemberEnd())
				{
					if (hook_mode->value.IsArray())
					{
						auto data_array = hook_mode->value.GetArray();

						for (rapidjson::Value& hookmember : data_array)
						{
							if (!hookmember.IsString())
							{
								logger::write(logger::LOG_TYPE_ERROR, std::format("mod {} is containing a cache member with a bad mode hook", mod_name));
								continue;
							}
							game::eModes loaded = game::Com_SessionMode_GetModeForAbbreviation(hookmember.GetString());
							if (loaded == game::eModes::MODE_COUNT)
							{
								logger::write(logger::LOG_TYPE_ERROR, std::format("mod {} is containing a cache member with a bad mode hook", mod_name));
								continue;
							}
							tmp.hooks_modes.insert(loaded);
						}
					}
					else if (hook_mode->value.IsString())
					{
						game::eModes loaded = game::Com_SessionMode_GetModeForAbbreviation(hook_mode->value.GetString());
						if (loaded == game::eModes::MODE_COUNT)
						{
							logger::write(logger::LOG_TYPE_ERROR, std::format("mod {} is containing a cache member with a bad mode hook", mod_name));
							return false;
						}
						tmp.hooks_modes.insert(loaded);
					}
					else
					{
						logger::write(logger::LOG_TYPE_ERROR, std::format("mod {} is containing a cache member with a bad mode hook", mod_name));
						return false;
					}
				}

				if (hook_gametype != member.MemberEnd())
				{
					if (hook_gametype->value.IsArray())
					{
						auto data_array = hook_gametype->value.GetArray();

						for (rapidjson::Value& hookmember : data_array)
						{
							if (!hookmember.IsString())
							{
								logger::write(logger::LOG_TYPE_ERROR, std::format("mod {} is containing a cache member with a bad gametype hook", mod_name));
								continue;
							}
							tmp.hooks_gametype.insert(fnv1a::generate_hash(hookmember.GetString()));
						}
					}
					else if (hook_gametype->value.IsString())
					{
						tmp.hooks_gametype.insert(fnv1a::generate_hash(hook_gametype->value.GetString()));
					}
					else
					{
						logger::write(logger::LOG_TYPE_ERROR, std::format("mod {} is containing a cache member with a bad gametype hook", mod_name));
						return false;
					}
				}

				cache_entries.push_back(tmp);

				return true;
			}
			bool read_redirect_entry(rapidjson::Value& member, const char* mod_name, const std::filesystem::path& mod_path)
			{
				auto type = member.FindMember("type");

				if (type == member.MemberEnd() || !type->value.IsString())
				{
					logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a redirect member without a valid type", mod_name));
					return false;
				}

				auto origin = member.FindMember("origin");

				if (origin == member.MemberEnd() || !origin->value.IsString())
				{
					logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a redirect member without a valid origin", mod_name));
					return false;
				}

				auto target = member.FindMember("target");

				if (target == member.MemberEnd() || !target->value.IsString())
				{
					logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a redirect member without a valid target", mod_name));
					return false;
				}

				xassets::XAssetType assettype = xassets::DB_GetXAssetTypeIndex(type->value.GetString());

				if (assettype == xassets::ASSET_TYPE_COUNT)
				{
					logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a redirect member without a valid type: {}", mod_name, type->value.GetString()));
					return false;
				}

				int64_t from = fnv1a::generate_hash_pattern(origin->value.GetString());
				int64_t to = fnv1a::generate_hash_pattern(target->value.GetString());
				assets_redirects[assettype][from] = to;

				logger::write(logger::LOG_TYPE_DEBUG, std::format("mod {}: loaded redirect {:x} -> {:x} ({})", mod_name, from, to, xassets::DB_GetXAssetTypeName(assettype)));

				return true;
			}

			bool load_mods()
			{
				std::lock_guard lg{ load_mutex };
				clear();
				rapidjson::Document info{};
				std::string mod_metadata{};

				bool err = false;

				std::filesystem::create_directories(mod_dir);
				for (const auto& mod : std::filesystem::directory_iterator{ mod_dir })
				{
					if (!mod.is_directory()) continue; // not a directory

					std::filesystem::path mod_path = mod.path();
					std::filesystem::path mod_metadata_path = mod_path / mod_metadata_file;

					if (!std::filesystem::exists(mod_metadata_path)) continue; // doesn't contain the metadata file


					std::string filename = mod_metadata_path.string();
					if (!utilities::io::read_file(filename, &mod_metadata))
					{
						logger::write(logger::LOG_TYPE_ERROR, std::format("can't read mod metadata file '{}'", filename));
						err = true;
						continue;
					}

					info.Parse(mod_metadata);

					if (info.HasParseError()) {
						logger::write(logger::LOG_TYPE_ERROR, std::format("can't parse mod json metadata '{}'", filename));
						err = true;
						continue;
					}

					auto name_member = info.FindMember("name");

					const char* mod_name;

					if (name_member != info.MemberEnd() && name_member->value.IsString())
					{
						mod_name = name_member->value.GetString();
					}
					else
					{
						mod_name = filename.c_str();
					}
					logger::write(logger::LOG_TYPE_INFO, std::format("loading mod {}...", mod_name));

					int mod_errors = 0;
					auto data_member = info.FindMember("data");

					if (data_member != info.MemberEnd() && data_member->value.IsArray())
					{
						auto data_array = data_member->value.GetArray();

						for (rapidjson::Value& member : data_array)
						{
							if (!member.IsObject())
							{
								logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad data member", mod_name));
								mod_errors++;
								continue;
							}

							auto ignore_error_mb = member.FindMember("ignore_error");
							bool ignore_error = ignore_error_mb != member.MemberEnd() && ignore_error_mb->value.IsBool() && ignore_error_mb->value.GetBool();

							if (!read_data_entry(member, mod_name, mod_path) && !ignore_error)
							{
								mod_errors++;
							}
						}
					}
					auto cache_member = info.FindMember("cache");

					if (cache_member != info.MemberEnd() && cache_member->value.IsArray())
					{
						auto data_array = cache_member->value.GetArray();

						for (rapidjson::Value& member : data_array)
						{
							if (!member.IsObject())
							{
								logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad cache member", mod_name));
								mod_errors++;
								continue;
							}

							if (!read_cache_entry(member, mod_name, mod_path))
							{
								mod_errors++;
							}
						}
					}
					auto redirect_member = info.FindMember("redirect");

					if (redirect_member != info.MemberEnd() && redirect_member->value.IsArray())
					{
						auto redirect_array = redirect_member->value.GetArray();

						for (rapidjson::Value& member : redirect_array)
						{
							if (!member.IsObject())
							{
								logger::write(logger::LOG_TYPE_WARN, std::format("mod {} is containing a bad redirect member", mod_name));
								mod_errors++;
								continue;
							}

							if (!read_redirect_entry(member, mod_name, mod_path))
							{
								mod_errors++;
							}
						}
					}

					if (mod_errors)
					{
						logger::write(logger::LOG_TYPE_WARN, std::format("mod {} loaded with {} error{}.", mod_name, mod_errors, mod_errors > 1 ? "s" : ""));
						err = true;
					}
				}
				return err;
			}
		};

		mod_storage storage{};


		void mods_reload_f()
		{
			if (!game::Com_IsRunningUILevel())
			{
				// avoid gsc issues, but if a script is loaded in the frontend, it will still crash
				logger::write(logger::LOG_TYPE_CONSOLE, "can't load mods while in-game!"); 
				return;
			}

			if (!storage.load_mods())
			{
				logger::write(logger::LOG_TYPE_CONSOLE, "mods reloaded.");
			}
			else
			{
				logger::write(logger::LOG_TYPE_CONSOLE, "mods reloaded with errors, see logs.");
			}
		}
	}

	utilities::hook::detour db_find_xasset_header_hook;
	utilities::hook::detour db_does_xasset_exist_hook;
	utilities::hook::detour scr_gsc_obj_link_hook;
	utilities::hook::detour hksl_loadfile_hook;

	void* db_find_xasset_header_stub(xassets::XAssetType type, game::BO4_AssetRef_t* name, bool errorIfMissing, int waitTime)
	{
		auto& redirect = storage.assets_redirects[type];

		auto replaced = redirect.find(name->hash & 0x7FFFFFFFFFFFFFFF);

		game::BO4_AssetRef_t redirected_name;
		if (replaced != redirect.end())
		{
			// replace xasset
			redirected_name.hash = replaced->second;
			redirected_name.null = 0;
			name = &redirected_name;
		}

		void* header = storage.get_xasset(type, name->hash);

		if (header)
		{
			return header; // overwrite/load custom data
		}

		return db_find_xasset_header_hook.invoke<void*>(type, name, errorIfMissing, waitTime);
	}

	bool db_does_xasset_exist_stub(xassets::XAssetType type, game::BO4_AssetRef_t* name)
	{
		auto& redirect = storage.assets_redirects[type];

		auto replaced = redirect.find(name->hash & 0x7FFFFFFFFFFFFFFF);

		game::BO4_AssetRef_t redirected_name;
		if (replaced != redirect.end())
		{
			// replace xasset
			redirected_name.hash = replaced->second;
			redirected_name.null = 0;
			name = &redirected_name;
		}

		void* header = storage.get_xasset(type, name->hash);

		if (header)
		{
			return true;
		}

		return db_does_xasset_exist_hook.invoke<bool>(type, name);
	}

	int scr_gsc_obj_link_stub(game::scriptInstance_t inst, game::GSC_OBJ* prime_obj, bool runScript)
	{
		// link the injected scripts if we find a hook, sync the gsic fields at the same time 
		// because we know the instance.
		for (auto& spt : storage.gsc_files)
		{
			if (spt.hooks.find(prime_obj->name) != spt.hooks.end())
			{
				gsc_custom::sync_gsic(inst, spt.gsic);
				int err = scr_gsc_obj_link_hook.invoke<int>(inst, spt.get_header()->buffer, runScript);

				if (err < 0)
				{
					return err; // error when linking
				}
			}
		}

		auto custom_replaced_it = std::find_if(storage.gsc_files.begin(), storage.gsc_files.end(),
			[prime_obj](scriptparsetree& e){ return e.get_header()->buffer == prime_obj; });

		if (custom_replaced_it != storage.gsc_files.end())
		{
			// replaced gsc file
			gsc_custom::sync_gsic(inst, custom_replaced_it->gsic);
		}

		return scr_gsc_obj_link_hook.invoke<int>(inst, prime_obj, runScript);
	}

	int hksl_loadfile_stub(game::lua_state* state, const char* filename)
	{

		uint64_t hash{};
		if (!storage.lua_files.empty())
		{
			hash = fnv1a::generate_hash_pattern(filename);
		}

		for (auto& lua : storage.lua_files)
		{
			// we need to use the hash because filename is x64:HASH or unhashed

			if (lua.hooks.find(hash) != lua.hooks.end())
			{
				std::string name = std::format("x64:{:x}.lua", lua.noext_name);
				if (!game::Lua_CoD_LoadLuaFile(state, name.c_str()))
				{
					logger::write(logger::LOG_TYPE_ERROR, std::format("error when loading hook lua {} (pre)", name));
				}
			}
		}

		int load = hksl_loadfile_hook.invoke<int>(state, filename);

		for (auto& lua : storage.lua_files)
		{
			// we need to use the hash because filename is x64:HASH or unhashed

			if (lua.hooks_post.find(hash) != lua.hooks_post.end())
			{
				std::string name = std::format("x64:{:x}.lua", lua.noext_name);
				if (!game::Lua_CoD_LoadLuaFile(state, name.c_str()))
				{
					logger::write(logger::LOG_TYPE_ERROR, std::format("error when loading hook lua {} (post)", name));
				}
			}
		}

		return load;
	}

	utilities::hook::detour bg_cache_sync_hook;

	void bg_cache_sync_stub()
	{
		storage.sync_cache_entries();

		xassets::Demo_AddBGCacheAndRegister(&storage.custom_cache, 0x16000); 
		
		// sync default
		bg_cache_sync_hook.invoke<void>();
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			storage.load_mods();

			// custom assets loading
			db_find_xasset_header_hook.create(xassets::DB_FindXAssetHeader.get(), db_find_xasset_header_stub);
			db_does_xasset_exist_hook.create(0x142EB6C90_g, db_does_xasset_exist_stub);
			scr_gsc_obj_link_hook.create(0x142748F10_g, scr_gsc_obj_link_stub);
			hksl_loadfile_hook.create(0x14375D6A0_g, hksl_loadfile_stub);
			bg_cache_sync_hook.create(0x1405CE0B0_g, bg_cache_sync_stub);

			command::add("reload_mods", mods_reload_f, "Reload the shield mods");
		}
	};
}

REGISTER_COMPONENT(mods::component)