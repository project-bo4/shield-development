#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utilities/io.hpp>
#include <utilities/hook.hpp>
#include <utilities/thread.hpp>

#include "demonware/servers/lobby_server.hpp"
#include "demonware/servers/auth3_server.hpp"
#include "demonware/servers/stun_server.hpp"
#include "demonware/servers/umbrella_server.hpp"
#include "demonware/servers/fileshare_server.hpp"
#include "demonware/server_registry.hpp"

#define TCP_BLOCKING true
#define UDP_BLOCKING false

namespace demonware
{
	const char* blocked_hosts[] =
	{
		"us.cdn.blizzard.com",
		"us.actual.battle.net",
		"eu.cdn.blizzard.com",
		"eu.actual.battle.net",
		"level3.blizzard.com",
		"blzddist1-a.akamaihd.net",
		"level3.ssl.blizzard.com"
	};

	namespace
	{
		std::atomic_bool exit_server{ false };
		std::thread server_thread{};
		utilities::concurrency::container<std::unordered_map<SOCKET, bool>> blocking_sockets{};
		utilities::concurrency::container<std::unordered_map<SOCKET, tcp_server*>> socket_map{};
		server_registry<tcp_server> tcp_servers{};
		server_registry<udp_server> udp_servers{};
		std::unordered_map<void*, void*> original_imports{};

		tcp_server* find_server(const SOCKET socket)
		{
			return socket_map.access<tcp_server*>([&](const std::unordered_map<SOCKET, tcp_server*>& map) -> tcp_server*
				{
					const auto entry = map.find(socket);
					if (entry == map.end())
					{
						return nullptr;
					}

					return entry->second;
				});
		}

		bool socket_link(const SOCKET socket, const uint32_t address)
		{
			auto* server = tcp_servers.find(address);
			if (!server)
			{
				return false;
			}

			socket_map.access([&](std::unordered_map<SOCKET, tcp_server*>& map)
				{
					map[socket] = server;
				});

			return true;
		}

		void socket_unlink(const SOCKET socket)
		{
			socket_map.access([&](std::unordered_map<SOCKET, tcp_server*>& map)
				{
					const auto entry = map.find(socket);
					if (entry != map.end())
					{
						map.erase(entry);
					}
				});
		}

		bool is_socket_blocking(const SOCKET socket, const bool def)
		{
			return blocking_sockets.access<bool>([&](std::unordered_map<SOCKET, bool>& map)
				{
					const auto entry = map.find(socket);
					if (entry == map.end())
					{
						return def;
					}

					return entry->second;
				});
		}

		void remove_blocking_socket(const SOCKET socket)
		{
			blocking_sockets.access([&](std::unordered_map<SOCKET, bool>& map)
				{
					const auto entry = map.find(socket);
					if (entry != map.end())
					{
						map.erase(entry);
					}
				});
		}

		void add_blocking_socket(const SOCKET socket, const bool block)
		{
			blocking_sockets.access([&](std::unordered_map<SOCKET, bool>& map)
				{
					map[socket] = block;
				});
		}

		void server_main()
		{
			exit_server = false;

			while (!exit_server)
			{
				tcp_servers.frame();
				udp_servers.frame();
				std::this_thread::sleep_for(50ms);
			}
		}

		namespace io
		{
			int getaddrinfo_stub(const char* name, const char* service,
				const addrinfo* hints, addrinfo** res)
			{
#ifndef NDEBUG
				logger::write(logger::LOG_TYPE_DEBUG, "[ network ]: [getaddrinfo]: \"%s\" \"%s\"", name, service);
#endif

				for (auto i = 0; i < ARRAYSIZE(blocked_hosts); ++i)
				{
					if (!strcmp(name, blocked_hosts[i]))
					{
						return WSAHOST_NOT_FOUND;
					}
				}

				base_server* server = tcp_servers.find(name);
				if (!server)
				{
					server = udp_servers.find(name);
				}

				if (!server)
				{
					return getaddrinfo(name, service, hints, res);
				}

				const auto address = utilities::memory::get_allocator()->allocate<sockaddr>();
				const auto ai = utilities::memory::get_allocator()->allocate<addrinfo>();

				auto in_addr = reinterpret_cast<sockaddr_in*>(address);
				in_addr->sin_addr.s_addr = server->get_address();
				in_addr->sin_family = AF_INET;

				ai->ai_family = AF_INET;
				ai->ai_socktype = SOCK_STREAM;
				ai->ai_addr = address;
				ai->ai_addrlen = sizeof(sockaddr);
				ai->ai_next = nullptr;
				ai->ai_flags = 0;
				ai->ai_protocol = 0;
				ai->ai_canonname = const_cast<char*>(name);

				*res = ai;

				return 0;
			}

			void freeaddrinfo_stub(addrinfo* ai)
			{
				if (!utilities::memory::get_allocator()->find(ai))
				{
					return freeaddrinfo(ai);
				}

				utilities::memory::get_allocator()->free(ai->ai_addr);
				utilities::memory::get_allocator()->free(ai);
			}

