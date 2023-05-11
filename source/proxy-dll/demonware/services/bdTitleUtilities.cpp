#include <std_include.hpp>
#include "../services.hpp"

namespace demonware
{
	bdTitleUtilities::bdTitleUtilities() : service(12, "bdTitleUtilities")
	{
		this->register_task(1, &bdTitleUtilities::verifyString);
		this->register_task(2, &bdTitleUtilities::getTitleStats);
		this->register_task(6, &bdTitleUtilities::getServerTime);
		this->register_task(7, &bdTitleUtilities::areUsersOnline);
		this->register_task(8, &bdTitleUtilities::getMAC);
		this->register_task(9, &bdTitleUtilities::getUserNames);
		this->register_task(10, &bdTitleUtilities::getUserIDs);
	}

	void bdTitleUtilities::getServerTime(service_server* server, byte_buffer* /*buffer*/) const
	{
		auto* const time_result = new bdTimeStamp;
		time_result->unix_time = uint32_t(time(nullptr));

		auto reply = server->create_reply(this->task_id());
		reply->add(time_result);
		reply->send();
	}

	void bdTitleUtilities::verifyString(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdTitleUtilities::getTitleStats(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdTitleUtilities::areUsersOnline(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdTitleUtilities::getMAC(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdTitleUtilities::getUserNames(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}

	void bdTitleUtilities::getUserIDs(service_server* server, byte_buffer* /*buffer*/) const
	{
		// TODO:
		auto reply = server->create_reply(this->task_id());
		reply->send();
	}
}
