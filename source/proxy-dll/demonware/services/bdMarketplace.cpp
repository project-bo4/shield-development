#include <std_include.hpp>
#include "../services.hpp"


namespace demonware
{
	bdMarketplace::bdMarketplace() : service(80, "bdMarketplace")
	{
		this->register_task(204, &bdMarketplace::getInventoryPaginated);
		this->register_task(245, &bdMarketplace::getBalancesV3);
	}

	void bdMarketplace::getInventoryPaginated(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdMarketplace::getBalancesV3(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_structed_reply(this->task_id());
		reply->send("");
	}
}
