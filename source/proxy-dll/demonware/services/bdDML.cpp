#include <std_include.hpp>
#include "../services.hpp"

#include <utilities/io.hpp>

namespace demonware
{
	bdDML::bdDML() : service(27, "bdDML")
	{
		this->register_task(3, &bdDML::getUserHierarchicalData);
	}

	void bdDML::getUserHierarchicalData(service_server* server, byte_buffer* /*buffer*/) const
	{
		auto result = new bdDMLHierarchicalInfo;
		result->country_code = "US";
		result->country = "United States";
		result->region = "New York";
		result->city = "New York";
		result->latitude = 0;
		result->longitude = 0;

		result->asn = 0x2119;
		result->timezone = "+01:00";

		result->m_tier0 = 0;
		result->m_tier1 = 0;
		result->m_tier2 = 0;
		result->m_tier3 = 0;
		result->m_confidence = 0;


		auto reply = server->create_reply(this->task_id());
		reply->add(result);
		reply->send();
	}
}
