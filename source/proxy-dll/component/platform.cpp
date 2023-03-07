#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include "utils/hook.hpp"
#include "WinReg.hpp"

namespace platform
{
	namespace
	{
		//utils::hook::detour BattleNet_API_RequestAppTicket_Hook;
		//bool BattleNet_API_RequestAppTicket_stub(char* sessionToken, char* accountToken)
		//{
		//   /* PLACE_HOLDER */
		//}

		utils::hook::detour PC_TextChat_Print_Hook;
		void PC_TextChat_Print_Stub(const char* text)
		{
			logger::write(logger::LOG_TYPE_DEBUG, "PC_TextChat_Print(%s)", text);
		}

		void check_platform_registry()
		{
			winreg::RegKey key;
			winreg::RegResult result = key.TryOpen(HKEY_CURRENT_USER, L"SOFTWARE\\Blizzard Entertainment\\Battle.net");
			if (!result)
			{
				MessageBoxA(nullptr, "You need to have BlackOps4 from Battle.Net to use this product...", "Error", MB_ICONWARNING);
				ShellExecuteA(nullptr, "open", "http://battle.net/", nullptr, nullptr, SW_SHOWNORMAL);
				logger::write(logger::LOG_TYPE_INFO, "[ PLATFORM ]: Couldnt find Battle.Net Launcher; Shutting down...");

				TerminateProcess(GetCurrentProcess(), 1);
			}
		}
	}

	class component final : public component_interface
	{
	public:
		void pre_start() override
		{
			check_platform_registry();
		}

		void post_unpack() override
		{
			utils::hook::set<uint16_t>(0x1423271D0_g, 0x01B0); // BattleNet_IsDisabled (patch to mov al,1)
			utils::hook::set<uint32_t>(0x1423271E0_g, 0x90C301B0); // BattleNet_IsConnected (patch to mov al,1 retn)

			utils::hook::set<uint8_t>(0x142325210_g, 0xC3); // patch#1 Annoying function crashing game; related to BattleNet (TODO : Needs Further Investigation)
			utils::hook::set<uint8_t>(0x142307B40_g, 0xC3); // patch#2 Annoying function crashing game; related to BattleNet (TODO : Needs Further Investigation)
			utils::hook::set<uint32_t>(0x143D08290_g, 0x90C301B0); // patch#3 BattleNet_IsModeAvailable? (patch to mov al,1 retn)

			utils::hook::nop(0x1437DA454_g, 13); // begin cross-auth even without platform being initialized
			utils::hook::set(0x1444E34C0_g, 0xC301B0); // Checks extended_data and extra_data in json object [bdAuthPC::processPlatformData]

			//PC_TextChat_Print_Hook.create(0x000000000_g, PC_TextChat_Print_Stub); // Disable useless system messages passed into chat box
			//BattleNet_API_RequestAppTicket_Hook.create(0x000000000_g, BattleNet_API_RequestAppTicket_stub); // Implement custom encryption token
		}
		
		int priority() override
		{
			return 9996;
		}
	};
}

REGISTER_COMPONENT(platform::component)