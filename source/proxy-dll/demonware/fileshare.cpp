#include <std_include.hpp>
#include "fileshare.hpp"
#include "component/platform.hpp"

#include <utilities/io.hpp>
#include <utilities/string.hpp>
#include <utilities/cryptography.hpp>

namespace demonware::fileshare
{
	const char* get_fileshare_host_name() 
	{
		return "ops4-fileshare.prod.schild.net";
	}

	const char* get_category_extension(fileshareCategory_e cat)
	{
		switch (cat)
		{
		case fileshareCategory_e::FILESHARE_CATEGORY_FILM:
			return "demo";
		case fileshareCategory_e::FILESHARE_CATEGORY_SCREENSHOT_PRIVATE:
			return "jpg";
		default:
			return "";
		}
	}

	fileshareCategory_e get_extension_category(const char* ext)
	{
		if (!strcmp(ext, "demo")) {
			return fileshareCategory_e::FILESHARE_CATEGORY_FILM;
		}
		else if (!strcmp(ext, "jpg")) {
			return fileshareCategory_e::FILESHARE_CATEGORY_SCREENSHOT_PRIVATE;
		}
		else {
			return fileshareCategory_e::FILESHARE_CATEGORY_ALL;
		}
	}

	std::string get_fileshare_directory()
	{
		return std::format("players/fileshare-{}", platform::bnet_get_userid());
	}

	std::string get_file_name(const uint64_t fileID, fileshareCategory_e category)
	{
		const char* extension = get_category_extension(category);

		if(!extension || !strlen(extension)) return std::to_string(fileID);

		return std::format("{}.{}", fileID, extension);
	}

	std::string get_file_url(const std::string& file)
	{
		return std::format("http://{}/{}", get_fileshare_host_name(), utilities::io::file_name(file));
	}

	std::string get_file_path(const std::string& file)
	{
		return std::format("{}/{}", get_fileshare_directory(), utilities::io::file_name(file));
	}

	std::string get_metadata_path(const std::string& file)
	{
		return std::format("{}/{}.metadata", get_fileshare_directory(), utilities::io::file_stem(file));
	}

	std::string FileMetadata::SerializeMetaJSON()
	{
		rapidjson::Document jsonDocument(rapidjson::kObjectType);
		rapidjson::Document::AllocatorType& allocator = jsonDocument.GetAllocator();
		
		jsonDocument.AddMember("status", this->state, allocator);
		jsonDocument.AddMember("category", this->category, allocator);
		jsonDocument.AddMember("fileName", this->ioFileName, allocator);
		if (this->state == FILE_STATE_UPLOADED || this->state == FILE_STATE_DESCRIBED) {
			jsonDocument.AddMember("fileSize", this->ioFileSize, allocator);
		}

		rapidjson::Value fileInfo(rapidjson::kObjectType);
		fileInfo.AddMember("id", this->file.id, allocator);
		fileInfo.AddMember("name", this->file.name, allocator);
		if (this->state == FILE_STATE_UPLOADED || this->state == FILE_STATE_DESCRIBED) {
			fileInfo.AddMember("size", this->file.size, allocator);
		}
		fileInfo.AddMember("timestamp", this->file.timestamp, allocator);
		jsonDocument.AddMember("file", fileInfo, allocator);

		rapidjson::Value fileAuthor(rapidjson::kObjectType);
		fileAuthor.AddMember("name", this->author.name, allocator);
		fileAuthor.AddMember("xuid", this->author.xuid, allocator);
		jsonDocument.AddMember("author", fileAuthor, allocator);

		if (this->state == FILE_STATE_DESCRIBED) {
			rapidjson::Value fileTags(rapidjson::kObjectType);
			for (const auto& tag : this->tags)
			{
				rapidjson::Value key(std::to_string(tag.first), allocator);
				fileTags.AddMember(key, tag.second, allocator);
			}
			jsonDocument.AddMember("tags", fileTags, allocator);

			jsonDocument.AddMember("metadata", utilities::cryptography::base64::encode(ddlMetadata), allocator);
		}
	
		rapidjson::StringBuffer strbuf;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(strbuf);
		jsonDocument.Accept(writer);

		return std::string(strbuf.GetString(), strbuf.GetLength());
	}

