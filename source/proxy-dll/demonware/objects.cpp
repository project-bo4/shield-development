#include <std_include.hpp>
#include "objects.hpp"

#include "protobuf.hpp"

#include "resource.hpp"
#include <utilities/nt.hpp>
#include <utilities/io.hpp>
#include <utilities/string.hpp>
#include <utilities/cryptography.hpp>
#include <component/platform.hpp>

#define PUBLISHER_OBJECTS_ENUMERATE_LPC_DIR

namespace demonware
{
	std::string HexStringToBinaryString(const std::string& hex_str)
	{
		std::string data{};

		for (unsigned int i = 0; i < hex_str.length(); i += 2) {
			std::string byteString = hex_str.substr(i, 2);
			char byte = (char)strtol(byteString.c_str(), NULL, 16);
			data.push_back(byte);
		}

		return data;
	}

	std::string get_publisher_file_checksum(std::string file)
	{
		std::string file_data;
		if (!utilities::io::read_file(file, &file_data)) return "";

		std::string checksum_md5 = utilities::cryptography::md5::compute(file_data);

		return utilities::cryptography::base64::encode(checksum_md5);
	}

	std::vector<objectMetadata> get_publisher_objects_list(const std::string& category)
	{
		std::vector<objectMetadata> result;

#ifdef PUBLISHER_OBJECTS_ENUMERATE_LPC_DIR
		std::vector<std::string> files = utilities::io::list_files("LPC");

		for (std::string file : files)
		{
			if (!utilities::string::ends_with(file, ".ff")) continue;

			int64_t timestamp = static_cast<int64_t>(time(nullptr));
			result.push_back({ "treyarch", utilities::io::file_name(file), get_publisher_file_checksum(file), utilities::io::file_size(file), timestamp, timestamp, "" });
		}
#else // PUBLISHER_OBJECTS_ENUMERATE_CSV_LIST
		const auto objects_list_csv = utilities::nt::load_resource(DW_PUBLISHER_OBJECTS_LIST);
		std::vector<std::string> items = utilities::string::split(objects_list_csv, "\r\n"); // WTF!?

		for (std::string item : items)
		{
			std::string checksum = utilities::cryptography::base64::encode(HexStringToBinaryString(utilities::string::split(item, ',')[2]));
			std::string name = utilities::string::split(item, ',')[0];
			uint64_t length = std::stoull(utilities::string::split(item, ',')[1]);

			int64_t timestamp = static_cast<int64_t>(time(nullptr));
			result.push_back({ "treyarch", name, checksum, length, timestamp, timestamp, "" });
		}
#endif // PUBLISHER_OBJECTS_ENUMERATE_LPC_DIR

		return result;
	}

	std::string generate_publisher_objects_list_json(const std::string& category)
	{
		rapidjson::StringBuffer json_buffer{};
		rapidjson::PrettyWriter<rapidjson::StringBuffer> json_writer(json_buffer);

		json_writer.StartObject();

		json_writer.Key("next");
		json_writer.String("TODO");

		json_writer.Key("nextPageToken");
		json_writer.Null();

		json_writer.Key("objects");
		json_writer.StartArray();

		std::vector<objectMetadata> objects = get_publisher_objects_list(category);

		for (objectMetadata object : objects)
		{
			json_writer.StartObject();

			json_writer.Key("owner");
			json_writer.String(object.owner);

			json_writer.Key("expiresOn");
			json_writer.Uint(0);

			json_writer.Key("name");
			json_writer.String(object.name);

			json_writer.Key("checksum");
			json_writer.String(object.checksum);

			json_writer.Key("acl");
			json_writer.String("public");

			json_writer.Key("objectID");
			json_writer.Uint(0);
			json_writer.Key("contentID");
			json_writer.Null();
			json_writer.Key("objectVersion");
			json_writer.String("");
			json_writer.Key("contentVersion");
			json_writer.Null();

			json_writer.Key("contentLength");
			json_writer.Uint64(object.contentLength);

			json_writer.Key("context");
			json_writer.String("t8-bnet");

			json_writer.Key("category");
			json_writer.Null();

			json_writer.Key("created");
			json_writer.Uint64(object.created);

			json_writer.Key("modified");
			json_writer.Uint64(object.modified);

			json_writer.Key("extraData");
			json_writer.Null();
			json_writer.Key("extraDataSize");
			json_writer.Null();
			json_writer.Key("summaryContentLength");
			json_writer.Null();
			json_writer.Key("summaryChecksum");
			json_writer.Null();
			json_writer.Key("hasSummary");
			json_writer.Bool(false);

			json_writer.EndObject();
		}

		json_writer.EndArray();

		json_writer.EndObject();

		return json_buffer.GetString();
	}

