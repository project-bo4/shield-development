#include <std_include.hpp>
#include "fileshare_server.hpp"

#include "../fileshare.hpp"
#include <utilities/string.hpp>
#include <utilities/io.hpp>

std::string fileshare_upload_file_name{};
std::vector<byte> fileshare_upload_buffer{};

namespace demonware
{
	void fileshare_server::handle(const std::string& packet)
	{
		static bool upload_in_progress = false;

		if (packet.starts_with("GET"))
		{
			std::string file = utilities::string::split(utilities::string::split(packet, '\n')[0], ' ')[1].substr(1, std::string::npos);

			this->send(download_file(file));
		}
		else if (packet.starts_with("PUT"))
		{
			fileshare_upload_file_name = utilities::string::split(utilities::string::split(packet, '\n')[0], ' ')[1].substr(1, std::string::npos);
			fileshare_upload_buffer.clear();

			logger::write(logger::LOG_TYPE_DEBUG, "[DW]: [fileshare]: upload stream started for '%s'\n", fileshare_upload_file_name.data());

			this->send("");
			upload_in_progress = true;
		}
		else
		{
			if (!upload_in_progress)
			{
				std::string http_response = "HTTP/1.1 501 Not Implemented\r\n";
				http_response.append("Connection: close\r\n\r\n");

				return this->send(http_response);
			}

			std::string chunk_header = utilities::string::split(packet, "\r\n")[0];
			size_t chunk_length = std::stoul(chunk_header, nullptr, 16);

			if (chunk_length != 0)
			{
				fileshare_upload_buffer.insert(fileshare_upload_buffer.end(), &packet[chunk_header.length() + 2], &packet[chunk_header.length() + 2 + chunk_length]);
			}
			else
			{
				std::string response = finalize_upload_file(fileshare_upload_file_name, fileshare_upload_buffer);
				upload_in_progress = false;
				this->send(response);
			}
		}
	}

	std::string fileshare_server::http_header_time()
	{
		// header time
		char date[64];
		const auto now = time(nullptr);
		tm gmtm{};
		gmtime_s(&gmtm, &now);
		strftime(date, 64, "%a, %d %b %G %T", &gmtm);

		return std::format("{} GMT", date);
	}

	std::string fileshare_server::download_file(const std::string& file)
	{
		std::string http_response{}, file_buffer{};

		if (utilities::io::read_file(fileshare::get_file_path(file), &file_buffer))
		{
			logger::write(logger::LOG_TYPE_DEBUG, "[DW]: [fileshare]: hosting requested file '%s'\n", file.data());

			http_response.append("HTTP/1.1 200 OK\r\n");
			http_response.append("Server: Apache/0.0.0 (Win64)\r\n");
			http_response.append("Content-Type: application/octet-stream\r\n");
			http_response.append(utilities::string::va("Date: %s\r\n", http_header_time().data()));
			http_response.append(utilities::string::va("Content-Length: %d\r\n\r\n", file_buffer.length()));
			http_response.append(file_buffer);
		}
		else
		{
			logger::write(logger::LOG_TYPE_DEBUG, "[DW]: [fileshare]: couldnt find requested file '%s'\n", file.data());

			file_buffer = "<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";

			http_response.append("HTTP/1.1 404 Not Found\r\n");
			http_response.append("Server: Apache/0.0.0 (Win64)\r\n");
			http_response.append("Content-Type: text/html\r\n");
			http_response.append(utilities::string::va("Date: %s\r\n", http_header_time().data()));
			http_response.append(utilities::string::va("Content-Length: %d\r\n\r\n", file_buffer.length()));
		}

		return http_response;
	}

	std::string fileshare_server::finalize_upload_file(const std::string& file, const std::vector<byte>& buffer)
	{
		if(utilities::io::write_file(fileshare::get_file_path(file), std::string(buffer.begin(), buffer.end())))
			logger::write(logger::LOG_TYPE_DEBUG, "[DW]: [fileshare]: file upload finalized; saved as '%s'\n", fileshare_upload_file_name.data());
		else
			logger::write(logger::LOG_TYPE_DEBUG, "[DW]: [fileshare]: error on saving uploaded file '%s'\n", fileshare_upload_file_name.data());

		return "HTTP/1.1 201 Created\r\nContent-Length: 0\r\n\r\n";
	}
}