#include <std_include.hpp>
#include "loader/component_loader.hpp"
#include <utils/hook.hpp>


namespace demonware
{
	const char* blocked_hosts[] = 
	{
		"eu.cdn.blizzard.com",
		"level3.blizzard.com",
		"blzddist1-a.akamaihd.net",
		"level3.ssl.blizzard.com",
		"eu.actual.battle.net"
	};

	namespace
	{
		std::unordered_map<void*, void*> original_imports{};

		namespace network
		{
			int getaddrinfo_stub(const char* name, const char* service,
				const addrinfo* hints, addrinfo** res)
			{
#ifdef DEBUG
				logger::write(logger::LOG_TYPE_DEBUG, "[ NETWORK ]: [getaddrinfo]: \"%s\" \"%s\"", name, service);
#endif

				for (auto i = 0; i < ARRAYSIZE(blocked_hosts); ++i)
				{
					if (!strcmp(name, blocked_hosts[i]))
					{
						return WSAHOST_NOT_FOUND;
					}
				}

				return WSAHOST_NOT_FOUND;
				/* TODO: RE-ROUTE DW HOSTS TO CUSTOM DW SERVER */

				return getaddrinfo(name, service, hints, res);
			}

			hostent* gethostbyname_stub(const char* name)
			{
#ifdef DEBUG
				logger::write(logger::LOG_TYPE_DEBUG, "[ NETWORK ]: [gethostbyname]: \"%s\"", name);
#endif

#pragma warning(push)
#pragma warning(disable: 4996)
				return gethostbyname(name);
#pragma warning(pop)
			}
		}

		void register_hook(const std::string& process, void* stub)
		{
			const utils::nt::library game_module{};

			std::optional<std::pair<void*, void*>> result{};
			if (!result) result = utils::hook::iat(game_module, "wsock32.dll", process, stub);
			if (!result) result = utils::hook::iat(game_module, "WS2_32.dll", process, stub);

			if (!result)
			{
				throw std::runtime_error("Failed to hook: " + process);
			}

			original_imports[result->first] = result->second;
		}
	}

	class component final : public component_interface
	{
	public:
		component()
		{
			/* PLACE_HOLDER */
		}

		void pre_start() override
		{
			register_hook("gethostbyname", network::gethostbyname_stub);
			register_hook("getaddrinfo", network::getaddrinfo_stub);
		}

		void post_unpack() override
		{
			utils::hook::set<uint8_t>(0x144508469_g, 0x0); // CURLOPT_SSL_VERIFYPEER
			utils::hook::set<uint8_t>(0x144508455_g, 0xAF); // CURLOPT_SSL_VERIFYHOST
			utils::hook::set<uint8_t>(0x144B28D98_g, 0x0); // HTTPS -> HTTP

			utils::hook::copy_string(0x144A27C70_g, "http://prod.umbrella.demonware.net");
			utils::hook::copy_string(0x144A2BAA0_g, "http://prod.uno.demonware.net/v1.0");
			utils::hook::copy_string(0x144A29CB0_g, "http://%s:%d/auth/");


			/*************************************************************************************************************
			** TODO : in order to record match, while playing (as host?) game live-streams netcode to the content server
			**        continuously troughout the play time. planning to patch it so it streams in memory before uploading 
			**        full demo at end of match to improve network performance
			**
			**
			*************************************************************************************************************/
		}

		void pre_destroy() override
		{
			for (const auto& import : original_imports)
			{
				utils::hook::set(import.first, import.second);
			}
		}
	};
}

REGISTER_COMPONENT(demonware::component)