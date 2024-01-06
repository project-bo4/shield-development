#include <std_include.hpp>
#include "gsc_funcs.hpp"
#include "gsc_custom.hpp"
#include "hashes.hpp"
#include "definitions/game.hpp"
#include "definitions/game_runtime_errors.hpp"
#include "definitions/xassets.hpp"
#include "loader/component_loader.hpp"
#include "component/scheduler.hpp"


#include <utilities/hook.hpp>
#include <utilities/io.hpp>
#include <utilities/json_config.hpp>

namespace gsc_funcs
{
	uint32_t canon_hash(const char* str)
	{
		uint32_t hash = 0x4B9ACE2F;

		for (const char* data = str; *data; data++)
		{
			char c = (char)tolower(*data);
			hash = ((c + hash) ^ ((c + hash) << 10)) + (((c + hash) ^ ((c + hash) << 10)) >> 6);
		}

		uint32_t val = 0x8001 * ((9 * hash) ^ ((9 * hash) >> 11));

		hashes::add_hash(val, str);

		return val;
	}

	uint32_t canon_hash_pattern(const char* str)
	{
		std::string_view v{ str };

		// basic notations hash_123, var_123
		if (!v.rfind("hash_", 0)) return std::strtoul(&str[5], nullptr, 16) & 0xFFFFFFFF;
		if (!v.rfind("var_", 0)) return std::strtoul(&str[4], nullptr, 16) & 0xFFFFFFFF;

		// unknown, use hashed value
		return canon_hash(str);
	}

	void gsc_error(const char* message, game::scriptInstance_t inst, bool terminal, ...)
	{
		static char buffer[game::scriptInstance_t::SCRIPTINSTANCE_MAX][0x800];

		va_list va;
		va_start(va, terminal);
		vsprintf_s(buffer[inst], message, va);
		va_end(va);

		game::ScrVm_Error(game::runtime_errors::custom_error_id, inst, buffer[inst], terminal);
	}

	const char* lookup_hash(game::scriptInstance_t inst, const char* type, uint64_t hash)
	{
		static char buffer[game::SCRIPTINSTANCE_MAX][0x50];
		const char* str = hashes::lookup(hash);

		if (str)
		{
			return str;
		}

		sprintf_s(buffer[inst], "%s_%llx", type, hash);

		return buffer[inst];
	}

	void ScrVm_AddToArrayIntIndexed(game::scriptInstance_t inst, uint64_t index)
	{
		auto& vm = game::scrVmPub[inst];
		--vm.callNesting;
		--vm.top;
		game::ScrVarIndex_t varidx = game::ScrVar_NewVariableByIndex(inst, vm.top->u.pointerValue, index);
		game::ScrVar_SetValue(inst, varidx, vm.top + 1);
	}


	namespace
	{
		constexpr auto gsc_json_data_name_max_length = 40;
		constexpr const char* gsc_json_type = "$.type";
		enum hud_elem_align_x
		{
			HUD_ALIGN_X_LEFT = 0,
			HUD_ALIGN_X_CENTER,
			HUD_ALIGN_X_RIGHT,
			HUD_ALIGN_X_MAX,
		};
		enum hud_elem_align_y
		{
			HUD_ALIGN_Y_TOP = 0,
			HUD_ALIGN_Y_MIDDLE,
			HUD_ALIGN_Y_BOTTOM,
			HUD_ALIGN_Y_MAX
		};

		struct hud_elem
		{
			std::string text{};
			hud_elem_align_x align_x{};
			hud_elem_align_y align_y{};
			hud_elem_align_x anchor_x{};
			hud_elem_align_y anchor_y{};
			game::vec4_t color{ 1,1,1,1 };
			float x{};
			float y{};
			float scale = 1;

			void set_color_rgba(uint32_t rgba)
			{
				color[0] = (game::vec_t)(rgba & 0xFF) / 0x100;
				color[1] = (game::vec_t)((rgba >> 8) & 0xFF) / 0x100;
				color[2] = (game::vec_t)((rgba >> 16) & 0xFF) / 0x100;
				uint32_t alpha = (rgba >> 24) & 0xFF;
				color[3] = alpha ? (game::vec_t)(alpha) / 0x100 : 1;
			}

			float get_relative_x(float screen_width, float width) const
			{
				float delta{};
				switch (align_x)
				{
				case HUD_ALIGN_X_LEFT:
					delta = 0;
					break;
				case HUD_ALIGN_X_CENTER:
					delta = -width / 2;
					break;
				case HUD_ALIGN_X_RIGHT:
					delta = -width;
					break;
				}

				switch (anchor_x)
				{
				case HUD_ALIGN_X_LEFT:
					return delta + x;
				case HUD_ALIGN_X_CENTER:
					return screen_width / 2 + delta + x;
				case HUD_ALIGN_X_RIGHT:
					return screen_width + delta + x;
				}
				return 0;
			}

			float get_relative_y(float screen_height, float height) const
			{
				float delta{};
				switch (align_y)
				{
				case HUD_ALIGN_Y_TOP:
					delta = 0;
					break;
				case HUD_ALIGN_Y_MIDDLE:
					delta = -height / 2;
					break;
				case HUD_ALIGN_Y_BOTTOM:
					delta = -height;
					break;
				}

				switch (anchor_y)
				{
				case HUD_ALIGN_Y_TOP:
					return delta + y;
				case HUD_ALIGN_Y_MIDDLE:
					return screen_height / 2 + delta + y;
				case HUD_ALIGN_Y_BOTTOM:
					return screen_height + delta + y;
				}
				return 0;
			}
		};

		std::unordered_map<uint64_t, hud_elem> hud_elems{};

