#pragma once

namespace logger
{
	enum type
	{
		LOG_TYPE_DEBUG = 0,
		LOG_TYPE_INFO = 1,
		LOG_TYPE_WARN = 2,
		LOG_TYPE_ERROR = 3,
		LOG_TYPE_CONSOLE = 4
	};

	void write(const int type, std::string str);
	void write(const int type, const char* fmt, ...);
}
