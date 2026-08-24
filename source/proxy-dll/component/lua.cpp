#include <std_include.hpp>
#include "definitions/game.hpp"
#include "loader/component_loader.hpp"

#include <utilities/json_config.hpp>
#include <utilities/hook.hpp>

namespace lua {
	typedef void (*HksLogFunc)(game::lua_state* s, const char* fmt, ...);

	utilities::hook::detour lual_error_hook;
	utilities::hook::detour hksi_lual_error_hook;

	void lual_error_stub(game::lua_state* state, const char* fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		char buffer[0x800];

		vsprintf_s(buffer, fmt, va);
		va_end(va);

		logger::write(logger::LOG_TYPE_ERROR, std::format("[lual_error] {}", buffer));
		lual_error_hook.invoke<void>(state, "%s", buffer);
	}

	void hksi_lual_error_stub(game::lua_state* state, const char* fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		char buffer[0x800];

		vsprintf_s(buffer, fmt, va);
		va_end(va);

		logger::write(logger::LOG_TYPE_ERROR, std::format("[hksi_lual_error] {}", buffer));
		hksi_lual_error_hook.invoke<void>(state, "%s", buffer);
	}

	void print_out(logger::type type, game::consoleLabel_e label, const char* info)
	{
		size_t len = std::strlen(info);

		while (len && info[len - 1] == '\n')
		{
			len--;
		}

		std::string buff{ info, len };

		if (label != game::CON_LABEL_TEMP)
		{
			const char* label_str;

			if (label > 0 && label < game::CON_LABEL_COUNT)
			{
				label_str = game::builtinLabels[label];
			}
			else
			{
				label_str = "INVALID";
			}

			logger::write(type, std::format("[lua] {} - {}", label_str, buff));

		}
		else
		{
			// no label
			logger::write(type, std::format("[lua] {}", buff));
		}
	}

	void lua_engine_print(logger::type type, game::lua_state* s)
	{
		game::hks_object* base = s->m_apistack.base;
		game::hks_object* top = s->m_apistack.top;

		game::consoleLabel_e label = game::CON_LABEL_TEMP;
		const char* info = "";

		if (base < top)
		{
			label = (game::consoleLabel_e)game::hks_obj_tonumber(s, base);
		}
		if (base + 1 < top)
		{
			info = game::hks_obj_tolstring(s, base + 1, nullptr);
		}

		print_out(type, label, info);
	}

	void lua_engine_print_info(game::lua_state* s)
	{
		lua_engine_print(logger::LOG_TYPE_INFO, s);
	}

	void lua_engine_print_warning(game::lua_state* s)
	{
		lua_engine_print(logger::LOG_TYPE_WARN, s);
	}

	void lua_engine_print_error(game::lua_state* s)
	{
		lua_engine_print(logger::LOG_TYPE_ERROR, s);
	}

	int lua_unsafe_function_stub(game::lua_state* state)
	{
		static std::once_flag f{};

		std::call_once(f, []() { logger::write(logger::LOG_TYPE_ERROR, "calling of a disabled lua unsafe method, these methods can enabled in the config file."); });
		return 0;
	}