		void shield_log_from(game::scriptInstance_t inst, unsigned int offset)
		{
			game::ScrVarType_t type = game::ScrVm_GetType(inst, offset);
			switch (type)
			{
			case game::TYPE_UNDEFINED:
				logger::write(logger::LOG_TYPE_INFO, "[ %s VM ] undefined", inst ? "CSC" : "GSC");
				break;
			case game::TYPE_POINTER:
				logger::write(logger::LOG_TYPE_INFO, "[ %s VM ] Pointer[%s]", inst ? "CSC" : "GSC", game::var_typename[game::ScrVm_GetPointerType(inst, offset)]);
				break;
			case game::TYPE_STRING:
				logger::write(logger::LOG_TYPE_INFO, "[ %s VM ] %s", inst ? "CSC" : "GSC", game::ScrVm_GetString(inst, offset));
				break;
			case game::TYPE_HASH:
			{
				game::BO4_AssetRef_t hash{};
				logger::write(logger::LOG_TYPE_INFO, "[ %s VM ] %llx", inst ? "CSC" : "GSC", lookup_hash(inst, "hash", game::ScrVm_GetHash(&hash, inst, offset)->hash));
			}
				break;
			case game::TYPE_INTEGER:
				logger::write(logger::LOG_TYPE_INFO, "[ %s VM ] %lld", inst ? "CSC" : "GSC", game::ScrVm_GetInt(inst, offset));
				break;
			case game::TYPE_FLOAT:
				logger::write(logger::LOG_TYPE_INFO, "[ %s VM ] %f", inst ? "CSC" : "GSC", game::ScrVm_GetFloat(inst, offset));
				break;
			case game::TYPE_VECTOR: 
			{
				game::vec3_t vec{};
				game::ScrVm_GetVector(inst, offset, &vec);
				logger::write(logger::LOG_TYPE_INFO, "[ %s VM ] (%f, %f, %f)", inst ? "CSC" : "GSC", vec[0], vec[1], vec[2]);
			}
				break;
			default:
				gsc_error("Call of ShieldLog with unknown type: %s", inst, false, game::var_typename[type]);
				break;
			}
		}

		void shield_log(game::scriptInstance_t inst)
		{
			shield_log_from(inst, 0);
		}

		void shield_clear_hud_elems(game::scriptInstance_t inst)
		{
			hud_elems.clear();
		}

		void shield_register_hud_elem(game::scriptInstance_t inst)
		{
			game::BO4_AssetRef_t hashRef{};

			uint64_t id = game::ScrVm_GetHash(&hashRef, inst, 0)->hash;

			hud_elem& elem = hud_elems[id];

			uint32_t params = game::ScrVm_GetNumParam(inst);

			if (params > 1)
			{
				elem.text = game::ScrVm_GetString(inst, 1);
			}
			else {
				elem.text = "";
			}

			if (params > 2)
			{
				elem.set_color_rgba((uint32_t)game::ScrVm_GetInt(inst, 2));
			}
			else {
				elem.set_color_rgba(0xFFFFFFFF);
			}

			if (params > 3)
			{
				elem.x = game::ScrVm_GetFloat(inst, 3);
			}
			else {
				elem.x = 0;
			}

			if (params > 4)
			{
				elem.y = game::ScrVm_GetFloat(inst, 4);
			}
			else {
				elem.y = 0;
			}

			if (params > 5)
			{
				int64_t val = game::ScrVm_GetInt(inst, 5);

				if (val >= 0 && val > HUD_ALIGN_X_MAX)
				{
					gsc_error("bad hud anchor x value: %lld", inst, false, val);
					elem.anchor_x = HUD_ALIGN_X_LEFT;
				}
				else {
					elem.anchor_x = (hud_elem_align_x)val;
				}
			}
			else {
				elem.anchor_x = HUD_ALIGN_X_LEFT;
			}

			if (params > 6)
			{
				int64_t val = game::ScrVm_GetInt(inst, 6);

				if (val >= 0 && val > HUD_ALIGN_Y_MAX)
				{
					gsc_error("bad hud anchor y value: %lld", inst, false, val);
					elem.anchor_y = HUD_ALIGN_Y_TOP;
				}
				else {
					elem.anchor_y = (hud_elem_align_y)val;
				}
			}
			else {
				elem.anchor_y = HUD_ALIGN_Y_TOP;
			}

			if (params > 7)
			{
				int64_t val = game::ScrVm_GetInt(inst, 7);

				if (val >= 0 && val > HUD_ALIGN_X_MAX)
				{
					gsc_error("bad hud align x value: %lld", inst, false, val);
					elem.align_x = HUD_ALIGN_X_LEFT;
				}
				else {
					elem.align_x = (hud_elem_align_x)val;
				}
			}
			else {
				elem.align_x = HUD_ALIGN_X_LEFT;
			}

			if (params > 8)
			{
				int64_t val = game::ScrVm_GetInt(inst, 8);

				if (val >= 0 && val > HUD_ALIGN_Y_MAX)
				{
					gsc_error("bad hud align y value: %lld", inst, false, val);
					elem.align_y = HUD_ALIGN_Y_TOP;
				}
				else {
					elem.align_y = (hud_elem_align_y)val;
				}
			}
			else {
				elem.align_y = HUD_ALIGN_Y_TOP;
			}

			if (params > 9)
			{
				float val = game::ScrVm_GetFloat(inst, 9);

				if (val >= 0.01)
				{
					elem.scale = val;
				}
				else {
					gsc_error("bad scale value: %f", inst, false, val);
					elem.scale = 1.0;
				}
			}
			else {
				elem.scale = 1.0;
			}
		}

		void shield_remove_hud_elem(game::scriptInstance_t inst)
		{
			game::BO4_AssetRef_t hashRef{};

			hud_elems.erase(game::ScrVm_GetHash(&hashRef, inst, 0)->hash);
		}

		void shield_hud_elem_set_text(game::scriptInstance_t inst)
		{
			game::BO4_AssetRef_t hashRef{};

			uint64_t id = game::ScrVm_GetHash(&hashRef, inst, 0)->hash;
			auto it = hud_elems.find(id);

			if (it == hud_elems.end())
			{
				gsc_error("can't find hud element with id 0x%llx", inst, false, id);
				return;
			}

			it->second.text = game::ScrVm_GetString(inst, 1);
		}

