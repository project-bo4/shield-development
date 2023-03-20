#pragma once

namespace demonware
{
	class bdObjectStore final : public service
	{
	public:
		bdObjectStore();

	private:
		void getUserObject(service_server* server, byte_buffer* buffer) const;
		void getPublisherObject(service_server* server, byte_buffer* buffer) const;
		void listUserObjects(service_server* server, byte_buffer* buffer) const;
		void getUserObjectCounts(service_server* server, byte_buffer* buffer) const;
		void listPublisherObjectsByCategory(service_server* server, byte_buffer* buffer) const;
		void getUserObjectsVectorized(service_server* server, byte_buffer* buffer) const;
		void getPublisherObjectMetadatas(service_server* server, byte_buffer* buffer) const;
		void uploadUserObject(service_server* server, byte_buffer* buffer) const;
		void uploadUserObjectsVectorized(service_server* server, byte_buffer* buffer) const;
	};
}
