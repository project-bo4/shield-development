#pragma once

namespace demonware
{
	class bdTitleUtilities final : public service
	{
	public:
		bdTitleUtilities();

	private:
		void verifyString(service_server* server, byte_buffer* buffer) const;
		void getTitleStats(service_server* server, byte_buffer* buffer) const;
		void getServerTime(service_server* server, byte_buffer* buffer) const;
		void areUsersOnline(service_server* server, byte_buffer* buffer) const;
		void getMAC(service_server* server, byte_buffer* buffer) const;
		void getUserNames(service_server* server, byte_buffer* buffer) const;
		void getUserIDs(service_server* server, byte_buffer* buffer) const;
	};
}