		void shield_hud_elem_set_x(game::scriptInstance_t inst)
		{
			game::BO4_AssetRef_t hashRef{};

			uint64_t id = game::ScrVm_GetHash(&hashRef, inst, 0)->hash;
			auto it = hud_elems.find(id);

			if (it == hud_elems.end())
			{
				gsc_error("can't find hud element with id 0x%llx", inst, false, id);
				return;
			}

			it->second.x = game::ScrVm_GetFloat(inst, 1);
		}

		void shield_hud_elem_set_y(game::scriptInstance_t inst)
		{
			game::BO4_AssetRef_t hashRef{};

			uint64_t id = game::ScrVm_GetHash(&hashRef, inst, 0)->hash;
			auto it = hud_elems.find(id);

			if (it == hud_elems.end())
			{
				gsc_error("can't find hud element with id 0x%llx", inst, false, id);
				return;
			}

			it->second.y = game::ScrVm_GetFloat(inst, 1);
		}

		void shield_hud_elem_set_color(game::scriptInstance_t inst)
		{
			game::BO4_AssetRef_t hashRef{};

			uint64_t id = game::ScrVm_GetHash(&hashRef, inst, 0)->hash;
			auto it = hud_elems.find(id);

			if (it == hud_elems.end())
			{
				gsc_error("can't find hud element with id 0x%llx", inst, false, id);
				return;
			}


			hud_elem& elem = it->second;
			uint32_t params = game::ScrVm_GetNumParam(inst);

			if (params == 2)
			{
				game::ScrVarType_t type = game::ScrVm_GetType(inst, 1);
				if (type == game::TYPE_VECTOR)
				{
					game::vec3_t vec{};
					game::ScrVm_GetVector(inst, 1, &vec);

					elem.color[0] = vec[0];
					elem.color[1] = vec[1];
					elem.color[2] = vec[2];

				}
				else if (type == game::TYPE_INTEGER)
				{
					elem.set_color_rgba((uint32_t)game::ScrVm_GetInt(inst, 1));
				}
				else
				{
					gsc_error("ShieldHudElemSetColor used with bad type %d", inst, false, type);
					return;
				}
			}
			else if (params == 4)
			{
				bool float_based = game::ScrVm_GetType(inst, 1) == game::TYPE_FLOAT || game::ScrVm_GetType(inst, 2) == game::TYPE_FLOAT || game::ScrVm_GetType(inst, 3) == game::TYPE_FLOAT;
				bool int_based = game::ScrVm_GetType(inst, 1) == game::TYPE_INTEGER || game::ScrVm_GetType(inst, 2) == game::TYPE_INTEGER || game::ScrVm_GetType(inst, 3) == game::TYPE_INTEGER;

				if (float_based && int_based)
				{
					gsc_error("ShieldHudElemSetColor should be used with only integers or floats", inst, false);
					return;
				}


				if (float_based)
				{
					elem.color[0] = game::ScrVm_GetFloat(inst, 1);
					elem.color[1] = game::ScrVm_GetFloat(inst, 2);
					elem.color[2] = game::ScrVm_GetFloat(inst, 3);
				} else 
				{
					elem.color[0] = (game::vec_t)game::ScrVm_GetInt(inst, 1) / 0x100;
					elem.color[1] = (game::vec_t)game::ScrVm_GetInt(inst, 2) / 0x100;
					elem.color[2] = (game::vec_t)game::ScrVm_GetInt(inst, 3) / 0x100;
				}

			}
			else
			{
				gsc_error("usage of ShieldHudElemSetColor with a bad number of params", inst, false);
			}

		}

		void shield_hud_elem_set_scale(game::scriptInstance_t inst)
		{
			game::BO4_AssetRef_t hashRef{};

			uint64_t id = game::ScrVm_GetHash(&hashRef, inst, 0)->hash;
			auto it = hud_elems.find(id);

			if (it == hud_elems.end())
			{
				gsc_error("can't find hud element with id 0x%llx", inst, false, id);
				return;
			}

			float val = game::ScrVm_GetFloat(inst, 1);
			if (val >= 0.01)
			{
				it->second.scale = val;
			}
			else {
				gsc_error("bad scale value: %f", inst, false, val);
			}
		}

		void serious_custom_func(game::scriptInstance_t inst)
		{
			// the t8compiler is converting the calls of compiler::func_name(...) to 
			// SeriousCustom(#"func_name", ...) using the canon id hash for func_name
			uint64_t hash = ((uint64_t)game::ScrVm_GetInt(inst, 0)) & 0xFFFFFFFF;

			static uint64_t detour_cid = canon_hash("detour");
			static uint64_t relink_detour_cid = canon_hash("relinkdetours");
			static uint64_t nprintln_cid = canon_hash("nprintln");

			if (hash == detour_cid || hash == relink_detour_cid)
			{
				logger::write(logger::LOG_TYPE_WARN, "a detour link function was called, but it isn't required by this client.");
			}
			else if (hash == nprintln_cid)
			{
				shield_log_from(inst, 1);
			}
			else
			{
				gsc_error("compiler::%s not implemented", inst, false, lookup_hash(inst, "function", hash));
			}
		}

		void pre_cache_resource(game::scriptInstance_t inst)
		{
			game::BO4_AssetRef_t hashRef{};

			byte type;
			if (game::ScrVm_GetType(inst, 0) == game::TYPE_STRING)
			{
				type = xassets::BG_Cache_GetTypeIndex(game::ScrVm_GetString(inst, 0));
			}
			else if (game::ScrVm_GetType(inst, 0) == game::TYPE_INTEGER)
			{
				type = (byte)(game::ScrVm_GetInt(inst, 0) & 0xFF);
			}
			else
			{
				gsc_error("bad param type for PreCache, excepted int or string, received %s", inst, false, game::var_typename[game::ScrVm_GetType(inst, 0)]);
				return;
			}

			if (!type || type >= xassets::BG_CACHE_TYPE_COUNT)
			{
				gsc_error("bad bgcache type for PreCache", inst, false);
				return;
			}

			uint64_t res = game::ScrVm_GetHash(&hashRef, inst, 1)->hash;

			logger::write(logger::LOG_TYPE_DEBUG, "precaching resource type=%d/name=hash_%llx", type, res);

			hashRef.hash = res;
			hashRef.null = 0;
			xassets::BG_Cache_RegisterAndGet((xassets::BGCacheTypes)type, &hashRef);
		}

