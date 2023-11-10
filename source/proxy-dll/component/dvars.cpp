#include <std_include.hpp>
#include "dvars.hpp"
#include "scheduler.hpp"
#include "spoofer/spoofcall.hpp"
#include "definitions/variables.hpp"
#include "loader/component_loader.hpp"

#include <utilities/string.hpp>

namespace dvars
{
	namespace
	{
		void fetch_dvar_pointers()
		{
			for (auto& dvar : variables::dvars_record)
			{
				dvar.pointer = spoofcall::invoke<uintptr_t>(game::Dvar_FindVar, dvar.name.data());
			}
		}

		std::string get_vector_string(const int components, const game::DvarLimits& domain)
		{
			if (domain.vector.min == -FLT_MAX)
			{
				if (domain.vector.max == FLT_MAX)
				{
					return utilities::string::va("Domain is any %iD vector", components);
				}
				else
				{
					return utilities::string::va("Domain is any %iD vector with components %g or smaller", components,
						domain.vector.max);
				}
			}
			else if (domain.vector.max == FLT_MAX)
			{
				return utilities::string::va("Domain is any %iD vector with components %g or bigger", components,
					domain.vector.min);
			}
			else
			{
				return utilities::string::va("Domain is any %iD vector with components from %g to %g", components,
					domain.vector.min, domain.vector.max);
			}
		}
	}

	std::string get_domain_string(const game::dvarType_t type, const game::DvarLimits& domain)
	{
		std::string str;

		switch (type)
		{
		case game::DVAR_TYPE_BOOL:
			return "Domain is 0 or 1"s;

		case game::DVAR_TYPE_FLOAT:
			if (domain.value.min == -FLT_MAX)
			{
				if (domain.value.max == FLT_MAX)
				{
					return "Domain is any number"s;
				}
				else
				{
					return utilities::string::va("Domain is any number %g or smaller", domain.value.max);
				}
			}
			else if (domain.value.max == FLT_MAX)
			{
				return utilities::string::va("Domain is any number %g or bigger", domain.value.min);
			}
			else
			{
				return utilities::string::va("Domain is any number from %g to %g", domain.value.min, domain.value.max);
			}

		case game::DVAR_TYPE_FLOAT_2:
			return get_vector_string(2, domain);

		case game::DVAR_TYPE_FLOAT_3:
		case game::DVAR_TYPE_LINEAR_COLOR_RGB:
		case game::DVAR_TYPE_COLOR_XYZ:
		case game::DVAR_TYPE_COLOR_LAB:
			return get_vector_string(3, domain);

		case game::DVAR_TYPE_FLOAT_4:
			return get_vector_string(4, domain);

		case game::DVAR_TYPE_INT:
			if (domain.integer.min == INT_MIN)
			{
				if (domain.integer.max == INT_MAX)
				{
					return "Domain is any integer"s;
				}
				else
				{
					return utilities::string::va("Domain is any integer %i or smaller", domain.integer.max);
				}
			}
			else if (domain.integer.max == INT_MAX)
			{
				return utilities::string::va("Domain is any integer %i or bigger", domain.integer.min);
			}
			else
			{
				return utilities::string::va("Domain is any integer from %i to %i", domain.integer.min, domain.integer.max);
			}

		case game::DVAR_TYPE_ENUM:
			str = "Domain is one of the following:"s;

			for (auto string_index = 0; string_index < domain.enumeration.stringCount; ++string_index)
			{
				str += utilities::string::va("\n  %2i: %s", string_index, domain.enumeration.strings[string_index]);
			}

			return str;

		case game::DVAR_TYPE_STRING:
			return "Domain is any text"s;

		case game::DVAR_TYPE_COLOR:
			return "Domain is any 4-component color, in RGBA format"s;

		case game::DVAR_TYPE_INT64:
			if (domain.integer64.min == _I64_MIN)
			{
				if (domain.integer64.max == _I64_MAX)
				{
					return "Domain is any integer"s;
				}
				else
				{
					return utilities::string::va("Domain is any integer %lli or smaller", domain.integer64.max);
				}
			}
			else if (domain.integer64.max == _I64_MAX)
			{
				return utilities::string::va("Domain is any integer %lli or bigger", domain.integer64.min);
			}
			else
			{
				return utilities::string::va("Domain is any integer from %lli to %lli", domain.integer64.min, domain.integer64.max);
			}

		case game::DVAR_TYPE_UINT64:
			if (domain.unsignedInt64.min)
			{
				if (domain.unsignedInt64.max == _UI64_MAX)
				{
					return utilities::string::va("Domain is any unsigned integer %zu or bigger", domain.unsignedInt64.min);

				}
				else
				{
					return utilities::string::va("Domain is any unsigned integer from %zu to %zu", domain.unsignedInt64.min, domain.unsignedInt64.max);
				}
			}
			else if (domain.unsignedInt64.max == _UI64_MAX)
			{
				return "Domain is any integer"s;
			}
			else
			{
				return utilities::string::va("Domain is any integer %zu or smaller", domain.unsignedInt64.max);
			}

		default:
			return utilities::string::va("unhandled dvar type '%i'", type);
		}
	}

