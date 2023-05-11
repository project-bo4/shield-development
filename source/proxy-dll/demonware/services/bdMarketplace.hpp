#pragma once

namespace demonware
{
	class bdMarketplace final : public service
	{
	public:
		bdMarketplace();

	private:
		void getInventoryPaginated(service_server* server, byte_buffer* buffer) const;
		void getBalancesV3(service_server* server, byte_buffer* buffer) const;
	};
}
