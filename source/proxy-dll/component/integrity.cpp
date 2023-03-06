#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/string.hpp>


namespace integrity
{
	namespace
	{	
#ifndef AVOID_UNNECESSARY_CHANGES

		/* PLACE_HOLDER */

#endif // AVOID_UNNECESSARY_CHANGES
		const std::vector<std::pair<uint8_t*, size_t>>& get_text_sections()
		{
			static const std::vector<std::pair<uint8_t*, size_t>> text = []
			{
				std::vector<std::pair<uint8_t*, size_t>> texts{};

				const utils::nt::library game{};
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
				MessageBoxA(nullptr, utils::string::va("No frame offset for: %llX", handler_address), "Error",
				            MB_ICONERROR);
				TerminateProcess(GetCurrentProcess(), 0xBAD);
				return current_checksum;
			}

			const auto correct_checksum = *context->original_checksum;
			*context->computed_checksum = correct_checksum;

			if (current_checksum != correct_checksum)
			{
#ifndef NDEBUG
				/*printf("Adjusting checksum (%llX): %X -> %X\n", handler_address,
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
				throw std::runtime_error(utils::string::va("Unable to patch intact basic block: %llX", game_address));
			}

			const auto other_frame_offset = static_cast<uint8_t>(next_inst >> 16);
			static const auto stub = utils::hook::assemble([](utils::hook::assembler& a)
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
			utils::hook::set<uint16_t>(game_address, static_cast<uint16_t>(0x6A | (other_frame_offset << 8)));
			utils::hook::call(game_address + 2, stub);
		}

		void patch_split_basic_block_integrity_check(void* address)
		{
			const auto game_address = reinterpret_cast<uint64_t>(address);
			constexpr auto inst_len = 3;

			const auto next_inst_addr = game_address + inst_len;

			if (*reinterpret_cast<uint8_t*>(next_inst_addr) != 0xE9)
			{
				throw std::runtime_error(utils::string::va("Unable to patch split basic block: %llX", game_address));
			}

			const auto jump_target = utils::hook::extract<void*>(reinterpret_cast<void*>(next_inst_addr + 1));
			const auto stub = utils::hook::assemble([jump_target](utils::hook::assembler& a)
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

			utils::hook::call(game_address, stub);
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
			const utils::nt::library game{};
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

			utils::hook::set(tls_callbacks, nullptr);
		}

		void restore_tls_callbacks()
		{
			auto* tls_callbacks = get_tls_callbacks();
			if (tls_callbacks)
			{
				utils::hook::set(tls_callbacks, original_first_tls_callback);
			}
		}

		utils::hook::detour create_thread_hook;
		HANDLE WINAPI create_thread_stub(const LPSECURITY_ATTRIBUTES thread_attributes, const SIZE_T stack_size,
			const LPTHREAD_START_ROUTINE start_address, const LPVOID parameter,
			const DWORD creation_flags,
			const LPDWORD thread_id)
		{
			if (utils::nt::library::get_by_address(start_address) == utils::nt::library{})
			{
				restore_tls_callbacks();

				create_thread_hook.clear();
				return CreateThread(thread_attributes, stack_size, start_address, parameter, creation_flags,
					thread_id);
			}

			return create_thread_hook.invoke<HANDLE>(thread_attributes, stack_size, start_address, parameter,
				creation_flags, thread_id);
		}

		utils::hook::detour get_thread_context_hook;
		BOOL WINAPI get_thread_context_stub(const HANDLE thread_handle, const LPCONTEXT context)
		{
			constexpr auto debug_registers_flag = (CONTEXT_DEBUG_REGISTERS & ~CONTEXT_AMD64);
			if (context && (context->ContextFlags & debug_registers_flag))
			{
				auto* source = _ReturnAddress();
				const auto game = utils::nt::library{};
				const auto source_module = utils::nt::library::get_by_address(source);

				if (source_module == game)
				{
					context->ContextFlags &= ~debug_registers_flag;
				}
			}

			return get_thread_context_hook.invoke<BOOL>(thread_handle, context);
		}
	}
	
	class component final : public component_interface
	{
	public:
		void pre_start() override
		{
#ifndef AVOID_UNNECESSARY_CHANGES
			disable_tls_callbacks();

			create_thread_hook.create(CreateThread, create_thread_stub);
			auto* get_thread_context_func = utils::nt::library("kernelbase.dll").get_proc<void*>("GetThreadContext");
			get_thread_context_hook.create(get_thread_context_func, get_thread_context_stub);

#endif // AVOID_UNNECESSARY_CHANGES

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
	};
}

REGISTER_COMPONENT(integrity::component)