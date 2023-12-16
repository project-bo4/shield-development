#include <std_include.hpp>
#include "gsc_funcs.hpp"
#include "gsc_custom.hpp"
#include "definitions/game.hpp"
#include "loader/component_loader.hpp"
#include "component/scheduler.hpp"


#include <utilities/hook.hpp>
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

		return 0x8001 * ((9 * hash) ^ ((9 * hash) >> 11));
	}
	void gsc_error(const char* message, game::scriptInstance_t inst, bool terminal, ...)
	{
		static char buffer[game::scriptInstance_t::SCRIPTINSTANCE_MAX][0x800];

		va_list va;
		va_start(va, terminal);
		vsprintf_s(buffer[inst], message, va);
		va_end(va);

		game::ScrVm_Error(custom_error_id, inst, buffer[inst], terminal);
	}

	namespace
	{
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
			case game::TYPE_STRING:
				logger::write(logger::LOG_TYPE_INFO, "[ %s VM ] %s", inst ? "CSC" : "GSC", game::ScrVm_GetString(inst, offset));
				break;
			case game::TYPE_HASH:
			{
				game::BO4_AssetRef_t hash{};
				logger::write(logger::LOG_TYPE_INFO, "[ %s VM ] %llx", inst ? "CSC" : "GSC", game::ScrVm_GetHash(&hash, inst, offset)->hash);
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
				gsc_error("Call of ShieldLog with unknown type: %d", inst, false, type);
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
				gsc_error("compiler::function_%llx not implemented", inst, false, hash);
			}
		}

		void pre_cache_resource(game::scriptInstance_t inst)
		{
			game::BO4_AssetRef_t hashRef{};
			byte type = (byte)(game::ScrVm_GetInt(inst, 0) & 0xFF);
			uint64_t res = game::ScrVm_GetHash(&hashRef, inst, 1)->hash;

			logger::write(logger::LOG_TYPE_DEBUG, "precaching resource type=%d/name=hash_%llx", type, res);

			hashRef.hash = res;
			hashRef.null = 0;
			game::BG_Cache_RegisterAndGet(type, &hashRef);
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
				.canonId = canon_hash("PreCacheResource"),
				.min_args = 2,
				.max_args = 2,
				.actionFunc = pre_cache_resource,
				.type = 0,
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
			if (code == custom_error_id)
			{
				game::scrVarPub[inst].error_message = unused;
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
			
			scheduler::loop(draw_hud, scheduler::renderer);
		}
	};
}

REGISTER_COMPONENT(gsc_funcs::component)