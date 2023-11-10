#include <std_include.hpp>
#include "definitions/game.hpp"
#include "component/scheduler.hpp"
#include "loader/component_loader.hpp"

#include <utilities/string.hpp>

namespace debugging
{
	namespace
	{
		const char* get_connectivity_info_string(int infoBitmask)
		{
			char connectionInfoString[64];

			for (int bitNumber = 0; bitNumber < 21; bitNumber++)
			{
				if ((1 << bitNumber) & infoBitmask)
				{
					connectionInfoString[bitNumber * 2] = bitNumber + 0x41;
				}
				else
				{
					connectionInfoString[bitNumber * 2] = 0x2D;
				}

				connectionInfoString[(bitNumber * 2) + 1] = 0x2E;
			}

			connectionInfoString[42] = NULL;

			return utilities::string::va("%s", connectionInfoString);
		}
		
		void draw_debug_info()
		{
			int infoBitmask = 0; int requiredMask = 0x1337FA;
			game::Live_GetConnectivityInformation(0, &infoBitmask);
			bool connected = (requiredMask & infoBitmask) == requiredMask;

			void* font = reinterpret_cast<void*>(game::sharedUiInfo->assets.bigFont); if (!font) return;

			if (!connected) 
			{
				float color[4] = { 0.8f, 1.0f, 0.3, 0.8f };

				const char* sz = get_connectivity_info_string(infoBitmask);

				game::ScreenPlacement* scrPlace = game::ScrPlace_GetView(0);

				float offset_x = scrPlace->realViewportSize[0] - 8.0f
					- game::UI_TextWidth(0, sz, 0x7FFFFFFF, font, 0.45f);
				float offset_y = scrPlace->realViewportSize[1] - 8.0f;

				game::R_AddCmdDrawText(sz, 0x7FFFFFFF, font, offset_x, offset_y, 0.45f, 0.45f, 0.0f, color, game::ITEM_TEXTSTYLE_BORDERED);
			}
		}

		void test_key_catcher()
		{
			static uint32_t last_press_time = 0;
			if ((GetAsyncKeyState(VK_HOME) & 0x01)/* && (static_cast<uint32_t>(time(nullptr)) - last_press_time) > 1*/)
			{
				last_press_time = static_cast<uint32_t>(time(nullptr));

            	/* ACTION_PLACE_HOLDER */
			}
		}
	}

	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			scheduler::loop(draw_debug_info, scheduler::renderer);
			scheduler::loop(test_key_catcher, scheduler::main);
		}
	};
}

REGISTER_COMPONENT(debugging::component)