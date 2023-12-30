#pragma once

namespace game::runtime_errors
{
	constexpr uint64_t custom_error_id = 0x42693201;
	const char* get_error_message(uint64_t code);
}