		void shield_to_json_val(game::scriptInstance_t inst, game::ScrVarValue_t* val, rapidjson::Value& member, rapidjson::Document& doc, int depth)
		{
			if (depth >= 10)
			{
				// avoid recursion
				member.SetNull();
				return;
			}
			switch (val->type)
			{
			case game::TYPE_UNDEFINED: return; // ignore
			case game::TYPE_FLOAT:
			{
				member.SetFloat(val->u.floatValue);
			}
			break;
			case game::TYPE_INTEGER:
			{
				member.SetInt64(val->u.intValue);
			}
			break;
			case game::TYPE_STRING:
			{
				member.SetString(game::ScrStr_ConvertToString(val->u.pointerValue), doc.GetAllocator());
			}
			break;
			case game::TYPE_VECTOR:
			{
				member.SetObject();
				auto obj = member.GetObj();
				obj.AddMember(rapidjson::StringRef(gsc_json_type), "vector", doc.GetAllocator());
				obj.AddMember(rapidjson::StringRef("x"), val->u.vectorValue[0], doc.GetAllocator());
				obj.AddMember(rapidjson::StringRef("y"), val->u.vectorValue[1], doc.GetAllocator());
				obj.AddMember(rapidjson::StringRef("z"), val->u.vectorValue[2], doc.GetAllocator());
			}
			break;
			case game::TYPE_HASH:
			{
				member.SetObject();
				auto obj = member.GetObj();
				obj.AddMember(rapidjson::StringRef(gsc_json_type), "hash", doc.GetAllocator());
				auto hash = val->u.intValue & 0x7FFFFFFFFFFFFFFF;
				std::string name = std::format("hash_{:x}", hash);
				obj.AddMember(rapidjson::StringRef("hash"), name, doc.GetAllocator());
			}
			break;
			case game::TYPE_POINTER:
			{
				game::ScrVarIndex_t ptr_id = val->u.pointerValue;
				game::ScrVarValue_t& ptr_val = game::scrVarGlob[inst].scriptValues[ptr_id];

				member.SetObject();
				auto obj = member.GetObj();

				
				if (ptr_val.type == game::TYPE_ARRAY)
				{
					obj.AddMember(rapidjson::StringRef(gsc_json_type), "array", doc.GetAllocator());

					auto size = game::scrVarGlob[inst].scriptVariablesObjectInfo1[ptr_id].size;
					if (size)
					{
						game::ScrVar_t* var = &game::scrVarGlob[inst].scriptVariables[ptr_val.u.pointerValue];
						game::ScrVarValue_t* value = &game::scrVarGlob[inst].scriptValues[ptr_val.u.pointerValue];

						while (var)
						{
							rapidjson::Value subval{};

							// read struct value
							shield_to_json_val(inst, value, subval, doc, depth + 1);

							if (var->_anon_0.nameType == 1) // integer index
							{
								std::string keyval = std::format("{}", var->nameIndex);
								rapidjson::Value keyjson{ rapidjson::kStringType };
								keyjson.SetString(keyval, doc.GetAllocator());
								obj.AddMember(keyjson, subval, doc.GetAllocator());
							}
							else
							{
								std::string keyval = std::format("#var_{:x}", var->nameIndex);
								rapidjson::Value keyjson{ rapidjson::kStringType };
								keyjson.SetString(keyval, doc.GetAllocator());
								obj.AddMember(keyjson, subval, doc.GetAllocator());
							}

							if (!var->nextSibling)
							{
								break;
							}

							value = &game::scrVarGlob[inst].scriptValues[var->nextSibling];
							var = &game::scrVarGlob[inst].scriptVariables[var->nextSibling];
						}
					}
					return;
				}
				
				if (ptr_val.type == game::TYPE_STRUCT)
				{
					auto size = game::scrVarGlob[inst].scriptVariablesObjectInfo1[ptr_id].size;
					if (size)
					{
						game::ScrVar_t* var = &game::scrVarGlob[inst].scriptVariables[ptr_val.u.pointerValue];
						game::ScrVarValue_t* value = &game::scrVarGlob[inst].scriptValues[ptr_val.u.pointerValue];

						while (var)
						{
							rapidjson::Value subval{};

							// read struct value
							shield_to_json_val(inst, value, subval, doc, depth + 1);

							std::string keyval = std::format("var_{:x}", var->nameIndex);
							rapidjson::Value keyjson{ rapidjson::kStringType };
							keyjson.SetString(keyval, doc.GetAllocator());
							obj.AddMember(keyjson, subval, doc.GetAllocator());

							if (!var->nextSibling)
							{
								break;
							}

							value = &game::scrVarGlob[inst].scriptValues[var->nextSibling];
							var = &game::scrVarGlob[inst].scriptVariables[var->nextSibling];
						}
					}
					return;
				}
				// shared_struct and entity aren't using the same syntax
				gsc_error("invalid tojson param pointer type: %s", inst, false, game::var_typename[ptr_val.type]);
				return;
			}
			break;
			default:
			{
				gsc_error("invalid tojson param type: %s", inst, false, game::var_typename[val->type]);
				return;
			}
			}
		}

