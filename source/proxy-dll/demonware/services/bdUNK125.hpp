#pragma once

namespace demonware
{
	class bdUNK125 final : public service
	{
	public:
		bdUNK125();

	private:
		void task_unk1(service_server* server, byte_buffer* buffer) const;
		void task_unk3(service_server* server, byte_buffer* buffer) const;
		void task_unk9(service_server* server, byte_buffer* buffer) const;
	};
}
