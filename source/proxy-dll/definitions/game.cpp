#include <std_include.hpp>
#include "game.hpp"

namespace game
{
	const char* Com_GetVersionString()
	{
		static std::string version_string{};

		if (version_string.empty()) {
			version_string = std::format("BlackOps4 {}", Com_GetBuildVersion());
		}

		return version_string.data();
	}

	void verify_game_version()
	{
		if (*(int*)0x1449C86B8_g != 27458377) // BlackOps4 CL(27458377) BEYQBBUILD107 DEV [Mon Jun 08 09:12:27 2026]
		{
			throw std::runtime_error("Unsupported BlackOps4.exe Version. This DLL Expects Latest BNET Build");
		}

#ifdef DEBUG
		logger::write(logger::LOG_TYPE_DEBUG, "[ SYSTEM ]: Verified Game Version '%s'", Com_GetVersionString());
#endif // DEBUG
	}

    scoped_critical_section::scoped_critical_section(int32_t s, scoped_critical_section_type type) : _s(0), _hasOwnership(false), _isScopedRelease(false), _next(nullptr)
    {
        game::ScopedCriticalSectionConstructor(this, s, type);
    }

    scoped_critical_section::~scoped_critical_section()
    {
        game::ScopedCriticalSectionDestructor(this);
    }
}