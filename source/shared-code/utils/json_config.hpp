#pragma once
#include <string>

namespace utils::json_config
{
	std::string ReadString(const char* szSection, const char* szKey, const std::string& strDefaultValue);
	void WriteString(const char* szSection, const char* szKey, const std::string& strValue);

	unsigned int ReadUnsignedInteger(const char* szSection, const char* szKey, unsigned int iDefaultValue);
	void WriteUnsignedInteger(const char* szSection, const char* szKey, unsigned int iValue);

	int ReadInteger(const char* szSection, const char* szKey, int iDefaultValue);
	void WriteInteger(const char* szSection, const char* szKey, int iValue);

	bool ReadBoolean(const char* szSection, const char* szKey, bool bolDefaultValue);
	void WriteBoolean(const char* szSection, const char* szKey, bool bolValue);
}
