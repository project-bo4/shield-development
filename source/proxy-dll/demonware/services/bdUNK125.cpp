#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdUNK125::bdUNK125() : service(125, "bdUNK125")
	{
		this->register_task(1, &bdUNK125::task_unk1);
		this->register_task(2, &bdUNK125::task_unk2);
		this->register_task(3, &bdUNK125::task_unk3);
		this->register_task(9, &bdUNK125::task_unk9);
	}

	void bdUNK125::task_unk1(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_structed_reply(this->task_id());
		reply->send("");
	}
	
	void bdUNK125::task_unk2(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_structed_reply(this->task_id());
		reply->send("");
	}
	
	void bdUNK125::task_unk3(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_structed_reply(this->task_id());
		reply->send("");
	}
	
	void bdUNK125::task_unk9(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_structed_reply(this->task_id());
		reply->send("");
	}
}
