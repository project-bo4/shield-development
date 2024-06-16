#include <std_include.hpp>
#include "version.hpp"
#include "scheduler.hpp"
#include "definitions/game.hpp"
#include "loader/component_loader.hpp"

namespace watermark
{
    namespace
    {
        void draw_version()
        {
            auto* font = reinterpret_cast<void*>(game::sharedUiInfo->assets.bigFont);
            constexpr auto scale = 0.4f;
            constexpr const char* text = "Project-BO4: " VERSION;

            if (!font /*|| *game::keyCatchers & 1*/) return;

            auto screenWidth = game::ScrPlace_GetView(0)->realViewportSize[0];
            auto textWidth = game::UI_TextWidth(0, text, 0x7FFFFFFF, font, scale);

            auto x = screenWidth - (textWidth + 14.0f);
            auto y = game::UI_TextHeight(font, scale) + 12.0f;

            float color[4] = { 0.666f, 0.666f, 0.666f, 0.666f };

            game::R_AddCmdDrawText(text, 0x7FFFFFFF, font, x, y, scale, scale, 0.0f, color, 
                                                                game::ITEM_TEXTSTYLE_NORMAL);
        }
    }

    class component final : public component_interface
    {
    public:
        void post_unpack() override
        {
            scheduler::loop(draw_version, scheduler::renderer);
        }
    };
}

REGISTER_COMPONENT(watermark::component)