	std::string get_user_file_path(const std::string& file)
	{
		return std::format("{}/{}", platform::get_userdata_directory(), file);
	}

	std::string get_user_file_checksum(std::string file_path)
	{
		std::string file_data;
		if (!utilities::io::read_file(file_path, &file_data)) return "";

		return std::to_string(utilities::cryptography::xxh32::compute(file_data));
	}

	std::string get_user_file_content(std::string file_path)
	{
		std::string file_data;
		if (!utilities::io::read_file(file_path, &file_data)) return "";

		return utilities::cryptography::base64::encode(file_data);
	}

	std::string deliver_user_objects_vectorized_json(std::vector<objectMetadata> requested_items)
	{
		rapidjson::StringBuffer json_buffer{};
		rapidjson::PrettyWriter<rapidjson::StringBuffer> json_writer(json_buffer);

		json_writer.StartObject();

		json_writer.Key("objects");
		json_writer.StartArray();

		for (size_t i = 0; i < requested_items.size(); i++)
		{
			if (requested_items[i].contentLength == 0 || requested_items[i].contentURL.empty())
			{
				continue;
			}

			//std::string file_path = get_user_file_path(requested_items[i].owner, requested_items[i].name);

			json_writer.StartObject();

			json_writer.Key("metadata");
			json_writer.StartObject();

			json_writer.Key("owner");
			json_writer.String(requested_items[i].owner.data());

			json_writer.Key("expiresOn");
			json_writer.Uint(0);

			json_writer.Key("name");
			json_writer.String(requested_items[i].name.data());

			json_writer.Key("checksum");
			json_writer.String(requested_items[i].checksum.data());

			json_writer.Key("acl");
			json_writer.String("public");

			json_writer.Key("objectID");
			json_writer.Uint(0);
			json_writer.Key("contentID");
			json_writer.Null();
			json_writer.Key("objectVersion");
			json_writer.String("");
			json_writer.Key("contentVersion");
			json_writer.Null();

			json_writer.Key("contentLength");
			json_writer.Uint64(requested_items[i].contentLength);

			json_writer.Key("context");
			json_writer.String("t8-bnet");

			json_writer.Key("category");
			json_writer.Null();

			json_writer.Key("created");
			json_writer.Uint64(requested_items[i].created);

			json_writer.Key("modified");
			json_writer.Uint64(requested_items[i].modified);

			json_writer.Key("extraData");
			json_writer.Null();
			json_writer.Key("extraDataSize");
			json_writer.Null();
			json_writer.Key("summaryContentLength");
			json_writer.Null();
			json_writer.Key("summaryChecksum");
			json_writer.Null();
			json_writer.Key("hasSummary");
			json_writer.Bool(false);

			json_writer.EndObject();

			json_writer.Key("content");
			json_writer.String(requested_items[i].contentURL.data());

			json_writer.Key("requestIndex");
			json_writer.Uint64(i);

			json_writer.EndObject();
		}

		json_writer.EndArray();

		json_writer.Key("errors");
		json_writer.StartArray();
		for (size_t i = 0; i < requested_items.size(); i++)
		{
			if (requested_items[i].contentLength == 0 || requested_items[i].contentURL.empty())
			{
				json_writer.StartObject();

				json_writer.Key("requestIndex");
				json_writer.Uint64(i);
				json_writer.Key("owner");
				json_writer.String(requested_items[i].owner.data());
				json_writer.Key("name");
				json_writer.String(requested_items[i].name.data());
				json_writer.Key("error");
				json_writer.String("Error:ClientError:NotFound");

				json_writer.EndObject();
			}
		}
		json_writer.EndArray();

		json_writer.EndObject();

		return json_buffer.GetString();
	}

	std::string deliver_user_objects_vectorized_json(std::vector<objectID> requested_items)
	{
		std::vector<objectMetadata> files_metadata_list;

		for (objectID file : requested_items)
		{
			std::string file_path = get_user_file_path(file.name);
			int64_t timestamp = static_cast<int64_t>(time(nullptr));
			files_metadata_list.push_back({ file.owner, file.name, get_user_file_checksum(file_path), utilities::io::file_size(file_path), timestamp, timestamp, get_user_file_content(file_path) });
		}

		return deliver_user_objects_vectorized_json(files_metadata_list);
	}

