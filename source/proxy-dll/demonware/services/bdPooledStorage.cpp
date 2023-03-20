#include <std_include.hpp>
#include "../services.hpp"

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

	void bdPooledStorage::getPooledMetaDataByID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdPooledStorage::_preUpload(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdPooledStorage::_postUploadFile(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
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
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdPooledStorage::_preUploadSummary(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdPooledStorage::_postUploadSummary(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
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