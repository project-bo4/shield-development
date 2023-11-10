#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utilities/hook.hpp>
#include <utilities/string.hpp>


namespace arxan
{
	namespace
	{
		const std::vector<std::pair<uint8_t*, size_t>>& get_text_sections()
		{
			static const std::vector<std::pair<uint8_t*, size_t>> text = []
			{
				std::vector<std::pair<uint8_t*, size_t>> texts{};

				const utilities::nt::library game{};
				for (const auto& section : game.get_section_headers())
				{
					if (section->Characteristics & IMAGE_SCN_MEM_EXECUTE)
					{
						texts.emplace_back(game.get_ptr() + section->VirtualAddress, section->Misc.VirtualSize);
					}
				}

				return texts;
			}();

			return text;
		}

		bool is_in_texts(const uint64_t addr)
		{
			const auto& texts = get_text_sections();
			for (const auto& text : texts)
			{
				const auto start = reinterpret_cast<ULONG_PTR>(text.first);
				if (addr >= start && addr <= (start + text.second))
				{
					return true;
				}
			}

			return false;
		}

		bool is_in_texts(const void* addr)
		{
			return is_in_texts(reinterpret_cast<uint64_t>(addr));
		}

		struct integrity_handler_context
		{
			uint32_t* computed_checksum;
			uint32_t* original_checksum;
		};

		bool is_on_stack(uint8_t* stack_frame, const void* pointer)
		{
			const auto stack_value = reinterpret_cast<uint64_t>(stack_frame);
			const auto pointer_value = reinterpret_cast<uint64_t>(pointer);

			const auto diff = static_cast<int64_t>(stack_value - pointer_value);
			return std::abs(diff) < 0x1000;
		}

		// Pretty trashy, but working, heuristic to search the integrity handler context
		bool is_handler_context(uint8_t* stack_frame, const uint32_t computed_checksum, const uint32_t frame_offset)
		{
			const auto* potential_context = reinterpret_cast<integrity_handler_context*>(stack_frame + frame_offset);
			return is_on_stack(stack_frame, potential_context->computed_checksum)
				&& *potential_context->computed_checksum == computed_checksum
				&& is_in_texts(potential_context->original_checksum);
		}

		integrity_handler_context* search_handler_context(uint8_t* stack_frame, const uint32_t computed_checksum)
		{
			for (uint32_t frame_offset = 0; frame_offset < 0x90; frame_offset += 8)
			{
				if (is_handler_context(stack_frame, computed_checksum, frame_offset))
				{
					return reinterpret_cast<integrity_handler_context*>(stack_frame + frame_offset);
				}
			}

			return nullptr;
		}

		uint32_t adjust_integrity_checksum(const uint64_t return_address, uint8_t* stack_frame,
			const uint32_t current_checksum)
		{
			const auto handler_address = reverse_b(return_address - 5);
			const auto* context = search_handler_context(stack_frame, current_checksum);

			if (!context)
			{
				MessageBoxA(nullptr, utilities::string::va("No frame offset for: %llX", handler_address), "Error",
					MB_ICONERROR);
				TerminateProcess(GetCurrentProcess(), 0xBAD);
				return current_checksum;
			}

			const auto correct_checksum = *context->original_checksum;
			*context->computed_checksum = correct_checksum;

			if (current_checksum != correct_checksum)
			{
#ifndef NDEBUG
				/*logger::write(logger::LOG_TYPE_DEBUG, "Adjusting checksum (%llX): %X -> %X\n", handler_address,
					   current_checksum, correct_checksum);*/
#endif
			}

			return correct_checksum;
		}

