#include <std_include.hpp>
#include "platform.hpp"
#include "definitions/game.hpp"
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>
#include <utils/identity.hpp>
#include <utils/json_config.hpp>
#include <utils/cryptography.hpp>
#include <WinReg.hpp>

namespace platform
{
	uint64_t bnet_get_userid()
	{
		static uint64_t userid = 0;
		if (!userid)
		{
			uint32_t default_xuid = utils::cryptography::xxh32::compute(utils::identity::get_sys_username());
			userid = utils::json_config::ReadUnsignedInteger64("identity", "xuid", default_xuid);
		}

		return userid;
	}

	const char* bnet_get_username()
	{
		static std::string username{};
		if (username.empty())
		{
			std::string default_name = utils::identity::get_sys_username();
			username = utils::json_config::ReadString("identity", "name", default_name);
		}

		return username.data();
	}

	std::string get_userdata_directory()
	{
		return std::format("players/bnet-{}", bnet_get_userid());
	}

	namespace
	{
		utils::hook::detour PC_TextChat_Print_Hook;
		void PC_TextChat_Print_Stub(const char* text)
		{
#ifdef DEBUG
			logger::write(logger::LOG_TYPE_DEBUG, "PC_TextChat_Print(%s)", text);
#endif
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

			utils::hook::nop(0x1437DA454_g, 13); // begin cross-auth even without platform being initialized [LiveConnect_BeginCrossAuthPlatform]
			utils::hook::set(0x1444D2D60_g, 0xC301B0); // Auth3 Response RSA signature check [bdAuth::validateResponseSignature]
			utils::hook::set(0x1444E34C0_g, 0xC301B0); // Auth3 Response platform extended data check [bdAuthPC::processPlatformData]

			utils::hook::nop(0x1438994E9_g, 22); // get live name even without platform being initialized [Live_UserSignedIn]
			utils::hook::nop(0x1438C3476_g, 22); // get live xuid even without platform being initialized [LiveUser_UserGetXuid]

			utils::hook::jump(0x142325C70_g, bnet_get_username); // detour battlenet username
			utils::hook::jump(0x142325CA0_g, bnet_get_userid); // detour battlenet userid

			//PC_TextChat_Print_Hook.create(0x1422D4A20_g, PC_TextChat_Print_Stub); // Disable useless system messages passed into chat box
			
			logger::write(logger::LOG_TYPE_INFO, "[ PLATFORM ]: BattleTag: '%s', BattleID: '%llu'", bnet_get_username(), bnet_get_userid());
		}
	};
}

REGISTER_COMPONENT(platform::component)