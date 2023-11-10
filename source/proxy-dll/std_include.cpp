#include <std_include.hpp>
#include <utilities/nt.hpp>

extern "C"
{
	int s_read_arc4random(void*, size_t)
	{
		return -1;
	}

	int s_read_getrandom(void*, size_t)
	{
		return -1;
	}

	int s_read_urandom(void*, size_t)
	{
		return -1;
	}

	int s_read_ltm_rng(void*, size_t)
	{
		return -1;
	}
}


size_t get_base()
{
	static auto base = size_t(utilities::nt::library{}.get_ptr());
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