		void shield_to_json(game::scriptInstance_t inst)
		{
			const char* fileid = game::ScrVm_GetString(inst, 0);
			std::string_view v{ fileid };

			if (v.rfind("/", 0) != std::string::npos || v.rfind("\\", 0) != std::string::npos)
			{
				gsc_error("can't save json containing '/' or '\\'", inst, false);
				return;
			}

			if (v.length() > gsc_json_data_name_max_length)
			{
				gsc_error("json name can't be longer than %d", inst, false, gsc_json_data_name_max_length);
				return;
			}

			std::string file{ std::format("project-bo4/saved/{}/{}.json", (inst ? "client" : "server"), fileid) };

			if (inst)
			{
				std::filesystem::create_directories("project-bo4/saved/client");
			}
			else
			{
				std::filesystem::create_directories("project-bo4/saved/server");
			}

			game::ScrVarValue_t* val = &game::scrVmPub[inst].top[-1];

			if (game::ScrVm_GetNumParam(inst) == 1 || val->type == game::TYPE_UNDEFINED)
			{
				std::filesystem::remove(file);
				return;
			}

			rapidjson::Document doc{};

			shield_to_json_val(inst, val, doc, doc, 0);

			rapidjson::StringBuffer buffer;
			rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
			doc.Accept(writer);

			std::string json_data(buffer.GetString(), buffer.GetLength());
			utilities::io::write_file(file, json_data);
		}

		void shield_from_json_push_struct(game::scriptInstance_t inst, rapidjson::Value& member)
		{
			if (member.IsString())
			{
				game::ScrVm_AddString(inst, member.GetString());
				return;
			}

			if (member.IsFloat())
			{
				game::ScrVm_AddFloat(inst, member.GetFloat());
				return;
			}

			if (member.IsNumber())
			{
				game::ScrVm_AddInt(inst, member.GetInt64());
				return;
			}

			if (member.IsBool())
			{
				game::ScrVm_AddBool(inst, member.GetBool());
				return;
			}

			if (member.IsNull())
			{
				logger::write(logger::LOG_TYPE_WARN, "ShieldFromJson: read null");
				game::ScrVm_AddUndefined(inst);
				return;
			}
			
			if (member.IsArray())
			{
				auto arr = member.GetArray();

				game::ScrVar_PushArray(inst);

				for (rapidjson::Value& elem : arr)
				{
					shield_from_json_push_struct(inst, elem);
					game::ScrVm_AddToArray(inst);
				}
				return;
			}

			if (member.IsObject())
			{
				auto obj = member.GetObj();
				auto typefield = obj.FindMember(gsc_json_type);

				if (typefield != obj.MemberEnd() && typefield->value.IsString())
				{
					const char* type = typefield->value.GetString();

					if (!_strcmpi(type, "array"))
					{
						game::ScrVar_PushArray(inst);

						game::BO4_AssetRef_t name{};

						for (auto& [key, elem] : obj)
						{
							if (!key.IsString())
							{
								gsc_error("read bad key value for array", inst, false);
								return;
							}
							if (!_strcmpi(key.GetString(), gsc_json_type))
							{
								continue;
							}
							const char* keystr = key.GetString();
							if (!*keystr)
							{
								continue;
							}
							shield_from_json_push_struct(inst, elem);

							if (*keystr == '#')
							{
								name.hash = fnv1a::generate_hash_pattern(key.GetString() + 1);
								game::ScrVm_AddToArrayStringIndexed(inst, &name);
							}
							else
							{
								uint64_t keyval{};
								try
								{
									keyval = std::strtoull(keystr, nullptr, 10);
								}
								catch (const std::invalid_argument& e)
								{
									gsc_error("invalid key for array member %s", inst, false, e.what());
									return;
								}
								ScrVm_AddToArrayIntIndexed(inst, keyval);
							}
						}
						return;
					}

					if (!_strcmpi(type, "vector"))
					{
						auto x = obj.FindMember("x");
						auto y = obj.FindMember("y");
						auto z = obj.FindMember("z");

						if (
							x != obj.MemberEnd() && y != obj.MemberEnd() && z != obj.MemberEnd()
							&& x->value.IsNumber() && y->value.IsNumber() && z->value.IsNumber())
						{
							game::vec3_t vec{};

							vec[0] = x->value.GetFloat();
							vec[1] = y->value.GetFloat();
							vec[2] = z->value.GetFloat();
							game::ScrVm_AddVector(inst, &vec);
							return;
						}
					}
					if (!_strcmpi(type, "hash"))
					{
						auto value = obj.FindMember("hash");

						if (value != obj.MemberEnd() && value->value.IsString()) {
							game::BO4_AssetRef_t hash
							{
								.hash = (int64_t)fnv1a::generate_hash_pattern(value->value.GetString())
							};

							game::ScrVm_AddHash(inst, &hash);
							return;
						}
					}
				}

				// object by default
				uint32_t struct_id = game::ScrVm_AddStruct(inst);

				for (auto& [key, elem] : obj)
				{
					if (!key.IsString())
					{
						gsc_error("read bad key value for struct", inst, false);
						return;
					}
					if (!_strcmpi(key.GetString(), gsc_json_type))
					{
						continue;
					}
					shield_from_json_push_struct(inst, elem);
					game::ScrVm_SetStructField(inst, struct_id, canon_hash_pattern(key.GetString()));
				}
				return;
			}
			gsc_error("bad json element: %d", inst, false, (int)member.GetType());
		}

		void shield_from_json(game::scriptInstance_t inst)
		{
			const char* fileid = game::ScrVm_GetString(inst, 0);
			std::string_view v{ fileid };

			if (v.rfind("/", 0) != std::string::npos || v.rfind("\\", 0) != std::string::npos)
			{
				gsc_error("can't save json containing '/' or '\\'", inst, false);
				return;
			}

			if (v.length() > gsc_json_data_name_max_length)
			{
				gsc_error("json name can't be longer than %d", inst, false, gsc_json_data_name_max_length);
				return;
			}
			
			std::string file{ std::format("project-bo4/saved/{}/{}.json", (inst ? "client" : "server"), fileid) };

			std::string file_content{};

			if (!utilities::io::read_file(file, &file_content))
			{
				logger::write(logger::LOG_TYPE_WARN, "trying to read unknown config file %s", file.c_str());
				return;
			}

			rapidjson::Document doc{};
			doc.Parse(file_content);

			shield_from_json_push_struct(inst, doc);
		}
		
