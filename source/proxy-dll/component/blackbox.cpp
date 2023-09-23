#include <std_include.hpp>
#include "definitions/game.hpp"
#include "loader/component_loader.hpp"

#include <utils/hook.hpp>
#include <utils/io.hpp>
#include <utils/string.hpp>
#include <utils/thread.hpp>
#include <utils/compression.hpp>
#include <exception/minidump.hpp>

namespace blackbox
{
	namespace
	{
		DWORD main_thread_id{};

		thread_local struct
		{
			DWORD code = 0;
			PVOID address = nullptr;
		} exception_data{};

		struct
		{
			std::chrono::time_point<std::chrono::high_resolution_clock> last_recovery{};
			std::atomic<int> recovery_counts = {0};
		} recovery_data{};

		bool is_game_thread()
		{
			return main_thread_id == GetCurrentThreadId();
		}

		bool is_exception_interval_too_short()
		{
			const auto delta = std::chrono::high_resolution_clock::now() - recovery_data.last_recovery;
			return delta < 1min;
		}

		bool too_many_exceptions_occured()
		{
			return recovery_data.recovery_counts >= 3;
		}

		volatile bool& is_initialized()
		{
			static volatile bool initialized = false;
			return initialized;
		}

		bool is_recoverable()
		{
			return is_initialized()
				&& is_game_thread()
				&& !is_exception_interval_too_short()
				&& !too_many_exceptions_occured();
		}

		void show_mouse_cursor()
		{
			while (ShowCursor(TRUE) < 0);
		}

		void display_error_dialog()
		{
			const std::string error_str = utils::string::va("Fatal error (0x%08X) at 0x%p (0x%p).\n"
			                                                "A minidump has been written.\n",
			                                                exception_data.code, exception_data.address,
				                                            reverse_b(reinterpret_cast<uint64_t>(exception_data.address)));

			utils::thread::suspend_other_threads();
			show_mouse_cursor();

			MessageBoxA(nullptr, error_str.data(), "Project-BO4 ERROR", MB_ICONERROR);
			TerminateProcess(GetCurrentProcess(), exception_data.code);
		}

		void reset_state()
		{
			if (is_recoverable())
			{
				recovery_data.last_recovery = std::chrono::high_resolution_clock::now();
				++recovery_data.recovery_counts;

				game::Com_Error(game::ERR_DROP, "Fatal error (0x%08X) at 0x%p (0x%p).\nA minidump has been written.\n\n"
				                "Project-BO4 has tried to recover your game, but it might not run stable anymore.\n\n"
				                "Make sure to update your graphics card drivers and install operating system updates!\n"
				                "Closing or restarting Steam might also help.",
				                exception_data.code, exception_data.address,
					            reverse_b(reinterpret_cast<uint64_t>(exception_data.address)));
			}
			else
			{
				display_error_dialog();
			}
		}

		size_t get_reset_state_stub()
		{
			static auto* stub = utils::hook::assemble([](utils::hook::assembler& a)
			{
				a.sub(rsp, 0x10);
				a.or_(rsp, 0x8);
				a.jmp(reset_state);
			});

			return reinterpret_cast<size_t>(stub);
		}

		std::string get_timestamp()
		{
			tm ltime{};
			char timestamp[MAX_PATH] = {0};
			const auto time = _time64(nullptr);

			_localtime64_s(&ltime, &time);
			strftime(timestamp, sizeof(timestamp) - 1, "%Y-%m-%d-%H-%M-%S", &ltime);

			return timestamp;
		}

