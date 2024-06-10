#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "definitions/game.hpp"
#include "version.hpp"
#include "scheduler.hpp"
#include <utilities/hook.hpp>

#define R_DrawTextFont reinterpret_cast<void*>(game::sharedUiInfo->assets.bigFont)

namespace branding
{
    namespace
    {
        void draw_branding()
        {
            constexpr auto x = 8; // 4 in boiii   //  dont like it overlapping in game console
            constexpr auto y = 28; // 0 in boiii  //  looks ugly
            constexpr auto scale = 0.45f;
            float color[4] = { 0.666f, 0.666f, 0.666f, 0.666f };

            if (!R_DrawTextFont) return;

            game::R_AddCmdDrawText("Shield: " VERSION, std::numeric_limits<int>::max(), R_DrawTextFont, static_cast<float>(x),
                y + static_cast<float>(75.0) * scale,
                scale, scale, 0.0f, color, game::ITEM_TEXTSTYLE_NORMAL);
        }
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            scheduler::loop(draw_branding, scheduler::renderer);

            // Change window title prefix 
            //utilities::hook::copy_string(0x144989ED8_g, "Shield");  // correct address just displays chinese 

        }
    };
}
REGISTER_COMPONENT(branding::component)