	std::string generate_user_objects_list_json()
	{
		rapidjson::StringBuffer json_buffer{};
		rapidjson::PrettyWriter<rapidjson::StringBuffer> json_writer(json_buffer);

		json_writer.StartObject();

		json_writer.Key("nextPageToken");
		json_writer.Null();

		json_writer.Key("next");
		json_writer.String("TODO");

		json_writer.Key("objects");
		json_writer.StartArray();

		std::string userdata_directory = platform::get_userdata_directory();

		if (utilities::io::directory_exists(userdata_directory))
		{
			std::vector<std::string> user_objects = utilities::io::list_files(userdata_directory);

			for (std::string object : user_objects)
			{
				json_writer.StartObject();

				json_writer.Key("metadata");
				json_writer.StartObject();

				json_writer.Key("owner");
				json_writer.String(std::format("bnet-{}", platform::bnet_get_userid()));

				json_writer.Key("expiresOn");
				json_writer.Uint(0);

				json_writer.Key("name");
				json_writer.String(utilities::io::file_name(object));

				json_writer.Key("checksum");
				json_writer.String(get_user_file_checksum(object));

				json_writer.Key("acl");
				json_writer.String("public");

				json_writer.Key("objectID");
				json_writer.Uint(0);
				json_writer.Key("contentID");
				json_writer.Null();
				json_writer.Key("objectVersion");
				json_writer.String("");
				json_writer.Key("contentVersion");
				json_writer.Null();

				json_writer.Key("contentLength");
				json_writer.Uint64(utilities::io::file_size(object));

				json_writer.Key("context");
				json_writer.String("t8-bnet");

				json_writer.Key("category");
				json_writer.Null();

				json_writer.Key("created");
				json_writer.Uint64(static_cast<int64_t>(time(nullptr)));

				json_writer.Key("modified");
				json_writer.Uint64(static_cast<int64_t>(time(nullptr)));

				json_writer.Key("extraData");
				json_writer.Null();
				json_writer.Key("extraDataSize");
				json_writer.Null();
				json_writer.Key("summaryContentLength");
				json_writer.Null();
				json_writer.Key("summaryChecksum");
				json_writer.Null();
				json_writer.Key("hasSummary");
				json_writer.Bool(false);

				json_writer.EndObject();

				json_writer.Key("tags");
				json_writer.StartArray();
				json_writer.EndArray();

				json_writer.Key("statistics");
				json_writer.StartArray();
				json_writer.EndArray();

				json_writer.EndObject();
			}
		}
		json_writer.EndArray();

		json_writer.EndObject();

		return json_buffer.GetString();
	}

	std::string generate_user_objects_count_json()
	{
		std::string userdata_directory = platform::get_userdata_directory();

		int files_count = 0;
		if (utilities::io::directory_exists(userdata_directory))
		{
			files_count = static_cast<int32_t>(utilities::io::list_files(userdata_directory).size());
		}


		rapidjson::StringBuffer json_buffer{};
		rapidjson::PrettyWriter<rapidjson::StringBuffer> json_writer(json_buffer);

		json_writer.StartObject();

		json_writer.Key("total");
		json_writer.Uint(files_count);

		json_writer.Key("noCategory");
		json_writer.Uint(files_count);

		json_writer.Key("categories");
		json_writer.StartObject();
		json_writer.EndObject();

		json_writer.EndObject();

		return json_buffer.GetString();
	}

	std::string construct_file_upload_result_json(const std::string& uploaded_file)
	{
		std::string file_path = get_user_file_path(uploaded_file);

		rapidjson::StringBuffer json_buffer{};
		rapidjson::PrettyWriter<rapidjson::StringBuffer> json_writer(json_buffer);

		json_writer.StartObject();

		json_writer.Key("metadata");
		json_writer.StartObject();

		json_writer.Key("owner");
		json_writer.String(std::format("bnet-{}", platform::bnet_get_userid()));

		json_writer.Key("expiresOn");
		json_writer.Uint(0);

		json_writer.Key("name");
		json_writer.String(uploaded_file);

		json_writer.Key("checksum");
		json_writer.String(get_user_file_checksum(file_path));

		json_writer.Key("acl");
		json_writer.String("public");

		json_writer.Key("objectID");
		json_writer.Uint(0);
		json_writer.Key("contentID");
		json_writer.Null();
		json_writer.Key("objectVersion");
		json_writer.String("");
		json_writer.Key("contentVersion");
		json_writer.Null();

		json_writer.Key("contentLength");
		json_writer.Uint64(utilities::io::file_size(file_path));

		json_writer.Key("context");
		json_writer.String("t8-bnet");

		json_writer.Key("category");
		json_writer.Null();

		json_writer.Key("created");
		json_writer.Uint64(static_cast<uint32_t>(time(nullptr)));

		json_writer.Key("modified");
		json_writer.Uint64(static_cast<uint32_t>(time(nullptr)));

		json_writer.Key("extraData");
		json_writer.Null();
		json_writer.Key("extraDataSize");
		json_writer.Null();
		json_writer.Key("summaryContentLength");
		json_writer.Null();
		json_writer.Key("summaryChecksum");
		json_writer.Null();
		json_writer.Key("hasSummary");
		json_writer.Bool(false);

		json_writer.EndObject();

		json_writer.EndObject();

		return json_buffer.GetString();
	}