		game::BO4_BuiltinFunctionDef custom_functions_gsc[] =
		{
			{ // ShieldLog(message)
				.canonId = canon_hash("ShieldLog"),
				.min_args = 1,
				.max_args = 1,
				.actionFunc = shield_log,
				.type = 0,
			},
			{ // ShieldClearHudElems()
				.canonId = canon_hash("ShieldClearHudElems"),
				.min_args = 0,
				.max_args = 0,
				.actionFunc = shield_clear_hud_elems,
				.type = 0,
			},
			{ // ShieldRegisterHudElem(id, text = "", color = 0xFFFFFFFF, x = 0, y = 0, anchor_x = 0, anchor_y = 0, align_x = 0, align_y = 0, scale = 1.0)
				.canonId = canon_hash("ShieldRegisterHudElem"),
				.min_args = 1,
				.max_args = 10,
				.actionFunc = shield_register_hud_elem,
				.type = 0,
			},
			{ // ShieldRemoveHudElem(id)
				.canonId = canon_hash("ShieldRemoveHudElem"),
				.min_args = 1,
				.max_args = 1,
				.actionFunc = shield_remove_hud_elem,
				.type = 0,
			},
			{ // ShieldHudElemSetText(id, text)
				.canonId = canon_hash("ShieldHudElemSetText"),
				.min_args = 2,
				.max_args = 2,
				.actionFunc = shield_hud_elem_set_text,
				.type = 0,
			},
			{ // ShieldHudElemSetX(id, x)
				.canonId = canon_hash("ShieldHudElemSetX"),
				.min_args = 2,
				.max_args = 2,
				.actionFunc = shield_hud_elem_set_x,
				.type = 0,
			},
			{ // ShieldHudElemSetY(id, y)
				.canonId = canon_hash("ShieldHudElemSetY"),
				.min_args = 2,
				.max_args = 2,
				.actionFunc = shield_hud_elem_set_y,
				.type = 0,
			},
			{ // ShieldHudElemSetColor(id, color_vec) |  ShieldHudElemSetColor(id, color_rgba) |  ShieldHudElemSetColor(id, r, g, b)
				.canonId = canon_hash("ShieldHudElemSetColor"),
				.min_args = 2,
				.max_args = 4,
				.actionFunc = shield_hud_elem_set_color,
				.type = 0,
			},
			{ // ShieldHudElemSetScale(id, scale)
				.canonId = canon_hash("ShieldHudElemSetScale"),
				.min_args = 2,
				.max_args = 2,
				.actionFunc = shield_hud_elem_set_scale,
				.type = 0,
			},
			{ // SeriousCustom(func_hash, ...)
				.canonId = canon_hash(serious_custom_func_name),
				.min_args = 1,
				.max_args = 255,
				.actionFunc = serious_custom_func,
				.type = 0,
			},
			{ // PreCache(type, name)
				.canonId = canon_hash("PreCache"),
				.min_args = 2,
				.max_args = 2,
				.actionFunc = pre_cache_resource,
				.type = 0,
			},
			{ // ShieldFromJson(name)->object
				.canonId = canon_hash("ShieldFromJson"),
				.min_args = 1,
				.max_args = 1,
				.actionFunc = shield_from_json,
				.type = 0
			},
			{ // ShieldFromJson(name)
				.canonId = canon_hash("ShieldRemoveJson"),
				.min_args = 1,
				.max_args = 1,
				.actionFunc = shield_to_json,
				.type = 0
			},
			{// ShieldFromJson(name, object = undefined)
				.canonId = canon_hash("ShieldToJson"),
				.min_args = 1,
				.max_args = 2,
				.actionFunc = shield_to_json,
				.type = 0
			}
		};
		game::BO4_BuiltinFunctionDef custom_functions_csc[] =
		{
			{ // ShieldLog(message)
				.canonId = canon_hash("ShieldLog"),
				.min_args = 1,
				.max_args = 1,
				.actionFunc = shield_log,
				.type = 0,
			},
			{ // ShieldClearHudElems()
				.canonId = canon_hash("ShieldClearHudElems"),
				.min_args = 0,
				.max_args = 0,
				.actionFunc = shield_clear_hud_elems,
				.type = 0,
			},
			{ // ShieldRegisterHudElem(id, text = "", color = 0xFFFFFFFF, x = 0, y = 0, anchor_x = 0, anchor_y = 0, align_x = 0, align_y = 0, scale = 1.0)
				.canonId = canon_hash("ShieldRegisterHudElem"),
				.min_args = 1,
				.max_args = 10,
				.actionFunc = shield_register_hud_elem,
				.type = 0,
			},
			{ // ShieldRemoveHudElem(id)
				.canonId = canon_hash("ShieldRemoveHudElem"),
				.min_args = 1,
				.max_args = 1,
				.actionFunc = shield_remove_hud_elem,
				.type = 0,
			},
			{ // ShieldHudElemSetText(id, text)
				.canonId = canon_hash("ShieldHudElemSetText"),
				.min_args = 2,
				.max_args = 2,
				.actionFunc = shield_hud_elem_set_text,
				.type = 0,
			},
			{ // ShieldHudElemSetX(id, x)
				.canonId = canon_hash("ShieldHudElemSetX"),
				.min_args = 2,
				.max_args = 2,
				.actionFunc = shield_hud_elem_set_x,
				.type = 0,
			},
			{ // ShieldHudElemSetY(id, y)
				.canonId = canon_hash("ShieldHudElemSetY"),
				.min_args = 2,
				.max_args = 2,
				.actionFunc = shield_hud_elem_set_y,
				.type = 0,
			},
			{ // ShieldHudElemSetColor(id, color_vec) |  ShieldHudElemSetColor(id, color_rgba) |  ShieldHudElemSetColor(id, r, g, b)
				.canonId = canon_hash("ShieldHudElemSetColor"),
				.min_args = 2,
				.max_args = 4,
				.actionFunc = shield_hud_elem_set_color,
				.type = 0,
			},
			{ // ShieldHudElemSetScale(id, scale)
				.canonId = canon_hash("ShieldHudElemSetScale"),
				.min_args = 2,
				.max_args = 2,
				.actionFunc = shield_hud_elem_set_scale,
				.type = 0,
			},
			{ // SeriousCustom(func_hash, ...)
				.canonId = canon_hash(serious_custom_func_name),
				.min_args = 1,
				.max_args = 255,
				.actionFunc = serious_custom_func,
				.type = 0,
			},
			{ // ShieldFromJson(name)->object
				.canonId = canon_hash("ShieldFromJson"),
				.min_args = 1,
				.max_args = 1,
				.actionFunc = shield_from_json,
				.type = 0
			},
			{ // ShieldFromJson(name)
				.canonId = canon_hash("ShieldRemoveJson"),
				.min_args = 1,
				.max_args = 1,
				.actionFunc = shield_to_json,
				.type = 0
			},
			{// ShieldToJson(name, object = undefined)
				.canonId = canon_hash("ShieldToJson"),
				.min_args = 1,
				.max_args = 2,
				.actionFunc = shield_to_json,
				.type = 0
			}
		};

