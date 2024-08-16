#include <std_include.hpp>
#include "logger.hpp"
#include "game_console.hpp"
#include "loader/component_loader.hpp"

#include <utilities/nt.hpp>

namespace logger
{
	const char* LogTypeNames[] =
	{
		"DEBUG",
		"INFO",
		"WARN",
		"ERROR",
		""
	};

	void write(const int type, std::string str)
	{
		game_console::print(str);
		if (type == type::LOG_TYPE_CONSOLE) return;

		std::stringstream ss;
		ss << "[ " << LogTypeNames[type] << " ] " << str << std::endl;

		std::string text = ss.str();

#ifdef OUTPUT_DEBUG_API
		OutputDebugStringA(text.c_str());
#endif // OUTPUT_DEBUG_API

		printf(text.c_str()); //print debug messages to new console
		std::ofstream fs;
		fs.open("project-bo4.log", std::ios_base::app);

		time_t now = time(0);
		std::tm time;
		localtime_s(&time, &now);

		fs << "" << std::put_time(&time, "%Y-%m-%d %H:%M:%S") << "\t" << text;
	}

	void write(const int type, const char* fmt, ...)
	{
		char va_buffer[0x800] = { 0 };

		va_list ap;
		va_start(ap, fmt);
		vsprintf_s(va_buffer, fmt, ap);
		va_end(ap);

		const auto formatted = std::string(va_buffer);
		write(type, formatted);
	}

	class component final : public component_interface
	{
	public:
		void pre_start() override
		{
#ifdef REMOVE_PREVIOUS_LOG
			utilities::io::remove_file("project-bo4.log");
#endif // REMOVE_PREVIOUS_LOG

			write(LOG_TYPE_INFO, "=======================================================================================================");
			write(LOG_TYPE_INFO, " Project-BO4 Initializing ... %s[0x%llX]", utilities::nt::library{}.get_name().c_str(), utilities::nt::library{}.get_ptr());
			write(LOG_TYPE_INFO, "=======================================================================================================");
		}
	};
}
REGISTER_COMPONENT(logger::component)