	std::string construct_vectorized_upload_list_json(std::vector<objectMetadata> uploaded_files)
	{
		rapidjson::StringBuffer json_buffer{};
		rapidjson::PrettyWriter<rapidjson::StringBuffer> json_writer(json_buffer);

		json_writer.StartObject();

		json_writer.Key("objects");
		json_writer.StartArray();

		for (size_t i = 0; i < uploaded_files.size(); i++)
		{
			//std::string file_path = get_user_file_path(requested_items[i].owner, requested_items[i].name);

			json_writer.StartObject();

			json_writer.Key("metadata");
			json_writer.StartObject();

			json_writer.Key("owner");
			json_writer.String(uploaded_files[i].owner.data());

			json_writer.Key("expiresOn");
			json_writer.Uint(0);

			json_writer.Key("name");
			json_writer.String(uploaded_files[i].name.data());

			json_writer.Key("checksum");
			json_writer.String(uploaded_files[i].checksum.data());

			json_writer.Key("acl");
			json_writer.String("public");

			json_writer.Key("objectID");
			json_writer.Uint(0);
			json_writer.Key("contentID");
			json_writer.Null();
			json_writer.Key("objectVersion");
			json_writer.String("");
			json_writer.Key("contentVersion");
			json_writer.Null();

			json_writer.Key("contentLength");
			json_writer.Uint64(uploaded_files[i].contentLength);

			json_writer.Key("context");
			json_writer.String("t8-bnet");

			json_writer.Key("category");
			json_writer.Null();

			json_writer.Key("created");
			json_writer.Uint64(uploaded_files[i].created);

			json_writer.Key("modified");
			json_writer.Uint64(uploaded_files[i].modified);

			json_writer.Key("extraData");
			json_writer.Null();
			json_writer.Key("extraDataSize");
			json_writer.Null();
			json_writer.Key("summaryContentLength");
			json_writer.Null();
			json_writer.Key("summaryChecksum");
			json_writer.Null();
			json_writer.Key("hasSummary");
			json_writer.Bool(false);

			json_writer.EndObject();

			json_writer.Key("requestIndex");
			json_writer.Uint64(i);

			json_writer.EndObject();
		}

		json_writer.EndArray();

		json_writer.Key("errors");
		json_writer.StartArray();
		json_writer.EndArray();

		json_writer.Key("validationTokens");
		json_writer.StartArray();
		json_writer.EndArray();

		json_writer.EndObject();

		return json_buffer.GetString();
	}

	std::string construct_vectorized_upload_list_json(std::vector<std::string> uploaded_files)
	{
		std::vector<objectMetadata> files_metadata_list;

		for (std::string file : uploaded_files)
		{
			std::string file_path = get_user_file_path(file);
			int64_t timestamp = static_cast<int64_t>(time(nullptr));
			files_metadata_list.push_back({ std::format("bnet-{}", platform::bnet_get_userid()), file, get_user_file_checksum(file_path), utilities::io::file_size(file_path), timestamp, timestamp, get_user_file_content(file_path) });
		}

		return construct_vectorized_upload_list_json(files_metadata_list);
	}

	std::string serialize_objectstore_structed_buffer(std::string payload)
	{
		bdProtobufHelper header_1st;
		header_1st.writeString(1, "Content-Length", 16);
		header_1st.writeString(2, utilities::string::va("%u", payload.length()), 8);

		bdProtobufHelper header_2nd;
		header_2nd.writeString(1, "Authorization", 16);
		header_2nd.writeString(2, "Bearer project-bo4", 2048);


		bdProtobufHelper buffer;
		buffer.writeString(1, header_1st.buffer.data(), static_cast<uint32_t>(header_1st.buffer.length()));
		buffer.writeString(1, header_2nd.buffer.data(), static_cast<uint32_t>(header_2nd.buffer.length()));
		buffer.writeUInt64(2, 200); // Status Code; Anything NON-2XX is Treated as Error
		buffer.writeString(3, payload.data(), static_cast<uint32_t>(payload.length()));

		return buffer.buffer;
	}
}
