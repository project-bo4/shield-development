#include <std_include.hpp>
#include "logger.hpp"
#include "loader/component_loader.hpp"
#include <utils/nt.hpp>

#define OUTPUT_DEBUG_API
#define PREPEND_TIMESTAMP

namespace logger
{
	const char* LogTypeNames[] =
	{
		"DEBUG",
		"INFO",
		"WARN",
		"ERROR"
	};

	void write(const int type, std::string str)
	{
#ifndef _DEBUG
		if (type == LOG_TYPE_DEBUG) return;
#endif // _DEBUG

#ifdef OUTPUT_DEBUG_API
		OutputDebugStringA(str.c_str());
#endif // OUTPUT_DEBUG_API

		std::ofstream stream;
		stream.open("project-bo4.log", std::ios_base::app);

#ifdef PREPEND_TIMESTAMP
		time_t now = time(0);
		std::tm* t = std::localtime(&now);
		stream << "" << std::put_time(t, "%Y-%m-%d %H:%M:%S") << "\t";
#endif // PREPEND_TIMESTAMP

		stream << "[ " << LogTypeNames[type] << " ] " << str << std::endl;
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

	namespace
	{
		/* PLACE_HOLDER */
	}

	class component final : public component_interface
	{
	public:
		void pre_start() override
		{
#ifdef REMOVE_PREVIOUS_LOG
			utils::io::remove_file("project-bo4.log");
#endif // REMOVE_PREVIOUS_LOG

			write(LOG_TYPE_INFO,  "=======================================================================================================");
			write(LOG_TYPE_INFO,  " Project-BO4 Initializing ... %s[0x%llX]", utils::nt::library{}.get_name().c_str(), utils::nt::library{}.get_ptr());
			write(LOG_TYPE_INFO,  "=======================================================================================================");
		}

		void post_unpack() override
		{
			/* PLACE_HOLDER */
		}
	};
}
REGISTER_COMPONENT(logger::component)