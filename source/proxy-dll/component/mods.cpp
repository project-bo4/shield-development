#include <std_include.hpp>
#include "definitions/game.hpp"

#include "game_console.hpp"
#include "loader/component_loader.hpp"

#include <utilities/io.hpp>
#include <utilities/hook.hpp>

#include <rapidjson/document.h>

namespace mods {
	constexpr uint64_t spt_magic = 0x36000A0D43534780;
	std::filesystem::path mod_dir = "project-bo4/mods";

	namespace {
		struct raw_file
		{
			struct {
				uint64_t name{};
				uint64_t pad8{};
				uint64_t size{};
				const char* buffer{};
			} header{};

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
			struct {
				uint64_t name{};
				uint64_t pad8{};
				game::GSC_OBJ* buffer{};
				uint32_t size{};
				uint32_t pad20{};
			} header{};

			std::string data{};
			std::unordered_set<uint64_t> hooks{};

			auto* get_header()
			{
				header.buffer = reinterpret_cast<game::GSC_OBJ*>(data.data());
				header.size = (uint32_t)data.length();

				return &header;
			}
		};
		struct lua_file
		{
			struct {
				uint64_t name{};
				uint64_t pad8{};
				uint64_t size{};
				byte* buffer{};
			} header{};

			std::string data{};

			auto* get_header()
			{
				header.buffer = reinterpret_cast<byte*>(data.data());
				header.size = (uint32_t)data.length();

				return &header;
			}
		};


		class mod_storage
		{
		public:
			std::vector<scriptparsetree> gsc_files{};
			std::vector<raw_file> raw_files{};
			std::vector<lua_file> lua_files{};

			void* get_xasset(game::XAssetType type, uint64_t name)
			{
				switch (type)
				{
				case game::ASSET_TYPE_SCRIPTPARSETREE:
				{
					auto it = std::find_if(gsc_files.begin(), gsc_files.end(), [name](const scriptparsetree& file) { return file.header.name == name; });

					if (it == gsc_files.end()) return nullptr;

					return it->get_header();
				}
					break;
				case game::ASSET_TYPE_RAWFILE:
				{
					auto it = std::find_if(raw_files.begin(), raw_files.end(), [name](const raw_file& file) { return file.header.name == name; });

					if (it == raw_files.end()) return nullptr;

					return it->get_header();
				}
				case game::ASSET_TYPE_LUAFILE:
				{
					auto it = std::find_if(lua_files.begin(), lua_files.end(), [name](const lua_file& file) { return file.header.name == name; });

					if (it == lua_files.end()) return nullptr;

					return it->get_header();
				}
					break;
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
					/*
						{
							"type" : "scriptparsetree",
							"name" : "scripts/shield/test.gsc",
							"path" : "compiled.gscc",
							"hooks" : [
								"scripts/core_common/load.gsc"
							]
						}
					*/

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

					if (tmp.data.length() < 8 || *reinterpret_cast<uint64_t*>(tmp.data.data()) != spt_magic)
					{
						logger::write(logger::LOG_TYPE_ERROR, std::format("bad scriptparsetree magic in {} for mod {} (gsic not supported)", spt_path.string(), mod_name));
						return false;
					}
					
					// fix compiler script name
					reinterpret_cast<game::GSC_OBJ*>(tmp.data.data())->name = tmp.header.name;

					logger::write(logger::LOG_TYPE_DEBUG, std::format("mod {}: loaded scriptparsetree {} -> {:x}", mod_name, spt_path.string(), tmp.header.name));
					gsc_files.emplace_back(tmp);
				}
				else if (!_strcmpi("rawfile", type_val))
				{
					/*
						{
							"type" : "rawfile",
							"name" : "gamedata/shield/info.txt",
							"path" : "test.txt"
						}
					*/

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
				else if (!_strcmpi("luafile", type_val))
				{
					/*
						{
							"type" : "luafile",
							"name" : "ui/shield/mod.lua",
							"path" : "test.lua"
						}
					*/

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
					tmp.header.name = fnv1a::generate_hash_pattern(name_mb->value.GetString());

					if (!utilities::io::read_file(lua_file_path.string(), &tmp.data))
					{
						logger::write(logger::LOG_TYPE_ERROR, std::format("can't read lua file {} for mod {}", lua_file_path.string(), mod_name));
						return false;
					}

					logger::write(logger::LOG_TYPE_DEBUG, std::format("mod {}: loaded lua file {} -> {:x}", mod_name, lua_file_path.string(), tmp.header.name));
					lua_files.emplace_back(tmp);
				}
				else
				{
					logger::write(logger::LOG_TYPE_ERROR, std::format("mod {} is load data member with an unknown type '{}'", mod_name, type_val));
					return false;
				}

				return true;
			}

			void clear()
			{
				// clear previously loaded files
				raw_files.clear();
				gsc_files.clear();
				lua_files.clear();
			}
		};

