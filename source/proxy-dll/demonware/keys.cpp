#include <std_include.hpp>
#include "keys.hpp"

#include <utilities/cryptography.hpp>
#include <utilities/string.hpp>
#include <utilities/io.hpp>

#include "resource.hpp"
#include <utilities/nt.hpp>


namespace demonware
{
	struct data_t
	{
		char m_session_key[24];
		char m_response[8];
		char m_hmac_key[20];
		char m_enc_key[16];
		char m_dec_key[16];
	} data{};

	std::string packet_buffer;

	void calculate_hmacs(const char* dataxx, const unsigned int data_size,
		const char* key, const unsigned int key_size,
		char* dest, const unsigned int dest_size)
	{
		char buffer[512];
		unsigned int pos = 0;
		unsigned int out_offset = 0;
		char count = 1;
		std::string result;

		// buffer add key
		std::memcpy(&buffer[pos], key, key_size);
		pos += key_size;

		// buffer add count
		buffer[pos] = count;
		pos++;

		// calculate hmac
		result = utilities::cryptography::hmac_sha1::compute(std::string(buffer, pos), std::string(dataxx, data_size));

		// save output
		std::memcpy(dest, result.data(), std::min(20u, (dest_size - out_offset)));
		out_offset = 20;

		// second loop
		while (true)
		{
			// if we filled the output buffer, exit
			if (out_offset >= dest_size)
				break;

			// buffer add last result
			pos = 0;
			std::memcpy(&buffer[pos], result.data(), 20);
			pos += 20;

			// buffer add key
			std::memcpy(&buffer[pos], key, key_size);
			pos += key_size;

			// buffer add count
			count++;
			buffer[pos] = count;
			pos++;

			// calculate hmac
			result = utilities::cryptography::hmac_sha1::compute(std::string(buffer, pos), std::string(dataxx, data_size));

			// save output
			std::memcpy(dest + out_offset, result.data(), std::min(20u, (dest_size - out_offset)));
			out_offset += 20;
		}
	}

	void derive_keys_iw8()
	{
		std::string BD_AUTH_TRAFFIC_SIGNING_KEY = utilities::nt::load_resource(DW_AUTH_TRAFFIC_SIGNING_KEY);

		const auto packet_hash = utilities::cryptography::sha1::compute(packet_buffer);

		char out_1[24];
		calculate_hmacs(data.m_session_key, 24, BD_AUTH_TRAFFIC_SIGNING_KEY.data(), 294, out_1, 24);

		auto data_3 = utilities::cryptography::hmac_sha1::compute(std::string(out_1, 24), packet_hash);

		char out_2[16];
		calculate_hmacs(data_3.data(), 20, "CLIENTCHAL", 10, out_2, 16);

		char out_3[72];
		calculate_hmacs(data_3.data(), 20, "BDDATA", 6, out_3, 72);


		std::memcpy(data.m_response, &out_2[8], 8);
		std::memcpy(data.m_hmac_key, &out_3[20], 20);
		std::memcpy(data.m_dec_key, &out_3[40], 16);
		std::memcpy(data.m_enc_key, &out_3[56], 16);

#ifndef NDEBUG
		logger::write(logger::LOG_TYPE_DEBUG, "[DW] Response id: %s", utilities::string::dump_hex(std::string(&out_2[8], 8)).data());
		logger::write(logger::LOG_TYPE_DEBUG, "[DW] Hash verify: %s", utilities::string::dump_hex(std::string(&out_3[20], 20)).data());
		logger::write(logger::LOG_TYPE_DEBUG, "[DW] AES dec key: %s", utilities::string::dump_hex(std::string(&out_3[40], 16)).data());
		logger::write(logger::LOG_TYPE_DEBUG, "[DW] AES enc key: %s", utilities::string::dump_hex(std::string(&out_3[56], 16)).data());
		logger::write(logger::LOG_TYPE_DEBUG, "[DW] Bravo 6, going dark.");
#endif
	}

	void queue_packet_to_hash(const std::string& packet)
	{
		packet_buffer.append(packet);
	}

	void set_session_key(const std::string& key)
	{
		std::memcpy(data.m_session_key, key.data(), 24);
	}

	std::string get_decrypt_key()
	{
		return std::string(data.m_dec_key, 16);
	}

	std::string get_encrypt_key()
	{
		return std::string(data.m_enc_key, 16);
	}

	std::string get_hmac_key()
	{
		return std::string(data.m_hmac_key, 20);
	}

	std::string get_response_id()
	{
		return std::string(data.m_response, 8);
	}
}