		void patch_intact_basic_block_integrity_check(void* address)
		{
			const auto game_address = reinterpret_cast<uint64_t>(address);
			constexpr auto inst_len = 3;

			const auto next_inst_addr = game_address + inst_len;
			const auto next_inst = *reinterpret_cast<uint32_t*>(next_inst_addr);

			if ((next_inst & 0xFF00FFFF) != 0xFF004583)
			{
				throw std::runtime_error(utilities::string::va("Unable to patch intact basic block: %llX", game_address));
			}

			const auto other_frame_offset = static_cast<uint8_t>(next_inst >> 16);
			static const auto stub = utilities::hook::assemble([](utilities::hook::assembler& a)
				{
					a.push(rax);

					a.mov(rax, qword_ptr(rsp, 8));
					a.sub(rax, 2); // Skip the push we inserted

					a.push(rax);
					a.pushad64();

					a.mov(r8, qword_ptr(rsp, 0x88));
					a.mov(rcx, rax);
					a.mov(rdx, rbp);
					a.call_aligned(adjust_integrity_checksum);

					a.mov(qword_ptr(rsp, 0x80), rax);

					a.popad64();
					a.pop(rax);

					a.add(rsp, 8);

					a.mov(dword_ptr(rdx, rcx, 4), eax);

					a.pop(rax); // return addr
					a.xchg(rax, qword_ptr(rsp)); // switch with push

					a.add(dword_ptr(rbp, rax), 0xFFFFFFFF);

					a.mov(rax, dword_ptr(rdx, rcx, 4)); // restore rax

					a.ret();
				});

			// push other_frame_offset
			utilities::hook::set<uint16_t>(game_address, static_cast<uint16_t>(0x6A | (other_frame_offset << 8)));
			utilities::hook::call(game_address + 2, stub);
		}

		void patch_split_basic_block_integrity_check(void* address)
		{
			const auto game_address = reinterpret_cast<uint64_t>(address);
			constexpr auto inst_len = 3;

			const auto next_inst_addr = game_address + inst_len;

			if (*reinterpret_cast<uint8_t*>(next_inst_addr) != 0xE9)
			{
				throw std::runtime_error(utilities::string::va("Unable to patch split basic block: %llX", game_address));
			}

			const auto jump_target = utilities::hook::extract<void*>(reinterpret_cast<void*>(next_inst_addr + 1));
			const auto stub = utilities::hook::assemble([jump_target](utilities::hook::assembler& a)
				{
					a.push(rax);

					a.mov(rax, qword_ptr(rsp, 8));
					a.push(rax);

					a.pushad64();

					a.mov(r8, qword_ptr(rsp, 0x88));
					a.mov(rcx, rax);
					a.mov(rdx, rbp);
					a.call_aligned(adjust_integrity_checksum);

					a.mov(qword_ptr(rsp, 0x80), rax);

					a.popad64();
					a.pop(rax);

					a.add(rsp, 8);

					a.mov(dword_ptr(rdx, rcx, 4), eax);

					a.add(rsp, 8);

					a.jmp(jump_target);
				});

			utilities::hook::call(game_address, stub);
		}

		void search_and_patch_integrity_checks()
		{
			// There seem to be 1219 results.
			// Searching them is quite slow.
			// Maybe precomputing that might be better?
			const auto intact_results = "89 04 8A 83 45 ? FF"_sig;
			const auto split_results = "89 04 8A E9"_sig;

			for (auto* i : intact_results)
			{
				patch_intact_basic_block_integrity_check(i);
			}

			for (auto* i : split_results)
			{
				patch_split_basic_block_integrity_check(i);
			}
		}

		void* original_first_tls_callback = nullptr;

		void** get_tls_callbacks()
		{
			const utilities::nt::library game{};
			const auto& entry = game.get_optional_header()->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
			if (!entry.VirtualAddress || !entry.Size)
			{
				return nullptr;
			}

			const auto* tls_dir = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(game.get_ptr() + entry.VirtualAddress);
			return reinterpret_cast<void**>(tls_dir->AddressOfCallBacks);
		}

		void disable_tls_callbacks()
		{
			auto* tls_callbacks = get_tls_callbacks();
			if (tls_callbacks)
			{
				original_first_tls_callback = *tls_callbacks;
			}

			utilities::hook::set(tls_callbacks, nullptr);
		}

