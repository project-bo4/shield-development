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
			
		utilities::hook::jump(0x1437358D0_g, lua_unsafe_function_stub); // base_loadfile
		utilities::hook::jump(0x143736A50_g, lua_unsafe_function_stub); // base_load
		utilities::hook::jump(0x14373B640_g, lua_unsafe_function_stub); // string_dump
		

		// io helpers
		utilities::hook::jump(0x14373F000_g, lua_unsafe_function_stub); // os_exit
		utilities::hook::jump(0x14373F020_g, lua_unsafe_function_stub); // os_remove
		utilities::hook::jump(0x14373F150_g, lua_unsafe_function_stub); // os_rename
		utilities::hook::jump(0x14373EEC0_g, lua_unsafe_function_stub); // os_tmpname
		utilities::hook::jump(0x14373EE90_g, lua_unsafe_function_stub); // os_sleep
		utilities::hook::jump(0x14373ED30_g, lua_unsafe_function_stub); // os_execute
		utilities::hook::jump(0x14373ED90_g, lua_unsafe_function_stub); // os_getenv

		utilities::hook::jump(0x14373D170_g, lua_unsafe_function_stub); // io_close
		utilities::hook::jump(0x14373D060_g, lua_unsafe_function_stub); // io_close_file
		utilities::hook::jump(0x14373D030_g, lua_unsafe_function_stub); // io_flush
		utilities::hook::jump(0x14375C7B0_g, lua_unsafe_function_stub); // io_output/io_input
		utilities::hook::jump(0x14373D7B0_g, lua_unsafe_function_stub); // io_lines
		utilities::hook::jump(0x14373D670_g, lua_unsafe_function_stub); // io_lines
		utilities::hook::jump(0x14373DD20_g, lua_unsafe_function_stub); // io_open
		utilities::hook::jump(0x14373D5B0_g, lua_unsafe_function_stub); // io_read
		utilities::hook::jump(0x14373D200_g, lua_unsafe_function_stub); // io_read_file
		utilities::hook::jump(0x14373DA90_g, lua_unsafe_function_stub); // io_type
		utilities::hook::jump(0x14373CFB0_g, lua_unsafe_function_stub); // io_write
		utilities::hook::jump(0x14373CF70_g, lua_unsafe_function_stub); // io_write_file
		utilities::hook::jump(0x14373DC60_g, lua_unsafe_function_stub); // io_tmpfile
		utilities::hook::jump(0x14373E160_g, lua_unsafe_function_stub); // io_popen
		utilities::hook::jump(0x14373DE40_g, lua_unsafe_function_stub); // io_seek_file
		utilities::hook::jump(0x14373DF80_g, lua_unsafe_function_stub); // io_setvbuf
		utilities::hook::jump(0x14373E0A0_g, lua_unsafe_function_stub); // io_tostring
		utilities::hook::jump(0x143735360_g, lua_unsafe_function_stub); // serialize_persist
		utilities::hook::jump(0x143735590_g, lua_unsafe_function_stub); // serialize_unpersist

		utilities::hook::jump(0x14373BC40_g, lua_unsafe_function_stub); // havokscript_compiler_settings
		utilities::hook::jump(0x14373C670_g, lua_unsafe_function_stub); // havokscript_getgcweights
		utilities::hook::jump(0x14373C100_g, lua_unsafe_function_stub); // havokscript_setgcweights
		utilities::hook::jump(0x143734DC0_g, lua_unsafe_function_stub); // package_loadlib

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
				utilities::hook::jump(0x143956010_g, lua_engine_print_info);

			}
			if (utilities::json_config::ReadBoolean("lua", "error", false))
			{
				hksi_lual_error_hook.create(0x143757780_g, hksi_lual_error_stub);
				lual_error_hook.create(0x14375D410_g, lual_error_stub);
				utilities::hook::jump(0x14423A1D0_g, lui_panic_stub);
				utilities::hook::jump(0x143955FA0_g, lua_engine_print_error);
				utilities::hook::jump(0x143956090_g, lua_engine_print_warning);
			}

			patch_unsafe_lua_functions();
		}
	};
}
REGISTER_COMPONENT(lua::component)