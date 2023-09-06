#pragma once

namespace spoofcall
{
	void* get_spoofcall_proxy(const void* funcAddr);

	template <typename T, typename... Args>
	static T invoke(size_t funcAddr, Args ... args)
	{
		return static_cast<T(*)(Args ...)>(get_spoofcall_proxy(reinterpret_cast<void*>(funcAddr)))(args...);
	}

	template <typename T, typename... Args>
	static T invoke(void* funcAddr, Args ... args)
	{
		return static_cast<T(*)(Args ...)>(get_spoofcall_proxy(funcAddr))(args...);
	}
}