		void restore_tls_callbacks()
		{
			auto* tls_callbacks = get_tls_callbacks();
			if (tls_callbacks)
			{
				utilities::hook::set(tls_callbacks, original_first_tls_callback);
			}
		}

		utilities::hook::detour create_thread_hook;
		HANDLE WINAPI create_thread_stub(const LPSECURITY_ATTRIBUTES thread_attributes, const SIZE_T stack_size,
			const LPTHREAD_START_ROUTINE start_address, const LPVOID parameter,
			const DWORD creation_flags,
			const LPDWORD thread_id)
		{
			if (utilities::nt::library::get_by_address(start_address) == utilities::nt::library{})
			{
				restore_tls_callbacks();

				create_thread_hook.clear();
				return CreateThread(thread_attributes, stack_size, start_address, parameter, creation_flags,
					thread_id);
			}

			return create_thread_hook.invoke<HANDLE>(thread_attributes, stack_size, start_address, parameter,
				creation_flags, thread_id);
		}

		utilities::hook::detour get_thread_context_hook;
		BOOL WINAPI get_thread_context_stub(const HANDLE thread_handle, const LPCONTEXT context)
		{
			constexpr auto debug_registers_flag = (CONTEXT_DEBUG_REGISTERS & ~CONTEXT_AMD64);
			if (context && (context->ContextFlags & debug_registers_flag))
			{
				auto* source = _ReturnAddress();
				const auto game = utilities::nt::library{};
				const auto source_module = utilities::nt::library::get_by_address(source);

				if (source_module == game)
				{
					context->ContextFlags &= ~debug_registers_flag;
				}
			}

			return get_thread_context_hook.invoke<BOOL>(thread_handle, context);
		}

		utilities::hook::detour create_mutex_ex_a_hook;
		HANDLE create_mutex_ex_a_stub(const LPSECURITY_ATTRIBUTES attributes, const LPCSTR name, const DWORD flags,
			const DWORD access)
		{
			if (name == "$ IDA trusted_idbs"s || name == "$ IDA registry mutex $"s)
			{
				return nullptr;
			}

			return create_mutex_ex_a_hook.invoke<HANDLE>(attributes, name, flags, access);
		}

		bool remove_evil_keywords_from_string(const UNICODE_STRING& string)
		{
			static const std::wstring evil_keywords[] =
			{
				L"IDA",
				L"ida",
				L"HxD",
				L"cheatengine",
				L"Cheat Engine",
				L"x96dbg",
				L"x32dbg",
				L"x64dbg",
				L"Wireshark",
			};

			if (!string.Buffer || !string.Length)
			{
				return false;
			}

			const std::wstring_view path(string.Buffer, string.Length / sizeof(string.Buffer[0]));

			bool modified = false;
			for (const auto& keyword : evil_keywords)
			{
				while (true)
				{
					const auto pos = path.find(keyword);
					if (pos == std::wstring::npos)
					{
						break;
					}

					modified = true;

					for (size_t i = 0; i < keyword.size(); ++i)
					{
						string.Buffer[pos + i] = L'a';
					}
				}
			}

			return modified;
		}

		bool remove_evil_keywords_from_string(wchar_t* str, const size_t length)
		{
			UNICODE_STRING unicode_string{};
			unicode_string.Buffer = str;
			unicode_string.Length = static_cast<uint16_t>(length);
			unicode_string.MaximumLength = unicode_string.Length;

			return remove_evil_keywords_from_string(unicode_string);
		}

		bool remove_evil_keywords_from_string(char* str, const size_t length)
		{
			std::string_view str_view(str, length);
			std::wstring wstr(str_view.begin(), str_view.end());

			if (!remove_evil_keywords_from_string(wstr.data(), wstr.size()))
			{
				return false;
			}

			const std::string regular_str(wstr.begin(), wstr.end());
			memcpy(str, regular_str.data(), length);

			return true;
		}