		const char* get_exception_string(DWORD exception)
		{
#define EXCEPTION_CASE(CODE)	case EXCEPTION_##CODE : return "EXCEPTION_" #CODE
			switch (exception)
			{
				EXCEPTION_CASE(ACCESS_VIOLATION);
				EXCEPTION_CASE(DATATYPE_MISALIGNMENT);
				EXCEPTION_CASE(BREAKPOINT);
				EXCEPTION_CASE(SINGLE_STEP);
				EXCEPTION_CASE(ARRAY_BOUNDS_EXCEEDED);
				EXCEPTION_CASE(FLT_DENORMAL_OPERAND);
				EXCEPTION_CASE(FLT_DIVIDE_BY_ZERO);
				EXCEPTION_CASE(FLT_INEXACT_RESULT);
				EXCEPTION_CASE(FLT_INVALID_OPERATION);
				EXCEPTION_CASE(FLT_OVERFLOW);
				EXCEPTION_CASE(FLT_STACK_CHECK);
				EXCEPTION_CASE(FLT_UNDERFLOW);
				EXCEPTION_CASE(INT_DIVIDE_BY_ZERO);
				EXCEPTION_CASE(INT_OVERFLOW);
				EXCEPTION_CASE(PRIV_INSTRUCTION);
				EXCEPTION_CASE(IN_PAGE_ERROR);
				EXCEPTION_CASE(ILLEGAL_INSTRUCTION);
				EXCEPTION_CASE(NONCONTINUABLE_EXCEPTION);
				EXCEPTION_CASE(STACK_OVERFLOW);
				EXCEPTION_CASE(INVALID_DISPOSITION);
				EXCEPTION_CASE(GUARD_PAGE);
				EXCEPTION_CASE(INVALID_HANDLE);
			default:
				return "UNKNOWN";
			}
#undef EXCEPTION_CASE
		}

		std::string get_memory_registers(const LPEXCEPTION_POINTERS exception_info)
		{
			if (IsBadReadPtr(exception_info, sizeof(EXCEPTION_POINTERS)))
				return "";

			const auto* ctx = exception_info->ContextRecord;

			std::string registers_scroll{};
			registers_scroll.append("registers:\r\n{\r\n");

			const auto x64register = [&registers_scroll](const char* key, DWORD64 value)
			{
				registers_scroll.append(utils::string::va("\t%s = 0x%llX\r\n", key, value));
			};

			x64register("rax", ctx->Rax);
			x64register("rbx", ctx->Rbx);
			x64register("rcx", ctx->Rcx);
			x64register("rdx", ctx->Rdx);
			x64register("rsp", ctx->Rsp);
			x64register("rbp", ctx->Rbp);
			x64register("rsi", ctx->Rsi);
			x64register("rdi", ctx->Rdi);
			x64register("r8", ctx->R8);
			x64register("r9", ctx->R9);
			x64register("r10", ctx->R10);
			x64register("r11", ctx->R11);
			x64register("r12", ctx->R12);
			x64register("r13", ctx->R13);
			x64register("r14", ctx->R14);
			x64register("r15", ctx->R15);
			x64register("rip", ctx->Rip);

			registers_scroll.append("}");

			return registers_scroll;
		}

		std::string get_callstack_summary(void* exception_addr, int trace_depth = 32)
		{
			std::string callstack_scroll("callstack:\r\n{\r\n");

			void* backtrace_stack[32]; int backtrace_stack_size = ARRAYSIZE(backtrace_stack);
			if (trace_depth > backtrace_stack_size) trace_depth = backtrace_stack_size;

			size_t count = RtlCaptureStackBackTrace(0, trace_depth, backtrace_stack, NULL);

			auto itr = std::find(backtrace_stack, backtrace_stack + backtrace_stack_size, exception_addr);
			auto exception_start_index = std::distance(backtrace_stack, itr);

			for (size_t i = exception_start_index; i < count; i++)
			{
				const auto from = utils::nt::library::get_by_address(backtrace_stack[i]);
				size_t rva = reinterpret_cast<uint64_t>(backtrace_stack[i]) - reinterpret_cast<uint64_t>(from.get_ptr());

				if (from.get_name() == "BlackOps4.exe"s) rva += 0x140000000;

				callstack_scroll.append(std::format("\t{}: {:012X}\r\n", from.get_name(), rva));
			}

			return callstack_scroll.append("}");
		}

