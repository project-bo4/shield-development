#include <std_include.hpp>
#include "loader/component_loader.hpp"

#include "command.hpp"
#include <utilities/hook.hpp>
#include <utilities/string.hpp>
#include <utilities/memory.hpp>

#include <definitions/game.hpp>

namespace command
{
	namespace
	{
		std::unordered_map<std::string, command_param_function>& get_command_map()
		{
			static std::unordered_map<std::string, command_param_function> command_map{};
			return command_map;
		}

		std::unordered_map<std::string, sv_command_param_function>& get_sv_command_map()
		{
			static std::unordered_map<std::string, sv_command_param_function> command_map{};
			return command_map;
		}

		void execute_custom_command()
		{
			const params params{};
			const auto command = utilities::string::to_lower(params[0]);

			auto& map = get_command_map();
			const auto entry = map.find(command);
			if (entry != map.end())
			{
				entry->second(params);
			}
		}

		void execute_custom_sv_command()
		{
			const params_sv params{};
			const auto command = utilities::string::to_lower(params[0]);

			auto& map = get_sv_command_map();
			const auto entry = map.find(command);
			if (entry != map.end())
			{
				entry->second(params);
			}
		}

		game::CmdArgs* get_cmd_args()
		{
			return game::Sys_GetTLS()->cmdArgs;
		}
	}

	params::params()
		: nesting_(get_cmd_args()->nesting)
	{
		assert(this->nesting_ < game::CMD_MAX_NESTING);
	}

	params::params(const std::string& text)
		: needs_end_(true)
	{
		auto* cmd_args = get_cmd_args();
		game::Cmd_TokenizeStringKernel(0, game::CONTROLLER_INDEX_FIRST, text.data(), 512 - cmd_args->totalUsedArgvPool, false, cmd_args);

		this->nesting_ = cmd_args->nesting;
	}

	params::~params()
	{
		if (this->needs_end_)
		{
			game::Cmd_EndTokenizedString();
		}
	}

	int params::size() const
	{
		return get_cmd_args()->argc[this->nesting_];
	}

	params_sv::params_sv()
		: nesting_(game::sv_cmd_args->nesting)
	{
		assert(this->nesting_ < game::CMD_MAX_NESTING);
	}

	params_sv::params_sv(const std::string& text)
		: needs_end_(true)
	{
		game::Cmd_TokenizeStringKernel(-1, game::CONTROLLER_INDEX_FIRST, text.data(), 512 - game::sv_cmd_args->totalUsedArgvPool, true, game::sv_cmd_args);
		this->nesting_ = game::sv_cmd_args->nesting;
	}

	params_sv::~params_sv()
	{
		if (this->needs_end_)
		{
			game::Cmd_EndTokenizedStringKernel();
		}
	}

	int params_sv::size() const
	{
		return game::sv_cmd_args->argc[this->nesting_];
	}

	const char* params_sv::get(const int index) const
	{
		if (index >= this->size())
		{
			return "";
		}

		return game::sv_cmd_args->argv[this->nesting_][index];
	}

	std::string params_sv::join(const int index) const
	{
		std::string result;

		for (auto i = index; i < this->size(); ++i)
		{
			if (i > index) result.append(" ");
			result.append(this->get(i));
		}

		return result;
	}

	const char* params::get(const int index) const
	{
		if (index >= this->size())
		{
			return "";
		}

		return get_cmd_args()->argv[this->nesting_][index];
	}

	std::string params::join(const int index) const
	{
		std::string result = {};

		for (auto i = index; i < this->size(); i++)
		{
			if (i > index) result.append(" ");
			result.append(this->get(i));
		}
		return result;
	}

	void add(const std::string& command, command_function function)
	{
		add(command, [f = std::move(function)](const params&)
		{
			f();
		});
	}

	void add(const std::string& command, command_param_function function)
	{
		auto lower_command = utilities::string::to_lower(command);

		auto& map = get_command_map();
		const auto is_registered = map.contains(lower_command);

		map[std::move(lower_command)] = std::move(function);

		if (is_registered) return;

		auto& allocator = *utilities::memory::get_allocator();
		auto* cmd_function = allocator.allocate<game::cmd_function_t>();
		const auto* cmd_string = allocator.duplicate_string(command);

		game::BO4_AssetRef_t cmd_string_converted{ static_cast<std::int64_t>(fnv1a::generate_hash(cmd_string)), 0 };
		game::Cmd_AddCommandInternal(&cmd_string_converted, execute_custom_command, cmd_function);
	}
}
