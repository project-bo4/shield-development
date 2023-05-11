#pragma once

namespace demonware
{
	class bdTags final : public service
	{
	public:
		bdTags();

	private:
		void getTagsForEntityIDs(service_server* server, byte_buffer* buffer) const;
		void setTagsForEntityID(service_server* server, byte_buffer* buffer) const;
		void removeTagsForEntityID(service_server* server, byte_buffer* buffer) const;
		void removeAllTagsForEntityID(service_server* server, byte_buffer* buffer) const;
		void searchByTagsBase(service_server* server, byte_buffer* buffer) const;
	};
}
