#include <std_include.hpp>
#include "../services.hpp"
#include "../fileshare.hpp"
#include "component/platform.hpp"

#include <utilities/io.hpp>

namespace demonware
{
	bdPooledStorage::bdPooledStorage() : service(58, "bdPooledStorage")
	{
		this->register_task(1, &bdPooledStorage::getPooledMetaDataByID);
		this->register_task(5, &bdPooledStorage::_preUpload);
		this->register_task(6, &bdPooledStorage::_postUploadFile);
		this->register_task(8, &bdPooledStorage::remove);
		this->register_task(9, &bdPooledStorage::_preDownload);
		this->register_task(17, &bdPooledStorage::_preUploadSummary);
		this->register_task(18, &bdPooledStorage::_postUploadSummary);
		this->register_task(19, &bdPooledStorage::_preDownloadSummary);
		this->register_task(20, &bdPooledStorage::_preUploadMultiPart);
		this->register_task(21, &bdPooledStorage::_postUploadMultiPart);
		this->register_task(22, &bdPooledStorage::_preDownloadMultiPart);
	}

	void bdPooledStorage::getPooledMetaDataByID(service_server* server, byte_buffer* buffer) const
	{
		std::vector<uint64_t> requested_files;
		buffer->read_array(10, &requested_files);

		auto reply = server->create_reply(this->task_id());

		for (auto fileID : requested_files)
		{
			std::string metafile = fileshare::get_metadata_path(fileshare::get_file_name(fileID));

			fileshare::FileMetadata metadata;
			if (metadata.ReadMetaDataJson(metafile, metadata.FILE_STATE_DESCRIBED)
				&& utilities::io::file_exists(fileshare::get_file_path(metadata.ioFileName)))
			{
				auto taskResult = new bdFileMetaData;
				metadata.MetadataTaskResult(taskResult, false);

				reply->add(taskResult);
			}
		}

		reply->send();
	}

	void bdPooledStorage::_preUpload(service_server* server, byte_buffer* buffer) const
	{
		std::string filename; uint16_t category;
		buffer->read_string(&filename);
		buffer->read_uint16(&category);

		uint32_t timestamp = static_cast<uint32_t>(time(nullptr));

		fileshare::FileMetadata metadata;
		metadata.file.id = timestamp;
		metadata.file.name = filename;
		metadata.file.timestamp = timestamp;

		metadata.author.xuid = platform::bnet_get_userid();
		metadata.author.name = platform::bnet_get_username();

		metadata.category = static_cast<fileshare::fileshareCategory_e>(category);
		metadata.ioFileName = fileshare::get_file_name(metadata.file.id, metadata.category);

		metadata.WriteMetaDataJson(fileshare::get_metadata_path(metadata.ioFileName), metadata.FILE_STATE_UPLOADING);

		auto reply = server->create_reply(this->task_id());

		auto result = new bdURL;
		result->m_url = fileshare::get_file_url(metadata.ioFileName);
		result->m_serverType = 8;
		result->m_serverIndex = "fs";
		result->m_fileID = metadata.file.id;

		reply->add(result);
		reply->send();
	}

	void bdPooledStorage::_postUploadFile(service_server* server, byte_buffer* buffer) const
	{
		uint64_t fileID; uint32_t fileSize;
		uint16_t serverType; std::string serverIndex;

		buffer->read_uint64(&fileID);
		buffer->read_uint16(&serverType);
		buffer->read_string(&serverIndex);
		buffer->read_uint32(&fileSize);

		auto metafile = fileshare::get_metadata_path(fileshare::get_file_name(fileID));

		fileshare::FileMetadata metadata;
		if (metadata.ReadMetaDataJson(metafile)) {
			auto ioSize = utilities::io::file_size(fileshare::get_file_path(metadata.ioFileName));
			metadata.file.size = fileSize;
			metadata.ioFileSize = static_cast<uint32_t>(ioSize);

			metadata.WriteMetaDataJson(metafile, metadata.FILE_STATE_UPLOADED);
		}

		auto reply = server->create_reply(this->task_id());

		auto result = new bdUInt64Result;
		result->value = fileID;
		reply->add(result);

		reply->send();
	}

	void bdPooledStorage::remove(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdPooledStorage::_preDownload(service_server* server, byte_buffer* buffer) const
	{
		uint64_t fileID;
		buffer->read_uint64(&fileID);

		std::string metafile = fileshare::get_metadata_path(fileshare::get_file_name(fileID));

		fileshare::FileMetadata metadata;
		if (metadata.ReadMetaDataJson(metafile, metadata.FILE_STATE_DESCRIBED)
			&& utilities::io::file_exists(fileshare::get_file_path(metadata.ioFileName)))
		{
			auto reply = server->create_reply(this->task_id());

			auto taskResult = new bdFileMetaData;
			metadata.MetadataTaskResult(taskResult, true);
			reply->add(taskResult);
			
			reply->send();
		}
		else
		{
			auto reply = server->create_reply(this->task_id(), 2000/*BD_CONTENTSTREAMING_FILE_NOT_AVAILABLE*/);
			reply->send();
		}
	}

	void bdPooledStorage::_preUploadSummary(service_server* server, byte_buffer* buffer) const
	{
		uint64_t fileID{}; uint32_t fileSize{};
		buffer->read_uint64(&fileID);
		buffer->read_uint32(&fileSize);

		auto metafile = fileshare::get_metadata_path(fileshare::get_file_name(fileID));

		fileshare::FileMetadata metadata;
		if (metadata.ReadMetaDataJson(metafile)) {
			buffer->read_blob(&metadata.ddlMetadata);
			buffer->read_array(10, &metadata.tags);

			metadata.WriteMetaDataJson(metafile, metadata.FILE_STATE_DESCRIBED);
		}

		auto reply = server->create_reply(this->task_id(), 108/*BD_SERVICE_NOT_AVAILABLE*/);
		reply->send();
	}

	void bdPooledStorage::_postUploadSummary(service_server* server, byte_buffer* /*buffer*/) const
	{
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdPooledStorage::_preDownloadSummary(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdPooledStorage::_preUploadMultiPart(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdPooledStorage::_postUploadMultiPart(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdPooledStorage::_preDownloadMultiPart(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}
}