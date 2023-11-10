#include <std_include.hpp>
#include "discovery.hpp"
#include "loader/component_loader.hpp"

#include <utilities/hook.hpp>
#include <utilities/signature.hpp>

std::unordered_map<std::string, size_t> symbols_list;

size_t operator"" _d(const char* str, const size_t len)
{
	auto itr = symbols_list.find(std::string(str, len));
	if (itr != symbols_list.end()) return itr->second; // found you!

	//return find_missing_symbol(name);
}

namespace discovery
{
	void export_address_list_to_json()
	{
		/* PLACE_HOLDER */
	}

	void import_address_list_from_json()
	{
		/* PLACE_HOLDER */
	}

	void start_address_list_discovery()
	{
		auto follow_jmp = [](size_t addr) -> size_t {
			return *(int32_t*)(addr + 1) + addr + 5;
		};

		auto follow_lea = [](size_t addr) -> size_t {
			return *(int32_t*)(addr + 3) + addr + 7;
		};

		std::vector<sig_instance> signature_list; // SYMBOL NAME , SIGNATURE, DISTANCE, SIGNATURE RELATION

		// Main Symbols
		signature_list.push_back({ "com_error", "4C 89 4C 24 ? 55 53 56 57 48", 0, SIG_RELEVANCE_DIRECT_HIT });
		signature_list.push_back({ "live_get_connectivity_info", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B DA C7 02", 0, SIG_RELEVANCE_DIRECT_HIT });
		signature_list.push_back({ "draw_text_cmd", "48 89 6C 24 ? 41 54 41 56 41 57 48 83 EC 30 80", 0, SIG_RELEVANCE_DIRECT_HIT });
		signature_list.push_back({ "ui_get_font_handle", "48 89 5C 24 ? 57 48 83 EC 30 8B DA 48 8B F9 83", 0, SIG_RELEVANCE_DIRECT_HIT });
		signature_list.push_back({ "scr_place_get_view", "E8 ? ? ? ? 48 85 C0 74 11 4C 8D", 0, SIG_RELEVANCE_JMP_FROM });
		signature_list.push_back({ "r_text_height", "E8 ? ? ? ? 66 0F 6E C0 0F 5B C0 F3 0F 59 C6 F3 0F", 0, SIG_RELEVANCE_JMP_FROM });

		// Platform Symbols
		signature_list.push_back({ "bnet_is_disabled", "40 55 48 8D 6C 24 ? 48 81 EC ? ? ? ? 45 85", 0x17, SIG_RELEVANCE_JMP_FROM });
		signature_list.push_back({ "bnet_is_connected", "48 83 EC 28 48 8B 0D ? ? ? ? E8 ? ? ? ? 84 C0 74 5A", 0, SIG_RELEVANCE_DIRECT_HIT });
		signature_list.push_back({ "bnet_patch_unk1", "48 83 EC 28 48 8B 0D ? ? ? ? E8 ? ? ? ? 84 C0 75 11", 0, SIG_RELEVANCE_DIRECT_HIT }); // Annoying function related to bnet; crashes game
		signature_list.push_back({ "bnet_patch_unk2", "E8 ? ? ? ? 8B CF E8 ? ? ? ? 45 33 C0 48", 0, SIG_RELEVANCE_JMP_FROM }); // Annoying function related to bnet; crashes game
		signature_list.push_back({ "bnet_patch_unk3", "E8 ? ? ? ? 88 85 ? ? ? ? 48 8B", 0, SIG_RELEVANCE_JMP_FROM }); // BattleNet_IsModeAvailable?
		signature_list.push_back({ "bnet_patch_auth3", "E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 8B CB E8 ? ? ? ? 4C", 0, SIG_RELEVANCE_DIRECT_HIT }); // LiveConnect_BeginCrossAuthPlatform

		// Frame Hooks
		signature_list.push_back({ "r_end_frame", "E8 ? ? ? ? 41 F6 DD 1B", 0, SIG_RELEVANCE_JMP_FROM });
		signature_list.push_back({ "com_frame", "48 83 EC 48 48 C7 44 24 ? ? ? ? ? 48 8D 0D", 0, SIG_RELEVANCE_DIRECT_HIT });
		signature_list.push_back({ "g_run_frame", "48 8B C4 48 89 58 10 48 89 70 18 48 89 78 20 55 41 54 41 55 41 56 41 57 48 8D A8 C8 F7 FF FF", 0, SIG_RELEVANCE_DIRECT_HIT });

		// Demonware 
		signature_list.push_back({ "curl_setup_ssl_verify_peer", "40 53 48 83 EC 20 BA ? ? ? ? 48 8B D9 48 8B", 0x29, SIG_RELEVANCE_DIRECT_HIT });
		signature_list.push_back({ "curl_setup_ssl_verify_host", "40 53 48 83 EC 20 BA ? ? ? ? 48 8B D9 48 8B", 0x15, SIG_RELEVANCE_DIRECT_HIT });
		signature_list.push_back({ "dw_https", "68 74 74 70 73 00", 0, SIG_RELEVANCE_DIRECT_HIT });
		signature_list.push_back({ "dw_prod_umbrella_url", "68 74 74 70 73 3A 2F 2F 70 72 6F 64 2E 75 6D 62 72 65 6C 6C 61 2E 64 65 6D 6F 6E 77 61 72 65 2E 6E 65 74 00", 0, SIG_RELEVANCE_DIRECT_HIT });
		signature_list.push_back({ "dw_prod_uno_url", "68 74 74 70 73 3A 2F 2F 70 72 6F 64 2E 75 6E 6F 2E 64 65 6D 6F 6E 77 61 72 65 2E 6E 65 74 2F 76 31 2E 30 00", 0, SIG_RELEVANCE_DIRECT_HIT });
		signature_list.push_back({ "dw_auth3_url_frmt", "68 74 74 70 73 3A 2F 2F 25 73 3A 25 64 2F 61 75 74 68 2F 00", 0, SIG_RELEVANCE_DIRECT_HIT });

		// BuildNumber
		signature_list.push_back({ "com_get_build_version", "40 53 48 83 EC 40 44 8B 0D", 0, SIG_RELEVANCE_DIRECT_HIT });
		
		// UI Symbols
		signature_list.push_back({ "ui_get_model_for_controller", "48 63 C1 48 8D 0D ? ? ? ? 0F B7 04", 0, SIG_RELEVANCE_DIRECT_HIT });
		signature_list.push_back({ "ui_create_model_from_path", "45 33 C9 41 B0 01 E9", 0, SIG_RELEVANCE_DIRECT_HIT });
		signature_list.push_back({ "ui_model_set_string", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 8B DA 49 8B F1", 0x4E, SIG_RELEVANCE_JMP_FROM });
		signature_list.push_back({ "ui_model_set_int", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 8B DA 49 8B F1", 0x20, SIG_RELEVANCE_JMP_FROM });
		signature_list.push_back({ "ui_model_set_bool", "48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 8B DA 49 8B F1", 0x31, SIG_RELEVANCE_JMP_FROM });
		signature_list.push_back({ "ui_model_set_real", "48 83 EC 28 66 85 C9 74 3A", 0, SIG_RELEVANCE_DIRECT_HIT });


		//// Impossible to make signature; should be updated manually
		//signature_list.push_back({ "bnet_process_auth3_data", "", 0x000000000_g, SIG_RELEVANCE_IMPOSSIBLE });
		//signature_list.push_back({ "bnet_patch_text_chat", "", 0x000000000_g, SIG_RELEVANCE_IMPOSSIBLE });


		logger::write(logger::LOG_TYPE_DEBUG, "[ DISCOVERY ]: Starting signature scan; total defined symbols: %u", signature_list.size());

		symbols_list.clear();
		int error_count = 0;

		for (sig_instance i : signature_list)
		{
			if (i.relv == SIG_RELEVANCE_IMPOSSIBLE)
			{
				symbols_list.insert({ i.name, i.dist });
				continue;
			}

			utilities::hook::signature::signature_result scan = utilities::hook::signature(std::string(i.sig, strlen(i.sig))).process();

			if (scan.size() == 0 || scan.size() > 1)
			{
				logger::write(logger::LOG_TYPE_DEBUG, "[ DISCOVERY ]: %s while searching for %s", scan.size() ? "Multiple Matches" : "No Hits", i.name);
				error_count++;
				continue;
			}

			if (i.relv == SIG_RELEVANCE_DIRECT_HIT)
			{
				symbols_list.insert({ i.name, reinterpret_cast<size_t>(scan[0]) + i.dist });
			}
			else if (i.relv == SIG_RELEVANCE_JMP_FROM)
			{
				symbols_list.insert({ i.name, follow_jmp(reinterpret_cast<size_t>(scan[0]) + i.dist) });
			}
			else
			{
				symbols_list.insert({ i.name, follow_lea(reinterpret_cast<size_t>(scan[0]) + i.dist) });
			}
		}

#ifdef DEBUG
		logger::write(logger::LOG_TYPE_DEBUG, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

		if (symbols_list.find("com_get_build_version") != symbols_list.end())
		{
			const char* build_version = utilities::hook::invoke<const char*>(symbols_list["com_get_build_version"]);
			logger::write(logger::LOG_TYPE_DEBUG, "Address-List Discovery Results for BlackOps4 %s", build_version);
		}

		for (auto symbol : symbols_list)
		{
			logger::write(logger::LOG_TYPE_DEBUG, "- %-28s: 0x%llX_g", symbol.first.c_str(), reverse_g(symbol.second));
		}

		logger::write(logger::LOG_TYPE_DEBUG, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
#endif // DEBUG

		logger::write(logger::LOG_TYPE_DEBUG, "[ DISCOVERY ]: Signature scanning complete. %u/%u successful", signature_list.size() - error_count, signature_list.size());
	}
	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
#ifdef DEBUG
			start_address_list_discovery();
#endif // DEBUG

			/*************************************************************************************************************
			** NOTE : Updating game code by developers depending on compiler and where and what changes made, most likely 
			**      will shift addresses. using signature patterns to find addresses at runtime is a good counter to this
			**      problem when game gets updated frequently but there should be decent fail-safe mechanism implemented 
			**      to detect sig-scanning errors when there is signature-breaking changes in binary to prevent misleadings
			**
			*************************************************************************************************************/
		}

		int priority() override
		{
			return 9998;
		}
	};
}

//REGISTER_COMPONENT(discovery::component)