	bool FileMetadata::ParseMetaJSON(const std::string& MetaDoc)
	{
		rapidjson::Document jsonDocument;
		jsonDocument.Parse(MetaDoc);

		if (jsonDocument.HasMember("status") && jsonDocument["status"].IsInt()) {
			this->state = static_cast<file_state>(jsonDocument["status"].GetInt());
		}

		if (jsonDocument.HasMember("category") && jsonDocument["category"].IsInt()) {
			this->category = static_cast<fileshareCategory_e>(jsonDocument["category"].GetInt());
		}

		if (jsonDocument.HasMember("fileName") && jsonDocument["fileName"].IsString()) {
			this->ioFileName = jsonDocument["fileName"].GetString();
		}

		if ((this->state == FILE_STATE_UPLOADED || this->state == FILE_STATE_DESCRIBED)
			&& jsonDocument.HasMember("fileSize") && jsonDocument["fileSize"].IsUint()) 
		{
			this->ioFileSize = jsonDocument["fileSize"].GetUint();
		}

		if (jsonDocument.HasMember("file") && jsonDocument["file"].IsObject())
		{
			rapidjson::Value& fileInfo = jsonDocument["file"];
			if (fileInfo.HasMember("id") && fileInfo["id"].IsUint64()
				&& fileInfo.HasMember("name") && fileInfo["name"].IsString()
				&& fileInfo.HasMember("timestamp") && fileInfo["timestamp"].IsUint())
			{
				this->file.id = fileInfo["id"].GetUint64();
				this->file.name = fileInfo["name"].GetString();
				this->file.timestamp = fileInfo["timestamp"].GetUint();
			}
			if ((this->state == FILE_STATE_UPLOADED || this->state == FILE_STATE_DESCRIBED)
				&& fileInfo.HasMember("size") && fileInfo["size"].IsUint())
			{
				this->file.size = fileInfo["size"].GetUint();
			}
		}

		if (jsonDocument.HasMember("author") && jsonDocument["author"].IsObject())
		{
			rapidjson::Value& fileAuthor = jsonDocument["author"];
			if (fileAuthor.HasMember("name") && fileAuthor["name"].IsString()
				&& fileAuthor.HasMember("xuid") && fileAuthor["xuid"].IsUint64())
			{
				this->author.name = fileAuthor["name"].GetString();
				this->author.xuid = fileAuthor["xuid"].GetUint64();
			}
		}

		if (this->state == FILE_STATE_DESCRIBED) {
			if (jsonDocument.HasMember("tags") && jsonDocument["tags"].IsObject())
			{
				rapidjson::Value& fileTags = jsonDocument["tags"];

				for (rapidjson::Value::ConstMemberIterator itr =
					fileTags.MemberBegin(); itr != fileTags.MemberEnd(); ++itr)
				{
					this->tags.insert({ atoi(itr->name.GetString()), itr->value.GetUint64() });
				}
			}

			if (jsonDocument.HasMember("metadata") && jsonDocument["metadata"].IsString())
			{
				rapidjson::Value& ddl_field = jsonDocument["metadata"];
				std::string metadata_b64(ddl_field.GetString(), ddl_field.GetStringLength());
				this->ddlMetadata = utilities::cryptography::base64::decode(metadata_b64);
			}
		}

		if (this->state == FILE_STATE_DESCRIBED) {
			return (this->ioFileName.size() && this->file.id && this->ddlMetadata.size());
		}
		else if (this->state == FILE_STATE_UPLOADED) {
			return (this->ioFileName.size() && this->file.id && this->ioFileSize);
		}
		else {
			return (this->ioFileName.size() && this->file.id && this->file.name.size());
		}
	}

	bool FileMetadata::MetadataTaskResult(bdFileMetaData* output, bool download)
	{
		if (this->state == FILE_STATE_DESCRIBED)
		{
			output->m_fileID = this->file.id;
			output->m_createTime = this->file.timestamp;
			output->m_modifedTime = output->m_createTime;
			output->m_fileSize = this->ioFileSize;
			output->m_ownerID = this->author.xuid;
			output->m_ownerName = this->author.name;
			output->m_fileSlot = 0;
			output->m_fileName = this->file.name;
			output->m_category = this->category;
			output->m_metaData = this->ddlMetadata;
			output->m_summaryFileSize = 0; // what!?

			if (download) {
				output->m_url = get_file_url(this->ioFileName);
			}
			else {
				output->m_tags = this->tags;
			}

			return true;
		}
		else {
			return false;
		}
	}

	bool FileMetadata::ReadMetaDataJson(const std::string& path, file_state expect)
	{
		std::string json;
		if (utilities::io::read_file(path, &json))
		{
			return (this->ParseMetaJSON(json)
				&& (this->state == expect || expect == FILE_STATE_UNKNOWN));
		}
		else {
			return false;
		}
	}

	bool FileMetadata::WriteMetaDataJson(const std::string& path, file_state _state)
	{
		if(_state != FILE_STATE_UNKNOWN) this->state = _state;
		return utilities::io::write_file(path, this->SerializeMetaJSON());
	}

	std::vector<uint64_t> fileshare_list_demo_ids()
	{
		std::vector<uint64_t> results;

		std::vector<std::string> files = utilities::io::list_files(get_fileshare_directory());

		for (auto& file : files)
		{
			std::string rawName = utilities::io::file_stem(file);

			if (utilities::string::is_integer(rawName)
				&& utilities::string::ends_with(file, ".demo")
				&& utilities::io::file_exists(get_metadata_path(rawName)))
			{
				results.push_back(std::stoull(rawName));
			}
		}

		return results;
	}
}
