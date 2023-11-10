#pragma once

#include "tcp_server.hpp"

namespace demonware
{
	class fileshare_server : public tcp_server
	{
	public:
		using tcp_server::tcp_server;

	private:
		void handle(const std::string& packet) override;

		std::string http_header_time();
		std::string download_file(const std::string& file);
		std::string finalize_upload_file(const std::string& file, const std::vector<byte>& buffer);
	};
}
