#pragma once

namespace logger
{
	enum type
	{
		LOG_TYPE_DEBUG = 0,
		LOG_TYPE_INFO = 1,
		LOG_TYPE_WARN = 2,
		LOG_TYPE_ERROR = 3
	};

	void set_log_level(type level);
	type get_level_from_name(const char* name);
	const char* get_level_name(type level);
	void write(const int type, std::string str);
	void write(const int type, const char* fmt, ...);
}
