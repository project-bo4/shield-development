#include "json_config.hpp"
#include "io.hpp"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

namespace utilities::json_config
{
	rapidjson::Document json_doc{};
	std::string file_name = "project-bo4.json";

	namespace
	{
		bool read_json_config()
		{
			std::string json_data{};
			if (!io::read_file(file_name, &json_data)) return false;

			json_doc.Parse(json_data);

			if (json_doc.HasParseError()) {
				return false;
			}

			return true;
		}

		bool write_json_config()
		{
			rapidjson::StringBuffer buffer;
			rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
			json_doc.Accept(writer);

			std::string json_data(buffer.GetString(), buffer.GetLength());
			if (!io::write_file(file_name, json_data)) return false;

			return true;
		}
	}

	rapidjson::Document& get_json_document()
	{
		static bool json_initialized = false;
		if (!json_initialized)
		{
			if (!read_json_config()) {
				json_doc = rapidjson::Document(rapidjson::kObjectType);
			}

			json_initialized = true;
		}

		/*if (json_doc != NULL) */return json_doc;
	}

	rapidjson::Value& get_json_section(const char* szSection)
	{
		rapidjson::Document& doc = get_json_document();

		if (!doc.HasMember(szSection))
		{
			doc.AddMember(rapidjson::StringRef(szSection), rapidjson::kObjectType, doc.GetAllocator());
		}

		rapidjson::Value& section = doc[szSection];

		if (!section.IsObject()) section.SetObject();

		return section;
	}

	bool ReadBoolean(const char* szSection, const char* szKey, bool bolDefaultValue)
	{
		rapidjson::Document& doc = get_json_document();
		rapidjson::Value& section = get_json_section(szSection);

		if (!section.HasMember(szKey)) {
			section.AddMember(rapidjson::StringRef(szKey), bolDefaultValue, doc.GetAllocator());
		}
		else if (!section[szKey].IsBool()) {
			section[szKey].SetBool(bolDefaultValue);
		}
		else {
			return section[szKey].GetBool();
		}

		write_json_config();
		return section[szKey].GetBool();
	}

	void WriteBoolean(const char* szSection, const char* szKey, bool bolValue)
	{
		rapidjson::Document& doc = get_json_document();
		rapidjson::Value& section = get_json_section(szSection);

		if (!section.HasMember(szKey)) {
			section.AddMember(rapidjson::StringRef(szKey), bolValue, doc.GetAllocator());
		}
		else {
			section[szKey].SetBool(bolValue);
		}

		write_json_config();
	}


	std::string ReadString(const char* szSection, const char* szKey, const std::string& strDefaultValue)
	{
		rapidjson::Document& doc = get_json_document();
		rapidjson::Value& section = get_json_section(szSection);

		if (!section.HasMember(szKey)) {
			section.AddMember(rapidjson::StringRef(szKey), strDefaultValue, doc.GetAllocator());
		}
		else if (!section[szKey].IsString()) {
			section[szKey].SetString(strDefaultValue, doc.GetAllocator());
		}
		else {
			return section[szKey].GetString();
		}

		write_json_config();
		return section[szKey].GetString();
	}

	void WriteString(const char* szSection, const char* szKey, const std::string& strValue)
	{
		rapidjson::Document& doc = get_json_document();
		rapidjson::Value& section = get_json_section(szSection);

		if (!section.HasMember(szKey)) {
			section.AddMember(rapidjson::StringRef(szKey), strValue, doc.GetAllocator());
		}
		else {
			section[szKey].SetString(strValue, doc.GetAllocator());
		}

		write_json_config();
	}


	int32_t ReadInteger(const char* szSection, const char* szKey, int32_t iDefaultValue)
	{
		rapidjson::Document& doc = get_json_document();
		rapidjson::Value& section = get_json_section(szSection);

		if (!section.HasMember(szKey)) {
			section.AddMember(rapidjson::StringRef(szKey), iDefaultValue, doc.GetAllocator());
		}
		else if (!section[szKey].IsInt()) {
			section[szKey].SetInt(iDefaultValue);
		}
		else {
			return section[szKey].GetInt();
		}

		write_json_config();
		return section[szKey].GetInt();
	}

