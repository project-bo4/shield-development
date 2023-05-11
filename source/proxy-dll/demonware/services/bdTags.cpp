#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdTags::bdTags() : service(52, "bdTags")
	{
		this->register_task(1, &bdTags::getTagsForEntityIDs);
		this->register_task(2, &bdTags::setTagsForEntityID);
		this->register_task(3, &bdTags::removeTagsForEntityID);
		this->register_task(4, &bdTags::removeAllTagsForEntityID);
		this->register_task(5, &bdTags::searchByTagsBase);
	}

	void bdTags::getTagsForEntityIDs(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdTags::setTagsForEntityID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdTags::removeTagsForEntityID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}
	
	void bdTags::removeAllTagsForEntityID(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}
	
	void bdTags::searchByTagsBase(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}
}