		void draw_hud()
		{
			const game::vec_t screen_width = game::ScrPlace_GetView(0)->realViewportSize[0];
			const game::vec_t screen_height = game::ScrPlace_GetView(0)->realViewportSize[1];

			if (game::Com_IsRunningUILevel())
			{
				// clear huds
				hud_elems.clear();
				return;
			}

			// render huds
			void* font = reinterpret_cast<void*>(game::sharedUiInfo->assets.bigFont); 
			if (!font) return;

			for (auto& [id, elem] : hud_elems)
			{
				const char* text = elem.text.c_str();
				int height = game::UI_TextHeight(font, elem.scale);
				int width = game::UI_TextWidth(0, text, 0x7FFFFFFF, font, elem.scale);
				float rx = elem.get_relative_x(screen_width, (float)width);
				float ry = elem.get_relative_y(screen_height, (float)height) + height;

				game::R_AddCmdDrawText(text, 0x7FFFFFFF, font, rx, ry, elem.scale, elem.scale, 0.0f, elem.color, 0);
			}
		}
	}


	bool enable_dev_func = false;

	utilities::hook::detour scr_get_function_reverse_lookup;
	utilities::hook::detour cscr_get_function_reverse_lookup;
	utilities::hook::detour scr_get_function;
	utilities::hook::detour cscr_get_function;
	utilities::hook::detour scr_get_method;
	utilities::hook::detour cscr_get_method;
	utilities::hook::detour scrvm_error;

	bool scr_get_function_reverse_lookup_stub(void* func, uint32_t* hash, bool* isFunction)
	{
		if (scr_get_function_reverse_lookup.invoke<bool>(func, hash, isFunction))
		{
			return true;
		}

		auto f = std::find_if(std::begin(custom_functions_gsc), std::end(custom_functions_gsc), [func](const game::BO4_BuiltinFunctionDef& def) { return def.actionFunc == func; });

		if (f != std::end(custom_functions_gsc))
		{
			*hash = f->canonId;
			*isFunction = true;
			return true;
		}

		return false;
	}
	bool cscr_get_function_reverse_lookup_stub(void* func, uint32_t* hash, bool* isFunction)
	{
		if (cscr_get_function_reverse_lookup.invoke<bool>(func, hash, isFunction))
		{
			return true;
		}

		auto f = std::find_if(std::begin(custom_functions_csc), std::end(custom_functions_csc), [func](const game::BO4_BuiltinFunctionDef& def) { return def.actionFunc == func; });

		if (f != std::end(custom_functions_csc))
		{
			*hash = f->canonId;
			*isFunction = true;
			return true;
		}

		return false;
	}
	
	void* scr_get_function_stub(uint32_t name, int32_t* type, int32_t* min_args, int32_t* max_args)
	{
		void* func = scr_get_function.invoke<void*>(name, type, min_args, max_args);

		if (enable_dev_func)
		{
			*type = 0;
		}

		if (func)
		{
			return func;
		}

		auto f = std::find_if(std::begin(custom_functions_gsc), std::end(custom_functions_gsc), [name](const game::BO4_BuiltinFunctionDef& func) { return func.canonId == name; });

		if (f != std::end(custom_functions_gsc))
		{
			*type = f->type && !enable_dev_func;
			*min_args = f->min_args;
			*max_args = f->max_args;

			return f->actionFunc;
		}

		return nullptr;
	}
	void* cscr_get_function_stub(uint32_t name, int32_t* type, int32_t* min_args, int32_t* max_args)
	{
		void* func = cscr_get_function.invoke<void*>(name, type, min_args, max_args);

		if (enable_dev_func)
		{
			*type = 0;
		}

		if (func)
		{
			return func;
		}

		auto f = std::find_if(std::begin(custom_functions_csc), std::end(custom_functions_csc), [name](const game::BO4_BuiltinFunctionDef& func) { return func.canonId == name; });

		if (f != std::end(custom_functions_csc))
		{
			*type = f->type && !enable_dev_func;
			*min_args = f->min_args;
			*max_args = f->max_args;

			return f->actionFunc;
		}

		return nullptr;
	}
	void* scr_get_method_stub(uint32_t name, int32_t* type, int32_t* min_args, int32_t* max_args)
	{
		void* func = scr_get_method.invoke<void*>(name, type, min_args, max_args);

		if (enable_dev_func)
		{
			*type = 0;
		}

		return func;
	}
	void* cscr_get_method_stub(uint32_t name, int32_t* type, int32_t* min_args, int32_t* max_args)
	{
		void* func = cscr_get_method.invoke<void*>(name, type, min_args, max_args);

		if (enable_dev_func)
		{
			*type = 0;
		}

		return func;
	}

