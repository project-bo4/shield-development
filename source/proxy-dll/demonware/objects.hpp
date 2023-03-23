#pragma once

namespace demonware
{
	struct objectID
	{
		std::string owner;
		std::string name;
	};

	struct objectMetadata
	{
		std::string owner;
		std::string name;
		std::string checksum;
		int64_t contentLength;
		int64_t created;
		int64_t modified;
		std::string contentURL;
	};

	std::string get_user_file_path(const std::string& file);

	std::string generate_publisher_objects_list_json(const std::string& category);

	std::string generate_user_objects_list_json();
	std::string generate_user_objects_count_json();

	std::string deliver_user_objects_vectorized_json(std::vector<objectMetadata> requested_items);
	std::string deliver_user_objects_vectorized_json(std::vector<objectID> requested_items);

	std::string construct_vectorized_upload_list_json(std::vector<objectMetadata> uploaded_files);
	std::string construct_vectorized_upload_list_json(std::vector<std::string> uploaded_files);

	std::string serialize_objectstore_structed_buffer(std::string payload);
}
