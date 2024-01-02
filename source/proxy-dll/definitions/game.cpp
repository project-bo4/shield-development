#include <std_include.hpp>
#include "game.hpp"
#include "loader/component_loader.hpp"

namespace game
{
    std::string version_string = "VERSION STRING UN-INITIALIZED";

    typedef const char* (__fastcall* Com_GetBuildVersion_t)();
    Com_GetBuildVersion_t Com_GetBuildVersion = (Com_GetBuildVersion_t)0x142892F40_g;


    scoped_critical_section::scoped_critical_section(int32_t s, scoped_critical_section_type type) : _s(0), _hasOwnership(false), _isScopedRelease(false), _next(nullptr)
    {
        game::ScopedCriticalSectionConstructor(this, s, type);
    }

    scoped_critical_section::~scoped_critical_section()
    {
        game::ScopedCriticalSectionDestructor(this);
    }

    namespace
    {
        void verify_game_version()
        {
            if (*(int*)0x1449CA7E8_g != 13869365) // BlackOps4 CL(13869365) BEYQBBUILD106 DEV [Wed Feb 22 16:31:32 2023]
            {
                throw std::runtime_error("Unsupported BlackOps4.exe Version. Update Your game using Battle.net Launcher");
            }

            version_string = std::format("BlackOps4 {}", Com_GetBuildVersion());

#ifdef DEBUG
            logger::write(logger::LOG_TYPE_DEBUG, "[ SYSTEM ]: game version string: %s", version_string.c_str());
#endif // DEBUG
        }
    }

    class component final : public component_interface
    {
    public:
        void pre_start() override
        {
            /* PLACE_HOLDER */
        }

        void post_unpack() override
        {
            verify_game_version();
        }

        int priority() override
        {
            return 9997;
        }
    };
}

REGISTER_COMPONENT(game::component)