	void WriteInteger(const char* szSection, const char* szKey, int32_t iValue)
	{
		rapidjson::Document& doc = get_json_document();
		rapidjson::Value& section = get_json_section(szSection);

		if (!section.HasMember(szKey)) {
			section.AddMember(rapidjson::StringRef(szKey), iValue, doc.GetAllocator());
		}
		else {
			section[szKey].SetInt(iValue);
		}

		write_json_config();
	}

	uint32_t ReadUnsignedInteger(const char* szSection, const char* szKey, uint32_t iDefaultValue)
	{
		rapidjson::Document& doc = get_json_document();
		rapidjson::Value& section = get_json_section(szSection);

		if (!section.HasMember(szKey)) {
			section.AddMember(rapidjson::StringRef(szKey), iDefaultValue, doc.GetAllocator());
		}
		else if (!section[szKey].IsUint()) {
			section[szKey].SetUint(iDefaultValue);
		}
		else {
			return section[szKey].GetUint();
		}

		write_json_config();
		return section[szKey].GetUint();
	}

	void WriteUnsignedInteger(const char* szSection, const char* szKey, uint32_t iValue)
	{
		rapidjson::Document& doc = get_json_document();
		rapidjson::Value& section = get_json_section(szSection);

		if (!section.HasMember(szKey)) {
			section.AddMember(rapidjson::StringRef(szKey), iValue, doc.GetAllocator());
		}
		else {
			section[szKey].SetUint(iValue);
		}

		write_json_config();
	}

	int64_t ReadInteger64(const char* szSection, const char* szKey, int64_t iDefaultValue)
	{
		rapidjson::Document& doc = get_json_document();
		rapidjson::Value& section = get_json_section(szSection);

		if (!section.HasMember(szKey)) {
			section.AddMember(rapidjson::StringRef(szKey), iDefaultValue, doc.GetAllocator());
		}
		else if (!section[szKey].IsInt64()) {
			section[szKey].SetInt64(iDefaultValue);
		}
		else {
			return section[szKey].GetInt64();
		}

		write_json_config();
		return section[szKey].GetInt64();
	}

	void WriteInteger64(const char* szSection, const char* szKey, int64_t iValue)
	{
		rapidjson::Document& doc = get_json_document();
		rapidjson::Value& section = get_json_section(szSection);

		if (!section.HasMember(szKey)) {
			section.AddMember(rapidjson::StringRef(szKey), iValue, doc.GetAllocator());
		}
		else {
			section[szKey].SetInt64(iValue);
		}

		write_json_config();
	}

	uint64_t ReadUnsignedInteger64(const char* szSection, const char* szKey, uint64_t iDefaultValue)
	{
		rapidjson::Document& doc = get_json_document();
		rapidjson::Value& section = get_json_section(szSection);

		if (!section.HasMember(szKey)) {
			section.AddMember(rapidjson::StringRef(szKey), iDefaultValue, doc.GetAllocator());
		}
		else if (!section[szKey].IsUint64()) {
			section[szKey].SetUint64(iDefaultValue);
		}
		else {
			return section[szKey].GetUint64();
		}

		write_json_config();
		return section[szKey].GetUint64();
	}

	void WriteUnsignedInteger64(const char* szSection, const char* szKey, uint64_t iValue)
	{
		rapidjson::Document& doc = get_json_document();
		rapidjson::Value& section = get_json_section(szSection);

		if (!section.HasMember(szKey)) {
			section.AddMember(rapidjson::StringRef(szKey), iValue, doc.GetAllocator());
		}
		else {
			section[szKey].SetUint64(iValue);
		}

		write_json_config();
	}
}