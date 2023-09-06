#pragma once
#include "definitions\game.hpp"

namespace dvars
{
	std::string get_value_string(const game::dvar_t* dvar, game::DvarValue* value);
	std::string get_domain_string(const game::dvarType_t type, const game::DvarLimits& domain);

	game::dvar_t* find_dvar(uint64_t hashRef);
	game::dvar_t* find_dvar(const char* nameRef);
	game::dvar_t* find_dvar(const std::string& nameRef);

}