	void scrvm_error_stub(uint64_t code, game::scriptInstance_t inst, char* unused, bool terminal)
	{
		static char buffer[game::scriptInstance_t::SCRIPTINSTANCE_MAX][0x200] = { 0 };

		// reimplement assert/assertmsg/errormsg functions
		switch (code)
		{
		case 2737681163:// Assert(val, msg) with message error
		{
			const char* msg = game::ScrVm_GetString(inst, 1);
			sprintf_s(buffer[inst], "assert fail: %s", msg);
			game::scrVarPub[inst].error_message = buffer[inst];
		}
			break;
		case 1385570291:// AssertMsg(msg)
		{
			const char* msg = game::ScrVm_GetString(inst, 0);
			sprintf_s(buffer[inst], "assert fail: %s", msg);
			game::scrVarPub[inst].error_message = buffer[inst];
		}
			break;
		case 2532286589:// ErrorMsg(msg)
		{
			const char* msg = game::ScrVm_GetString(inst, 0);
			sprintf_s(buffer[inst], "error: %s", msg);
			game::scrVarPub[inst].error_message = buffer[inst];
		}
			break;
		default:
			// put custom message for our id
			if (code == game::runtime_errors::custom_error_id)
			{
				game::scrVarPub[inst].error_message = unused;
			}
			else
			{
				game::scrVarPub[inst].error_message = game::runtime_errors::get_error_message(code);
			}
			break;
		}

		logger::write(terminal ? logger::LOG_TYPE_ERROR : logger::LOG_TYPE_WARN, "[ %s VM ] %s (%lld)", 
			inst ? "CSC" : "GSC", game::scrVarPub[inst].error_message ? game::scrVarPub[inst].error_message : "no message", code);

		scrvm_error.invoke<void>(code, inst, unused, terminal);
	}

	void scrvm_log_compiler_error(const char* fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		char buffer[0x800];

		int e = vsprintf_s(buffer, fmt, va);
		va_end(va);

		if (e > 0 && buffer[e - 1] == '\n')
		{
			buffer[e - 1] = 0; // remove end new line
		}

		std::string str{ buffer };

		logger::write(logger::LOG_TYPE_ERROR, str);
	}

	void patch_scrvm_runtime_error()
	{
		const auto scrvm_runtimeerror = 0x1427775B0_g;
		void* stub = utilities::hook::assemble([scrvm_runtimeerror](utilities::hook::assembler& a)
			{
				a.mov(ebx, 0x20); // set errorcode to 0x20
				a.mov(al, 0); // set ZF to avoid the ebx overwrite

				a.jmp(scrvm_runtimeerror + 0x3DC);
			}
		);

		utilities::hook::jump(scrvm_runtimeerror + 0x3A0, stub);
	}

	void get_gsc_export_info(game::scriptInstance_t inst, byte* codepos, const char** scriptname, int32_t* sloc, int32_t* crc, int32_t* vm)
	{
		static char scriptnamebuffer[game::scriptInstance_t::SCRIPTINSTANCE_MAX][0x200];
		game::GSC_OBJ* script_obj = nullptr;
		{
			game::scoped_critical_section scs{ 0x36, game::SCOPED_CRITSECT_NORMAL };

			uint32_t count = game::gObjFileInfoCount[inst];

			for (size_t i = 0; i < count; i++)
			{
				game::objFileInfo_t& info = (*game::gObjFileInfo)[inst][i];

				game::GSC_OBJ* obj = info.activeVersion;

				if (codepos >= obj->magic + obj->start_data && codepos < obj->magic + obj->start_data + obj->data_length)
				{
					script_obj = obj;
					break;
				}
			}
		}

		if (script_obj)
		{
			game::GSC_EXPORT_ITEM* export_item = nullptr;

			uint32_t rloc = (uint32_t)(codepos - script_obj->magic);

			for (size_t i = 0; i < script_obj->exports_count; i++)
			{
				game::GSC_EXPORT_ITEM* exp = reinterpret_cast<game::GSC_EXPORT_ITEM*>(script_obj->magic + script_obj->exports_offset) + i;

				if (rloc < exp->address)
				{
					continue; // our code is after
				}

				if (export_item && export_item->address > exp->address)
				{
					continue; // we already have a better candidate
				}

				export_item = exp;
			}


			if (scriptname)
			{
				if (export_item)
				{
					std::string script_name = lookup_hash(inst, "script", script_obj->name & 0x7FFFFFFFFFFFFFFF);

					sprintf_s(scriptnamebuffer[inst], "%s::%s@%x", script_name.c_str(), lookup_hash(inst, "function", export_item->name), rloc - export_item->address);
				} else
				{
					sprintf_s(scriptnamebuffer[inst], "%s", lookup_hash(inst, "script", script_obj->name & 0x7FFFFFFFFFFFFFFF));
				}

				*scriptname = scriptnamebuffer[inst];
			}

			if (sloc)
			{
				*sloc = rloc;
			}

			if (crc)
			{
				*crc = script_obj->crc;
			}

			if (vm)
			{
				*vm = script_obj->magic[7];
			}
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			// enable dev functions still available in the game
			enable_dev_func = utilities::json_config::ReadBoolean("gsc", "dev_funcs", false);

			scr_get_function_reverse_lookup.create(0x1433AF8A0_g, scr_get_function_reverse_lookup_stub);
			cscr_get_function_reverse_lookup.create(0x141F132A0_g, cscr_get_function_reverse_lookup_stub);
			cscr_get_function.create(0x141F13140_g, cscr_get_function_stub);
			cscr_get_method.create(0x141F13650_g, cscr_get_method_stub);
			scr_get_function.create(0x1433AF840_g, scr_get_function_stub);
			scr_get_method.create(0x1433AFC20_g, scr_get_method_stub);

			// log gsc errors
			scrvm_error.create(0x142770330_g, scrvm_error_stub);
			utilities::hook::jump(0x142890470_g, scrvm_log_compiler_error);

			// better runtime error
			utilities::hook::jump(0x142748550_g, get_gsc_export_info);
			patch_scrvm_runtime_error();
			
			scheduler::loop(draw_hud, scheduler::renderer);
		}
	};
}

REGISTER_COMPONENT(gsc_funcs::component)