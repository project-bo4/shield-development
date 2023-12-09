#pragma once
#include "definitions/game.hpp"

namespace gsc_custom
{
	enum gsic_field_type
	{
		GSIC_FIELD_DETOUR = 0
	};

	struct gsic_detour
	{
		uint32_t fixup_name{};
		uint32_t replace_namespace{};
		uint32_t replace_function{};
		uint32_t fixup_offset{};
		uint32_t fixup_size{};
		uint64_t target_script{};
		byte* fixup_function{};
	};

	struct gsic_info
	{
		bool sync{};
		std::vector<gsic_detour> detours{};
	};

	void sync_gsic(game::scriptInstance_t inst, gsic_info& info);
}