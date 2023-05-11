#pragma once

namespace demonware
{
	class bdLootGeneration final : public service
	{
	public:
		bdLootGeneration();

	private:
		void getPlayerState(service_server* server, byte_buffer* buffer) const;
	};
}