	std::string get_value_string(const game::dvar_t * dvar, game::DvarValue * value)
	{
		std::string result = "N/A";

		switch (dvar->type)
		{
		case game::DVAR_TYPE_BOOL:
			if (value->naked.enabled) 
			{
				result = "1"s;
			}
			else 
			{
				result = "0"s;
			}
			break;

		case game::DVAR_TYPE_FLOAT:
			result = std::format("{:.2f}", value->naked.value);
			break;

		case game::DVAR_TYPE_FLOAT_2:
			result = std::format("{:.2f} {:.2f}", value->naked.vector[0], value->naked.vector[1]);
			break;

		case game::DVAR_TYPE_FLOAT_3:
		case game::DVAR_TYPE_LINEAR_COLOR_RGB:
		case game::DVAR_TYPE_COLOR_XYZ:
		case game::DVAR_TYPE_COLOR_LAB:
			result = std::format("{:.2f} {:.2f} {:.2f}", value->naked.vector[0], value->naked.vector[1], value->naked.vector[2]);
			break;

		case game::DVAR_TYPE_FLOAT_4:
			result = std::format("{:.2f} {:.2f} {:.2f} {:.2f}", value->naked.vector[0], value->naked.vector[1], value->naked.vector[2], value->naked.vector[3]);
			break;

		case game::DVAR_TYPE_INT:
			result = std::format("{}", value->naked.integer);
			break;

		case game::DVAR_TYPE_ENUM:
			if (dvar->domain.enumeration.stringCount)
			{
				result = std::string(dvar->domain.enumeration.strings[value->naked.integer]);
			}
			break;

		case game::DVAR_TYPE_STRING:
			result = std::string(value->naked.string);
			break;

		case game::DVAR_TYPE_COLOR:
			result = std::format("{:.2f} {:.2f} {:.2f} {:.2f}", (float)value->naked.color[0] * 0.0039215689, (float)value->naked.color[1] * 0.0039215689, (float)value->naked.color[2] * 0.0039215689, (float)value->naked.color[3] * 0.0039215689);
			break;

		case game::DVAR_TYPE_INT64:
			result = std::format("{}", value->naked.integer64);
			break;

		case game::DVAR_TYPE_UINT64:
			result = std::format("{}", value->naked.unsignedInt64);
			break;

		default:
			result = "ERROR:DVAR_TYPE_UNKNOWN"s;
			break;
		}

		return result;
	}

	game::dvar_t* find_dvar(uint64_t hashRef)
	{
		if (hashRef == 0) return NULL;

		auto it = std::find_if(variables::dvars_record.begin(), variables::dvars_record.end(), [&hashRef](variables::varEntry& i) { return i.fnv1a == hashRef; });

		if (it != variables::dvars_record.end() && it->pointer)
		{
			return reinterpret_cast<game::dvar_t*>(it->pointer);
		}

		return spoofcall::invoke<game::dvar_t*>(game::Dvar_FindVar_Hash, game::AssetRef(hashRef));
	}

	game::dvar_t* find_dvar(const char* nameRef)
	{
		return spoofcall::invoke<game::dvar_t*>(game::Dvar_FindVar, nameRef);
	}

	game::dvar_t* find_dvar(const std::string& nameRef)
	{
		auto it = std::find_if(variables::dvars_record.begin(), variables::dvars_record.end(), [&nameRef](variables::varEntry& i) {  return utilities::string::compare(i.name, nameRef); });

		if (it != variables::dvars_record.end() && it->pointer)
		{
			return reinterpret_cast<game::dvar_t*>(it->pointer);
		}

		return spoofcall::invoke<game::dvar_t*>(game::Dvar_FindVar, nameRef.data());
	}


	class component final : public component_interface
	{
	public:
		void post_unpack() override
		{
			scheduler::once(fetch_dvar_pointers, scheduler::pipeline::main);
		}
	};
}

REGISTER_COMPONENT(dvars::component)
