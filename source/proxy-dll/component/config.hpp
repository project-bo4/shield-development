#pragma once
#include <string>

namespace config
{
	const std::string& noconfig();
	void register_config_value(const std::string& key, const std::string& default_value, const char* description);
	const std::string& get_config_value(const std::string& cfg);
}