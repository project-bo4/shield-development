#pragma once
#include <std_include.hpp>
#include "definitions/game.hpp"

namespace gsc_funcs
{
	constexpr auto serious_custom_func_name = "SeriousCustom";

	extern bool enable_dev_func;

	uint32_t canon_hash(const char* str);
	uint32_t canon_hash_pattern(const char* str);
	
	void gsc_error(const char* message, game::scriptInstance_t inst, bool terminal, ...);
	const char* lookup_hash(game::scriptInstance_t inst, const char* type, uint64_t hash);

	void ScrVm_AddToArrayIntIndexed(game::scriptInstance_t inst, uint64_t index);
}