		mod_storage storage{};

		bool load_mods()
		{
			storage.clear();
			rapidjson::Document info{};
			std::string mod_info{};

			bool err = false;

			std::filesystem::create_directories(mod_dir);
			for (const auto& mod : std::filesystem::directory_iterator{ mod_dir })
			{
				if (!mod.is_directory()) continue; // not a directory

				std::filesystem::path mod_path = mod.path();
				std::filesystem::path mod_config = mod_path / "mod.json";

				if (!std::filesystem::exists(mod_config)) continue; // doesn't contain the config file


				std::string filename = mod_config.string();
				if (!utilities::io::read_file(filename, &mod_info))
				{
					logger::write(logger::LOG_TYPE_ERROR, std::format("can't read mod config file '{}'", filename));
					err = true;
					continue;
				}

				info.Parse(mod_info);

				if (info.HasParseError()) {
					logger::write(logger::LOG_TYPE_ERROR, std::format("can't parse mod json config '{}'", filename));
					err = true;
					continue;
				}

				/*
				  prototype:
				  {
					  "name": "test mod",
					  "data": [
						  {
							  "type" : "scriptparsetree",
							  "name" : "scripts/shield/test.gsc",
							  "path" : "compiled.gscc",
							  "hooks" : [
								  "scripts/core_common/load.gsc"
							  ]
						  },
						  {
							  "type" : "stringtable",
							  "name" : "gamedata/shield/info.csv",
							  "path" : "test.csv"
						  },
						  {
							  "type" : "rawfile",
							  "name" : "gamedata/shield/info.txt",
							  "path" : "test.txt"
						  }
					  ]
				  }
				*/

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

						if (!storage.read_data_entry(member, mod_name, mod_path))
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

		void load_mods_cmd()
		{
			if (!load_mods())
			{
				game_console::print("mods reloaded.");
			}
			else
			{
				game_console::print("mods reloaded with errors, see logs.");
			}
		}
	}

	utilities::hook::detour db_find_xasset_header_hook;
	utilities::hook::detour scr_gsc_obj_link_hook;

	void* db_find_xasset_header_stub(game::XAssetType type, game::BO4_AssetRef_t* name, bool errorIfMissing, int waitTime)
	{
		void* header = storage.get_xasset(type, name->hash);

		if (header)
		{
			return header; // overwrite/load custom data
		}

		return db_find_xasset_header_hook.invoke<void*>(type, name, errorIfMissing, waitTime);
	}

	int scr_gsc_obj_link_stub(game::scriptInstance_t inst, game::GSC_OBJ* prime_obj, bool runScript)
	{
		// link the injected scripts if we find a hook
		for (auto& spt : storage.gsc_files)
		{
			if (spt.hooks.find(prime_obj->name) != spt.hooks.end())
			{
				game::GSC_OBJ* script = spt.get_header()->buffer;
				int err = scr_gsc_obj_link_hook.invoke<int>(inst, script, runScript);

				if (err < 0)
				{
					return err; // error when linking
				}
			}
		}

		return scr_gsc_obj_link_hook.invoke<int>(inst, prime_obj, runScript);
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			// custom assets loading
			db_find_xasset_header_hook.create(0x142EB75B0_g, db_find_xasset_header_stub);
			scr_gsc_obj_link_hook.create(0x142748F10_g, scr_gsc_obj_link_stub);

			// register load mods command
			Cmd_AddCommand("shield_load_mods", load_mods_cmd);
		}

		void pre_start() override
		{
			load_mods();
		}
	};
}

REGISTER_COMPONENT(mods::component)