#pragma once

namespace platform
{
	uint64_t bnet_get_userid();
	const char* bnet_get_username();
	std::string get_game_locale();
	std::string get_userdata_directory();
}