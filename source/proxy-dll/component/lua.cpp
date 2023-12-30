#include <std_include.hpp>
#include "definitions/game.hpp"
#include "loader/component_loader.hpp"

#include <utilities/json_config.hpp>
#include <utilities/hook.hpp>

namespace lua {

	utilities::hook::detour lual_error_hook;
	utilities::hook::detour hksi_lual_error_hook;

	void lual_error_stub(void* state, const char* fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		char buffer[0x800];

		vsprintf_s(buffer, fmt, va);
		va_end(va);

		logger::write(logger::LOG_TYPE_ERROR, std::format("[lual_error] {}", buffer));
		lual_error_hook.invoke<void>(state, "%s", buffer);
	}

	void hksi_lual_error_stub(void* state, const char* fmt, ...)
	{
		va_list va;
		va_start(va, fmt);
		char buffer[0x800];

		vsprintf_s(buffer, fmt, va);
		va_end(va);

		logger::write(logger::LOG_TYPE_ERROR, std::format("[hksi_lual_error] {}", buffer));
		hksi_lual_error_hook.invoke<void>(state, "%s", buffer);
	}

	void ui_interface_error(const char* error)
	{
		logger::write(logger::LOG_TYPE_ERROR, "[ui_interface_error] %s", error);
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

	void print_info(game::consoleLabel_e label, const char* info)
	{
		print_out(logger::LOG_TYPE_INFO, label, info);
	}

	void print_warning(game::consoleLabel_e label, const char* info)
	{
		print_out(logger::LOG_TYPE_WARN, label, info);
	}

	void print_error(game::consoleLabel_e label, const char* info)
	{
		print_out(logger::LOG_TYPE_ERROR, label, info);
	}

	int lua_unsafe_function_stub(void* state)
	{
		std::once_flag f{};

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

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			if (utilities::json_config::ReadBoolean("lua", "info", false))
			{
				utilities::hook::jump(0x143A96E10_g, print_info);

			}
			if (utilities::json_config::ReadBoolean("lua", "error", false))
			{
				hksi_lual_error_hook.create(0x143757780_g, hksi_lual_error_stub);
				utilities::hook::jump(0x1439B4CF0_g, ui_interface_error);
				lual_error_hook.create(0x14375D410_g, lual_error_stub);
				utilities::hook::jump(0x143A96E00_g, print_error);
				utilities::hook::jump(0x143A96E30_g, print_warning);
			}

			patch_unsafe_lua_functions();
		}
	};
}
REGISTER_COMPONENT(lua::component)