		std::string generate_crash_info(const LPEXCEPTION_POINTERS exceptioninfo)
		{
			const auto& build_info = game::version_string;
			const auto main_module = utils::nt::library{};
			const auto rip_address = exceptioninfo->ExceptionRecord->ExceptionAddress;

			std::string info{};
			const auto line = [&info](const std::string& text)
			{
				info.append(text);
				info.append("\r\n");
			};

			line(build_info + " Crash Report\r\n");

			line(utils::string::va("Exception Code: 0x%08X(%s)", exceptioninfo->ExceptionRecord->ExceptionCode, 
				get_exception_string(exceptioninfo->ExceptionRecord->ExceptionCode)));
			line(utils::string::va("Exception Addr: 0x%llX[%s]", exceptioninfo->ExceptionRecord->ExceptionAddress, 
				utils::nt::library::get_by_address(exceptioninfo->ExceptionRecord->ExceptionAddress).get_name().c_str()));
			line(utils::string::va("Main Module: %s[0x%llX]", main_module.get_name().c_str(), main_module.get_ptr()));
			line(utils::string::va("Thread ID: %d(%s)", GetCurrentThreadId(), is_game_thread() ? "Main Thread" : "Auxiliary Threads"));

			if (exceptioninfo->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION)
			{
				line(utils::string::va("\r\nExtended Info: Attempted to %s 0x%012X",
					exceptioninfo->ExceptionRecord->ExceptionInformation[0] == 1 ? "write to" : "read from",
					exceptioninfo->ExceptionRecord->ExceptionInformation[1]));
			}

			line("\r\n");
			line(get_callstack_summary(rip_address));
			line(get_memory_registers(exceptioninfo));

			line("\r\nTimestamp: "s + get_timestamp());

			return info;
		}

		void write_minidump(const LPEXCEPTION_POINTERS exceptioninfo)
		{
			const std::string crash_name = utils::string::va("minidumps/shield-crash-%s.zip",
			                                                 get_timestamp().data());

			utils::compression::zip::archive zip_file{};
			zip_file.add("crash.dmp", exception::create_minidump(exceptioninfo));
			zip_file.add("info.txt", generate_crash_info(exceptioninfo));
			zip_file.write(crash_name, "Project-bo4 Crash Dump");
		}

		bool is_harmless_error(const LPEXCEPTION_POINTERS exceptioninfo)
		{
			const auto code = exceptioninfo->ExceptionRecord->ExceptionCode;
			return code == STATUS_INTEGER_OVERFLOW || code == STATUS_FLOAT_OVERFLOW || code == STATUS_SINGLE_STEP;
		}

		LONG WINAPI exception_filter(const LPEXCEPTION_POINTERS exceptioninfo)
		{
			if (is_harmless_error(exceptioninfo))
			{
				return EXCEPTION_CONTINUE_EXECUTION;
			}

			write_minidump(exceptioninfo);

			exception_data.code = exceptioninfo->ExceptionRecord->ExceptionCode;
			exception_data.address = exceptioninfo->ExceptionRecord->ExceptionAddress;
			exceptioninfo->ContextRecord->Rip = get_reset_state_stub();

			return EXCEPTION_CONTINUE_EXECUTION;
		}

		void WINAPI set_unhandled_exception_filter_stub(LPTOP_LEVEL_EXCEPTION_FILTER)
		{
			// Don't register anything here...
		}
	}

	class component final : public component_interface
	{
	public:
		component()
		{
			main_thread_id = GetCurrentThreadId();
			SetUnhandledExceptionFilter(exception_filter);
		}

		void pre_start() override
		{
			const utils::nt::library ntdll("ntdll.dll");
			auto* set_filter = ntdll.get_proc<void(*)(LPTOP_LEVEL_EXCEPTION_FILTER)>("RtlSetUnhandledExceptionFilter");

			set_filter(exception_filter);
			utils::hook::jump(set_filter, set_unhandled_exception_filter_stub);
		}
	};
}

REGISTER_COMPONENT(blackbox::component)