#pragma once

#include "../utilities/nt.hpp"

namespace exception
{
	std::string create_minidump(LPEXCEPTION_POINTERS exceptioninfo);
}