			int getpeername_stub(const SOCKET s, sockaddr* addr, socklen_t* addrlen)
			{
				auto* server = find_server(s);

				if (server)
				{
					auto in_addr = reinterpret_cast<sockaddr_in*>(addr);
					in_addr->sin_addr.s_addr = server->get_address();
					in_addr->sin_family = AF_INET;
					*addrlen = sizeof(sockaddr);

					return 0;
				}

				return getpeername(s, addr, addrlen);
			}

			int getsockname_stub(const SOCKET s, sockaddr* addr, socklen_t* addrlen)
			{
				auto* server = find_server(s);

				if (server)
				{
					auto in_addr = reinterpret_cast<sockaddr_in*>(addr);
					in_addr->sin_addr.s_addr = server->get_address();
					in_addr->sin_family = AF_INET;
					*addrlen = sizeof(sockaddr);

					return 0;
				}

				return getsockname(s, addr, addrlen);
			}

			hostent* gethostbyname_stub(const char* name)
			{
#ifndef NDEBUG
				logger::write(logger::LOG_TYPE_DEBUG, "[ network ]: [gethostbyname]: \"%s\"", name);
#endif

				base_server* server = tcp_servers.find(name);
				if (!server)
				{
					server = udp_servers.find(name);
				}

				if (!server)
				{
#pragma warning(push)
#pragma warning(disable: 4996)
					return gethostbyname(name);
#pragma warning(pop)
				}

				static thread_local in_addr address{};
				address.s_addr = server->get_address();

				static thread_local in_addr* addr_list[2]{};
				addr_list[0] = &address;
				addr_list[1] = nullptr;

				static thread_local hostent host{};
				host.h_name = const_cast<char*>(name);
				host.h_aliases = nullptr;
				host.h_addrtype = AF_INET;
				host.h_length = sizeof(in_addr);
				host.h_addr_list = reinterpret_cast<char**>(addr_list);

				return &host;
			}

			int connect_stub(const SOCKET s, const struct sockaddr* addr, const int len)
			{
				if (len == sizeof(sockaddr_in))
				{
					const auto* in_addr = reinterpret_cast<const sockaddr_in*>(addr);
					if (socket_link(s, in_addr->sin_addr.s_addr)) return 0;
				}

				return connect(s, addr, len);
			}

			int closesocket_stub(const SOCKET s)
			{
				remove_blocking_socket(s);
				socket_unlink(s);

				return closesocket(s);
			}

			int send_stub(const SOCKET s, const char* buf, const int len, const int flags)
			{
				auto* server = find_server(s);

				if (server)
				{
					server->handle_input(buf, len);
					return len;
				}

				return send(s, buf, len, flags);
			}

			int recv_stub(const SOCKET s, char* buf, const int len, const int flags)
			{
				auto* server = find_server(s);

				if (server)
				{
					if (server->pending_data())
					{
						return static_cast<int>(server->handle_output(buf, len));
					}
					else
					{
						WSASetLastError(WSAEWOULDBLOCK);
						return -1;
					}
				}

				return recv(s, buf, len, flags);
			}

			int sendto_stub(const SOCKET s, const char* buf, const int len, const int flags, const sockaddr* to,
				const int tolen)
			{
				const auto* in_addr = reinterpret_cast<const sockaddr_in*>(to);
				auto* server = udp_servers.find(in_addr->sin_addr.s_addr);

				if (server)
				{
					server->handle_input(buf, len, { s, to, tolen });
					return len;
				}

				return sendto(s, buf, len, flags, to, tolen);
			}

			int recvfrom_stub(const SOCKET s, char* buf, const int len, const int flags, struct sockaddr* from,
				int* fromlen)
			{
				// Not supported yet
				if (is_socket_blocking(s, UDP_BLOCKING))
				{
					return recvfrom(s, buf, len, flags, from, fromlen);
				}

				size_t result = 0;
				udp_servers.for_each([&](udp_server& server)
					{
						if (server.pending_data(s))
						{
							result = server.handle_output(
								s, buf, static_cast<size_t>(len), from, fromlen);
						}
					});

				if (result)
				{
					return static_cast<int>(result);
				}

				return recvfrom(s, buf, len, flags, from, fromlen);
			}

			int select_stub(const int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds,
				struct timeval* timeout)
			{
				if (exit_server)
				{
					return select(nfds, readfds, writefds, exceptfds, timeout);
				}

				auto result = 0;
				std::vector<SOCKET> read_sockets;
				std::vector<SOCKET> write_sockets;

				socket_map.access([&](std::unordered_map<SOCKET, tcp_server*>& sockets)
					{
						for (auto& s : sockets)
						{
							if (readfds)
							{
								if (FD_ISSET(s.first, readfds))
								{
									if (s.second->pending_data())
									{
										read_sockets.push_back(s.first);
										FD_CLR(s.first, readfds);
									}
								}
							}

							if (writefds)
							{
								if (FD_ISSET(s.first, writefds))
								{
									write_sockets.push_back(s.first);
									FD_CLR(s.first, writefds);
								}
							}

							if (exceptfds)
							{
								if (FD_ISSET(s.first, exceptfds))
								{
									FD_CLR(s.first, exceptfds);
								}
							}
						}
					});