		int WINAPI get_window_text_a_stub(const HWND wnd, const LPSTR str, const int max_count)
		{
			std::wstring wstr{};
			wstr.resize(max_count);

			const auto res = GetWindowTextW(wnd, wstr.data(), max_count);
			if (res)
			{
				remove_evil_keywords_from_string(wstr.data(), res);

				const std::string regular_str(wstr.begin(), wstr.end());
				memset(str, 0, max_count);
				memcpy(str, regular_str.data(), res);
			}

			return res;
		}

		utilities::hook::detour nt_query_system_information_hook;
		NTSTATUS NTAPI nt_query_system_information_stub(const SYSTEM_INFORMATION_CLASS system_information_class,
			const PVOID system_information,
			const ULONG system_information_length,
			const PULONG return_length)
		{
			const auto status = nt_query_system_information_hook.invoke<NTSTATUS>(
				system_information_class, system_information, system_information_length, return_length);

			if (NT_SUCCESS(status))
			{
				if (system_information_class == SystemProcessInformation && !utilities::nt::is_shutdown_in_progress())
				{
					auto addr = static_cast<uint8_t*>(system_information);
					while (true)
					{
						const auto info = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(addr);
						remove_evil_keywords_from_string(info->ImageName);

						if (!info->NextEntryOffset)
						{
							break;
						}

						addr = addr + info->NextEntryOffset;
					}
				}
			}

			return status;
		}

		utilities::hook::detour nt_query_information_process_hook;
		NTSTATUS WINAPI nt_query_information_process_stub(const HANDLE handle, const PROCESSINFOCLASS info_class,
			const PVOID info,
			const ULONG info_length, const PULONG ret_length)
		{
			NTSTATUS status = nt_query_information_process_hook.invoke<NTSTATUS>(handle, info_class, info, info_length,
				ret_length);

			if (NT_SUCCESS(status))
			{
				if (info_class == ProcessImageFileName || static_cast<int>(info_class) == 43 /*ProcessImageFileNameWin32*/)
				{
					remove_evil_keywords_from_string(*static_cast<UNICODE_STRING*>(info));
				}
			}

			return status;
		}
	}

	class component final : public component_interface
	{
	public:
		void pre_start() override
		{
			disable_tls_callbacks();

			create_thread_hook.create(CreateThread, create_thread_stub);
			auto* get_thread_context_func = utilities::nt::library("kernelbase.dll").get_proc<void*>("GetThreadContext");
			get_thread_context_hook.create(get_thread_context_func, get_thread_context_stub);

			create_mutex_ex_a_hook.create(CreateMutexExA, create_mutex_ex_a_stub);

			utilities::hook::copy(this->window_text_buffer_, GetWindowTextA, sizeof(this->window_text_buffer_));
			utilities::hook::jump(GetWindowTextA, get_window_text_a_stub, true, true);
			utilities::hook::move_hook(GetWindowTextA);

			const utilities::nt::library ntdll("ntdll.dll");

			const auto nt_query_information_process = ntdll.get_proc<void*>("NtQueryInformationProcess");
			nt_query_information_process_hook.create(nt_query_information_process,
				nt_query_information_process_stub);

			const auto nt_query_system_information = ntdll.get_proc<void*>("NtQuerySystemInformation");
			nt_query_system_information_hook.create(nt_query_system_information, nt_query_system_information_stub);
			nt_query_system_information_hook.move();


			/*************************************************************************************************************
			** TODO : There is some kind of dormant defence mechanism. works so random makes it harder to investigate
			**        It will Exit process with code zero in case gets induced mid-game or just prevent initialization
			**        causing to get black screened if tripped boot-time. Apparently it verifies IAT's integrity.
			**
			**
			*************************************************************************************************************/
		}

		void post_unpack() override
		{
			search_and_patch_integrity_checks();
		}

		int priority() override
		{
			return 9999;
		}

	private:
		uint8_t window_text_buffer_[15]{};
	};
}

REGISTER_COMPONENT(arxan::component)