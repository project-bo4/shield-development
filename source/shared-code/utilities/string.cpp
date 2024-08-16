#include "string.hpp"
#include <regex>
#include <cstdarg>
#include <algorithm>

#include "nt.hpp"

namespace utilities::string
{
	const char* va(const char* fmt, ...)
	{
		static thread_local va_provider<8, 256> provider;

		va_list ap;
		va_start(ap, fmt);

		const char* result = provider.get(fmt, ap);

		va_end(ap);
		return result;
	}

	std::vector<std::string> split(const std::string& s, const char delim)
	{
		std::stringstream ss(s);
		std::string item;
		std::vector<std::string> elems;

		while (std::getline(ss, item, delim))
		{
			elems.push_back(item); // elems.push_back(std::move(item)); // if C++11 (based on comment from @mchiasson)
		}

		return elems;
	}

	std::vector<std::string> split(const std::string& s, const std::string& delim)
	{
		size_t pos_start = 0, pos_end, delim_len = delim.length();
		std::string token;
		std::vector<std::string> elems;

		while ((pos_end = s.find(delim, pos_start)) != std::string::npos) {
			token = s.substr(pos_start, pos_end - pos_start);
			pos_start = pos_end + delim_len;
			elems.push_back(token);
		}

		elems.push_back(s.substr(pos_start));

		return elems;
	}
	
	std::string to_lower(std::string text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](const char input)
		{
			return static_cast<char>(tolower(input));
		});

		return text;
	}

	std::string to_upper(std::string text)
	{
		std::transform(text.begin(), text.end(), text.begin(), [](const char input)
		{
			return static_cast<char>(toupper(input));
		});

		return text;
	}

	bool is_integer(const std::string& str) {
		return std::regex_match(str, std::regex("[(-|+)|][0-9]+"));
	}
	
	bool starts_with(const std::string& text, const std::string& substring)
	{
		return text.find(substring) == 0;
	}

	bool ends_with(const std::string& text, const std::string& substring)
	{
		if (substring.size() > text.size()) return false;
		return std::equal(substring.rbegin(), substring.rend(), text.rbegin());
	}

	std::string dump_hex(const std::string& data, const std::string& separator)
	{
		std::string result;

		for (unsigned int i = 0; i < data.size(); ++i)
		{
			if (i > 0)
			{
				result.append(separator);
			}

			result.append(va("%02X", data[i] & 0xFF));
		}

		return result;
	}

	std::string get_clipboard_data()
	{
		if (OpenClipboard(nullptr))
		{
			std::string data;

			auto* const clipboard_data = GetClipboardData(1u);
			if (clipboard_data)
			{
				auto* const cliptext = static_cast<char*>(GlobalLock(clipboard_data));
				if (cliptext)
				{
					data.append(cliptext);
					GlobalUnlock(clipboard_data);
				}
			}
			CloseClipboard();

			return data;
		}
		return {};
	}

	void strip(const char* in, char* out, int max)
	{
		if (!in || !out) return;

		max--;
		auto current = 0;
		while (*in != 0 && current < max)
		{
			const auto color_index = (*(in + 1) - 48) >= 0xC ? 7 : (*(in + 1) - 48);

			if (*in == '^' && (color_index != 7 || *(in + 1) == '7'))
			{
				++in;
			}
			else
			{
				*out = *in;
				++out;
				++current;
			}

			++in;
		}
		*out = '\0';
	}

#pragma warning(push)
#pragma warning(disable: 4100)
	std::string convert(const std::wstring& wstr)
	{
		std::string result;
		result.reserve(wstr.size());

		for (const auto& chr : wstr)
		{
			result.push_back(static_cast<char>(chr));
		}

		return result;
	}

	std::wstring convert(const std::string& str)
	{
		std::wstring result;
		result.reserve(str.size());

		for (const auto& chr : str)
		{
			result.push_back(static_cast<wchar_t>(chr));
		}

		return result;
	}
#pragma warning(pop)

	void copy(char* dest, const size_t max_size, const char* src)
	{
		if (!max_size)
		{
			return;
		}

		for (size_t i = 0;; ++i)
		{
			if (i + 1 == max_size)
			{
				dest[i] = 0;
				break;
			}

			dest[i] = src[i];

			if (!src[i])
			{
				break;
			}
		}
	}

	std::string replace(std::string str, const std::string& from, const std::string& to)
	{
		if (from.empty())
		{
			return str;
		}

		size_t start_pos = 0;
		while ((start_pos = str.find(from, start_pos)) != std::string::npos)
		{
			str.replace(start_pos, from.length(), to);
			start_pos += to.length();
		}

		return str;
	}

	std::string& ltrim(std::string& str)
	{
		str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](const unsigned char input)
			{
				return !std::isspace(input);
			}));

		return str;
	}

	std::string& rtrim(std::string& str)
	{
		str.erase(std::find_if(str.rbegin(), str.rend(), [](const  unsigned char input)
			{
				return !std::isspace(input);
			}).base(), str.end());

		return str;
	}

	std::string& trim(std::string& str)
	{
		return ltrim(rtrim(str));
	}

	bool is_truely_empty(std::string str)
	{
		return trim(str).size() == 0;
	}

	StringMatch compare(const std::string& s1, const std::string& s2)
	{
		if (s1 == s2)
			return StringMatch::Identical;
		else if (to_lower(s1) == to_lower(s2))
			return StringMatch::CaseVariant;
		else
			return StringMatch::Mismatch;
	}

	float match(const std::string& input, const std::string& text)
	{
		if (text == input) return 1.00; // identical

		auto offset = to_lower(text).find(to_lower(input));
		if (offset == std::string::npos) return 0.00; // mismatch

		auto len_variance = text.length() - input.length();
		size_t match_percent = 100 - (1 + len_variance + offset);

		return (static_cast<float>(match_percent) / 100);
	}

	bool contains(std::string text, std::string substr, bool sensetive)
	{
		if (!sensetive) {
			text = to_lower(text);
			substr = to_lower(substr);
		}

		if (text.find(substr) != std::string::npos) return true;
		return false;
	}
}