				if ((!readfds || readfds->fd_count == 0) && (!writefds || writefds->fd_count == 0))
				{
					timeout->tv_sec = 0;
					timeout->tv_usec = 0;
				}

				result = select(nfds, readfds, writefds, exceptfds, timeout);
				if (result < 0) result = 0;

				for (const auto& socket : read_sockets)
				{
					if (readfds)
					{
						FD_SET(socket, readfds);
						result++;
					}
				}

				for (const auto& socket : write_sockets)
				{
					if (writefds)
					{
						FD_SET(socket, writefds);
						result++;
					}
				}

				return result;
			}

			int ioctlsocket_stub(const SOCKET s, const long cmd, u_long* argp)
			{
				if (static_cast<unsigned long>(cmd) == (FIONBIO))
				{
					add_blocking_socket(s, *argp == 0);
				}

				return ioctlsocket(s, cmd, argp);
			}
		}

		void register_hook(const std::string& process, void* stub)
		{
			const utilities::nt::library game_module{};

			std::optional<std::pair<void*, void*>> result{};
			if (!result) result = utilities::hook::iat(game_module, "wsock32.dll", process, stub);
			if (!result) result = utilities::hook::iat(game_module, "WS2_32.dll", process, stub);

			if (!result)
			{
				throw std::runtime_error("Failed to hook: " + process);
			}

			original_imports[result->first] = result->second;
		}

		void check_lpc_files()
		{
			if (!utilities::io::file_exists("LPC/.manifest") || !utilities::io::file_exists("LPC/core_ffotd_tu23_639_cf92ecf4a75d3f79.ff") || !utilities::io::file_exists("LPC/core_playlists_tu23_639_cf92ecf4a75d3f79.ff"))
			{
				MessageBoxA(nullptr, "some required LPC files seems to be missing. You need to get and place them manually since this emulator doesnt host and provide those files; read instructions in github documentation for more info.",
					"LPC Files Missing", MB_ICONERROR);
			}
		}
	}

	class component final : public component_interface
	{
	public:
		component()
		{
			udp_servers.create<stun_server>("ops4-stun.us.demonware.net");
			udp_servers.create<stun_server>("ops4-stun.eu.demonware.net");
			udp_servers.create<stun_server>("ops4-stun.jp.demonware.net");
			udp_servers.create<stun_server>("ops4-stun.au.demonware.net");

			tcp_servers.create<auth3_server>("ops4-pc-auth3.prod.demonware.net");
			tcp_servers.create<lobby_server>("ops4-pc-lobby.prod.demonware.net");
			tcp_servers.create<umbrella_server>("prod.umbrella.demonware.net");
			tcp_servers.create<fileshare_server>("ops4-fileshare.prod.schild.net");
		}

		void pre_start() override
		{
			register_hook("send", io::send_stub);
			register_hook("recv", io::recv_stub);
			register_hook("sendto", io::sendto_stub);
			register_hook("recvfrom", io::recvfrom_stub);
			register_hook("connect", io::connect_stub);
			register_hook("select", io::select_stub);
			register_hook("closesocket", io::closesocket_stub);
			register_hook("ioctlsocket", io::ioctlsocket_stub);
			register_hook("gethostbyname", io::gethostbyname_stub);
			register_hook("getaddrinfo", io::getaddrinfo_stub);
			register_hook("freeaddrinfo", io::freeaddrinfo_stub);
			register_hook("getpeername", io::getpeername_stub);
			register_hook("getsockname", io::getsockname_stub);

			check_lpc_files();
		}

		void post_unpack() override
		{
			server_thread = utilities::thread::create_named_thread("Demonware", server_main);

			utilities::hook::set<uint8_t>(0x144508469_g, 0x0); // CURLOPT_SSL_VERIFYPEER
			utilities::hook::set<uint8_t>(0x144508455_g, 0xAF); // CURLOPT_SSL_VERIFYHOST
			utilities::hook::set<uint8_t>(0x144B28D98_g, 0x0); // HTTPS -> HTTP

			utilities::hook::copy_string(0x144A27C70_g, "http://prod.umbrella.demonware.net");
			utilities::hook::copy_string(0x144A2BAA0_g, "http://prod.uno.demonware.net/v1.0");
			utilities::hook::copy_string(0x144A29CB0_g, "http://%s:%d/auth/");
			
			utilities::hook::set(0x142DD0E10_g, 0xC301B0); // Live_Qos_Finished
			utilities::hook::set(0x1438C2C70_g, 0xC301B0); // Live_Contracts? related to bdUNK125
		}

		void pre_destroy() override
		{
			exit_server = true;
			if (server_thread.joinable())
			{
				server_thread.join();
			}

			for (const auto& import : original_imports)
			{
				utilities::hook::set(import.first, import.second);
			}
		}
	};
}

REGISTER_COMPONENT(demonware::component)