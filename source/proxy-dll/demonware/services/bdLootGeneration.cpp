#include <std_include.hpp>
#include "../services.hpp"


namespace demonware
{
	bdLootGeneration::bdLootGeneration() : service(195, "bdLootGeneration")
	{
		this->register_task(2, &bdLootGeneration::getPlayerState);
	}

	void bdLootGeneration::getPlayerState(service_server* server, byte_buffer* buffer) const
	{
		auto reply = server->create_structed_reply(this->task_id());
		reply->send(""); // Un-handled
	}
}
