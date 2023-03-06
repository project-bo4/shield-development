#include <std_include.hpp>
#include <utils/nt.hpp>

size_t get_base()
{
	static auto base = size_t(utils::nt::library{}.get_ptr());
	assert(base && "Failed to resolve base");
	return base;
}


size_t operator"" _b(const size_t val)
{
	return get_base() + val;
}

size_t reverse_b(const size_t val)
{
	return val - get_base();
}

size_t reverse_b(const void* val)
{
	return reverse_b(reinterpret_cast<size_t>(val));
}


size_t operator"" _g(const size_t val)
{
	static auto base = get_base();
	return base + (val - 0x140000000);
}

size_t reverse_g(const size_t val)
{
	static auto base = get_base();
	return (val - base) + 0x140000000;
}

size_t reverse_g(const void* val)
{
	return reverse_g(reinterpret_cast<size_t>(val));
}
