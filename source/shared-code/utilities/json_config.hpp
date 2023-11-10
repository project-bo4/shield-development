#pragma once
#include <string>

namespace utilities::json_config
{
	bool ReadBoolean(const char* szSection, const char* szKey, bool bolDefaultValue);
	void WriteBoolean(const char* szSection, const char* szKey, bool bolValue);

	std::string ReadString(const char* szSection, const char* szKey, const std::string& strDefaultValue);
	void WriteString(const char* szSection, const char* szKey, const std::string& strValue);

	int32_t ReadInteger(const char* szSection, const char* szKey, int32_t iDefaultValue);
	void WriteInteger(const char* szSection, const char* szKey, int32_t iValue);
	uint32_t ReadUnsignedInteger(const char* szSection, const char* szKey, uint32_t iDefaultValue);
	void WriteUnsignedInteger(const char* szSection, const char* szKey, uint32_t iValue);

	int64_t ReadInteger64(const char* szSection, const char* szKey, int64_t iDefaultValue);
	void WriteInteger64(const char* szSection, const char* szKey, int64_t iValue);
	uint64_t ReadUnsignedInteger64(const char* szSection, const char* szKey, uint64_t iDefaultValue);
	void WriteUnsignedInteger64(const char* szSection, const char* szKey, uint64_t iValue);
}
