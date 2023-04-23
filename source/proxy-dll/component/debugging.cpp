#include <std_include.hpp>
#include "component/config.hpp"
#include "loader/component_loader.hpp"
#include "definitions/t8_engine.hpp"
#include "scheduler.hpp"


namespace debugging
{
	static bool should_draw_debugging_info = false;
	typedef short(__fastcall* UI_Model_GetModelForController_t)(int controllerIndex);
	UI_Model_GetModelForController_t UI_Model_GetModelForController = (UI_Model_GetModelForController_t)0x143AD0200_g;

	typedef short(__fastcall* UI_Model_CreateModelFromPath_t)(short parentNodeIndex, const char* path);
	UI_Model_CreateModelFromPath_t UI_Model_CreateModelFromPath = (UI_Model_CreateModelFromPath_t)0x143ACFC10_g;

	typedef bool(__fastcall* UI_Model_SetString_t)(short nodeIndex, const char* newValue);
	UI_Model_SetString_t UI_Model_SetString = (UI_Model_SetString_t)0x143AD18C0_g;

	typedef bool(__fastcall* UI_Model_SetInt_t)(short nodeIndex, int newValue);
	UI_Model_SetInt_t UI_Model_SetInt = (UI_Model_SetInt_t)0x143AD1820_g;

	typedef bool(__fastcall* UI_Model_SetBool_t)(short nodeIndex, bool newValue);
	UI_Model_SetBool_t UI_Model_SetBool = (UI_Model_SetBool_t)0x143AD1780_g;

	typedef bool(__fastcall* UI_Model_SetReal_t)(short nodeIndex, float newValue);
	UI_Model_SetReal_t UI_Model_SetReal = (UI_Model_SetReal_t)0x143AD1870_g;

	void LUI_ShowToast(const char* title, const char* desc, const char* icon)
	{
		short main_model = UI_Model_GetModelForController(0);
		short toast_model = UI_Model_CreateModelFromPath(main_model, "FrontendToast");

		short sub_model = UI_Model_CreateModelFromPath(toast_model, "state");
		UI_Model_SetString(sub_model, "DefaultState");

		sub_model = UI_Model_CreateModelFromPath(toast_model, "kicker");
		UI_Model_SetString(sub_model, title);

		sub_model = UI_Model_CreateModelFromPath(toast_model, "description");
		UI_Model_SetString(sub_model, desc);

		sub_model = UI_Model_CreateModelFromPath(toast_model, "contentIcon");
		UI_Model_SetString(sub_model, icon);

		sub_model = UI_Model_CreateModelFromPath(toast_model, "functionIcon");
		UI_Model_SetString(sub_model, "blacktransparent");

		sub_model = UI_Model_CreateModelFromPath(toast_model, "backgroundId");
		UI_Model_SetInt(sub_model, 0);

		sub_model = UI_Model_CreateModelFromPath(toast_model, "emblemDecal");
		UI_Model_SetReal(sub_model, 0.000000);

		sub_model = UI_Model_CreateModelFromPath(toast_model, "notify");
		UI_Model_SetBool(sub_model, true);
	}

	namespace
	{
		const char* s_connectivityNames[] =
		{
			"user is non-guest", // 0x1
			"connected to live", // 0x2
			"user has multiplayer privs", // 0x4
			"networking initialized", // 0x8
			"connected to demonware", // 0x10
			"lpc ready", // 0x20
			"retrieved ffotd", // 0x40
			"retrieved playlists", // 0x80
			"publisher variables inited", // 0x100
			"ffotd is valid", // 0x200
			"user has stats and loadouts", // 0x400
			"time is synced", // 0x800
			"retrieved geo location", // 0x1000
			"dedicated pings done", // 0x2000
			"dedicated ping responses ok", // 0x4000
			"literally unlisted", // 0x8000
			"unknown - lpc related", // 0x10000
			"inventory fetched", // 0x20000
			"marketing messages received", // 0x40000
			"bnet initialized", // 0x80000
			"achievements fetched" // 0x100000
		};

		std::string GetConnectivityInfo()
		{
			int infoBitmask = 0; int requiredMask = 0x1337FA;
			game::Live_GetConnectivityInformation(0, &infoBitmask);
			bool connected = (requiredMask & infoBitmask) == requiredMask;

			std::string result{};
			//result.append(std::format("Can play online (controller: {}): {}\n", 0, connected ? "true" : "false"));

			for (int i = 1; i < 21; ++i)
			{
				if (i == 15) continue; // unlisted bit

				const char* v13;
				const char* v14;
				const char* v15;

				if (((1 << i) & infoBitmask) != 0 || (requiredMask & (1 << i)) == 0)
					v13 = "^7";
				else
					v13 = "^1";
				if ((requiredMask & (1 << i)) != 0)
					v14 = "required";
				else
					v14 = "optional";
				if (((1 << i) & infoBitmask) != 0)
					v15 = "true";
				else
					v15 = "false";

				result.append(std::format("{}{}({}) - {}\n", v13, s_connectivityNames[i], v14, v15));
			}

			return result;
		}

		void draw_debug_info()
		{
			if (GetAsyncKeyState(VK_INSERT) & 0x01) should_draw_debugging_info ^= 1;

			if (!should_draw_debugging_info) return;


			float color[4] = { 0.666f, 0.666f, 0.666f, 1.0f };
			game::ScreenPlacement* scrPlace = game::ScrPlace_GetView(0);
			void* font = game::UI_GetFontHandle(scrPlace, 0, 1.0f); if (!font) return;

			std::string sz = GetConnectivityInfo();
			game::R_AddCmdDrawText(sz.data(), 0x7FFFFFFF, font, 18.0f, 1.0f * (game::R_TextHeight(font) * 0.45f) + 4.0f, 0.45f, 0.45f, 0.0f, color, game::ITEM_TEXTSTYLE_BORDERED);

		}

		void test_key_catcher()
		{
			static uint32_t last_press_time = 0;
			if ((GetAsyncKeyState(VK_HOME) & 0x01)/* && (static_cast<uint32_t>(time(nullptr)) - last_press_time) > 1*/)
			{
				last_press_time = static_cast<uint32_t>(time(nullptr));
				LUI_ShowToast("Title", "Description", "uie_bookmark");
			}
		}
	}

class component final : public component_interface
{
public:
	void pre_start() override
	{
		config::register_config_value("debug_info", "false", "Draw debugging info");

		const std::string& dcfg = config::get_config_value("debug_info");

		should_draw_debugging_info = dcfg != config::noconfig() && dcfg == "true";
	}
	void post_unpack() override
	{	
		scheduler::loop(draw_debug_info, scheduler::renderer);
		scheduler::loop(test_key_catcher, scheduler::main);
	}
};
}

REGISTER_COMPONENT(debugging::component)