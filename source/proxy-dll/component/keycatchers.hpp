#pragma once

namespace keycatchers
{
	using ke_typedef = std::function<bool(int, int, bool, unsigned int)>;
	using ch_typedef = std::function<bool(int, int, bool)>;

	void add_key_event(ke_typedef&& func);
	void add_char_event(ch_typedef&& func);
}