	void patch_unsafe_lua_functions()
	{
		if (utilities::json_config::ReadBoolean("lua", "allow_unsafe_function", false))
		{
			return; 
		}

		// in boiii, need to be added, not in bo4?
		// - 0x141FD3220_g engine_openurl
		// - 0x141D34190_g debug
			
		utilities::hook::jump(0x143733A00_g, lua_unsafe_function_stub); // base_loadfile
		utilities::hook::jump(0x143734B80_g, lua_unsafe_function_stub); // base_load
		utilities::hook::jump(0x143739770_g, lua_unsafe_function_stub); // string_dump
		

		// io helpers
		utilities::hook::jump(0x14373D130_g, lua_unsafe_function_stub); // os_exit
		utilities::hook::jump(0x14373D150_g, lua_unsafe_function_stub); // os_remove
		utilities::hook::jump(0x14373D280_g, lua_unsafe_function_stub); // os_rename
		utilities::hook::jump(0x14373CFF0_g, lua_unsafe_function_stub); // os_tmpname
		utilities::hook::jump(0x14373CFC0_g, lua_unsafe_function_stub); // os_sleep
		utilities::hook::jump(0x14373CE60_g, lua_unsafe_function_stub); // os_execute
		utilities::hook::jump(0x14373CEC0_g, lua_unsafe_function_stub); // os_getenv

		utilities::hook::jump(0x14373B2A0_g, lua_unsafe_function_stub); // io_close
		utilities::hook::jump(0x14373B190_g, lua_unsafe_function_stub); // io_close_file
		utilities::hook::jump(0x14373B160_g, lua_unsafe_function_stub); // io_flush
		utilities::hook::jump(0x14375A8E0_g, lua_unsafe_function_stub); // io_output/io_input
		utilities::hook::jump(0x14373B8E0_g, lua_unsafe_function_stub); // io_lines
		utilities::hook::jump(0x14373B7A0_g, lua_unsafe_function_stub); // io_lines
		utilities::hook::jump(0x14373BE50_g, lua_unsafe_function_stub); // io_open
		utilities::hook::jump(0x14373B6E0_g, lua_unsafe_function_stub); // io_read
		utilities::hook::jump(0x14373B330_g, lua_unsafe_function_stub); // io_read_file
		utilities::hook::jump(0x14373BBC0_g, lua_unsafe_function_stub); // io_type
		utilities::hook::jump(0x14373B0E0_g, lua_unsafe_function_stub); // io_write
		utilities::hook::jump(0x14373B0A0_g, lua_unsafe_function_stub); // io_write_file
		utilities::hook::jump(0x14373BD90_g, lua_unsafe_function_stub); // io_tmpfile
		utilities::hook::jump(0x14373C290_g, lua_unsafe_function_stub); // io_popen
		utilities::hook::jump(0x14373BF70_g, lua_unsafe_function_stub); // io_seek_file
		utilities::hook::jump(0x14373C0B0_g, lua_unsafe_function_stub); // io_setvbuf
		utilities::hook::jump(0x14373C1D0_g, lua_unsafe_function_stub); // io_tostring
		utilities::hook::jump(0x143733490_g, lua_unsafe_function_stub); // serialize_persist
		utilities::hook::jump(0x1437336C0_g, lua_unsafe_function_stub); // serialize_unpersist

		utilities::hook::jump(0x143739D70_g, lua_unsafe_function_stub); // havokscript_compiler_settings
		utilities::hook::jump(0x14373A7A0_g, lua_unsafe_function_stub); // havokscript_getgcweights
		utilities::hook::jump(0x14373A230_g, lua_unsafe_function_stub); // havokscript_setgcweights
		utilities::hook::jump(0x143732EF0_g, lua_unsafe_function_stub); // package_loadlib

	}

	int lui_panic_stub(game::lua_state* vm)
	{
		game::hks_object* arg0 = vm->m_apistack.top - 1;

		const char* error_message = "";
		if (arg0 >= vm->m_apistack.base)
		{
			error_message = game::hks_obj_tolstring(vm, arg0, nullptr);

			logger::write(logger::LOG_TYPE_ERROR, std::format("[lui_panic] {}", error_message));
		}
		else {
			logger::write(logger::LOG_TYPE_ERROR, std::format("[lui_panic] empty"));
		}

		game::Lua_CoD_LuaStateManager_Error(100007, error_message, vm);
		return 0;
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (utilities::json_config::ReadBoolean("lua", "info", false))
			{
				utilities::hook::jump(0x143954280_g, lua_engine_print_info);

			}
			if (utilities::json_config::ReadBoolean("lua", "error", false))
			{
				hksi_lual_error_hook.create(0x1437558B0_g, hksi_lual_error_stub);
				lual_error_hook.create(0x14375B540_g, lual_error_stub);
				utilities::hook::jump(0x144238380_g, lui_panic_stub);
				utilities::hook::jump(0x143954210_g, lua_engine_print_error);
				utilities::hook::jump(0x143954300_g, lua_engine_print_warning);
			}

			patch_unsafe_lua_functions();
		}
	};
}
REGISTER_COMPONENT(lua::component)
