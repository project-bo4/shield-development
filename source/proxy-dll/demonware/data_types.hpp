#pragma once

#include "byte_buffer.hpp"

namespace demonware
{
	class bdTaskResult
	{
	public:
		virtual ~bdTaskResult() = default;

		virtual void serialize(byte_buffer*)
		{
		}

		virtual void deserialize(byte_buffer*)
		{
		}
	};

	class bdStringResult final : public bdTaskResult
	{
	public:
		std::string content = "";

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_string(this->content);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_string(&this->content);
		}
	};

	class bdUInt64Result final : public bdTaskResult
	{
	public:
		uint64_t value = 0;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_uint64(this->value);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_uint64(&this->value);
		}
	};
	
	class bdBoolResult final : public bdTaskResult
	{
	public:
		bool value = false;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_bool(this->value);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_bool(&this->value);
		}
	};

	class bdTimeStamp final : public bdTaskResult
	{
	public:
		uint32_t unix_time = 0;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_uint32(this->unix_time);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_uint32(&this->unix_time);
		}
	};

	class bdDMLInfo : public bdTaskResult
	{
	public:
		std::string country_code = ""; // Char [3]
		std::string country = ""; // Char [65]
		std::string region = ""; // Char [65]
		std::string city = ""; // Char [129]
		float latitude = 0.0f;
		float longitude = 0;
		uint32_t asn = 0; // Autonomous System Number.
		std::string timezone = "";

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_string(this->country_code);
			buffer->write_string(this->country);
			buffer->write_string(this->region);
			buffer->write_string(this->city);
			buffer->write_float(this->latitude);
			buffer->write_float(this->longitude);
			buffer->write_uint32(this->asn);
			buffer->write_string(this->timezone);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_string(&this->country_code);
			buffer->read_string(&this->country);
			buffer->read_string(&this->region);
			buffer->read_string(&this->city);
			buffer->read_float(&this->latitude);
			buffer->read_float(&this->longitude);
			buffer->read_uint32(&this->asn);
			buffer->read_string(&this->timezone);
		}
	};

	class bdDMLHierarchicalInfo final : public bdDMLInfo
	{
	public:
		uint32_t m_tier0 = 0;
		uint32_t m_tier1 = 0;
		uint32_t m_tier2 = 0;
		uint32_t m_tier3 = 0;
		uint32_t m_confidence = 0;

		void serialize(byte_buffer* buffer) override
		{
			bdDMLInfo::serialize(buffer);

			buffer->write_uint32(this->m_tier0);
			buffer->write_uint32(this->m_tier1);
			buffer->write_uint32(this->m_tier2);
			buffer->write_uint32(this->m_tier3);
			buffer->write_uint32(this->m_confidence);
		}

		void deserialize(byte_buffer* buffer) override
		{
			bdDMLInfo::deserialize(buffer);

			buffer->read_uint32(&this->m_tier0);
			buffer->read_uint32(&this->m_tier1);
			buffer->read_uint32(&this->m_tier2);
			buffer->read_uint32(&this->m_tier3);
			buffer->read_uint32(&this->m_confidence);
		}
	};

	class bdPublicProfileInfo final : public bdTaskResult
	{
	public:
		uint64_t m_entityID = 0;
		int32_t m_VERSION = 0;
		std::string m_ddl = "";

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_uint64(this->m_entityID);
			buffer->write_int32(this->m_VERSION);
			buffer->write_blob(this->m_ddl);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_uint64(&this->m_entityID);
			buffer->read_int32(&this->m_VERSION);
			buffer->read_blob(&this->m_ddl);
		}
	};
	
	class bdFileMetaData final : public bdTaskResult
	{
	public:
		uint64_t m_fileID = 0;
		uint32_t m_createTime = 0;
		uint32_t m_modifedTime = 0;
		uint32_t m_fileSize = 0;
		uint64_t m_ownerID = 0;
		std::string m_ownerName = "";
		uint16_t m_fileSlot = 0;
		std::string m_fileName = "";
		std::string m_url = "";
		uint16_t m_category = 0;
		std::string m_metaData = "";
		uint32_t m_summaryFileSize = 0;
		std::map<uint64_t, uint64_t> m_tags;
		uint32_t m_numCopiesMade = 0;
		uint64_t m_originID = 0;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_uint64(this->m_fileID);
			buffer->write_uint32(this->m_createTime);
			buffer->write_uint32(this->m_modifedTime);
			buffer->write_uint32(this->m_fileSize);
			buffer->write_uint64(this->m_ownerID);
			buffer->write_string(this->m_ownerName);
			buffer->write_uint16(this->m_fileSlot);
			buffer->write_string(this->m_fileName);
			buffer->write_string(this->m_url);
			buffer->write_uint16(this->m_category);
			buffer->write_blob(this->m_metaData);
			buffer->write_uint32(this->m_summaryFileSize);
			buffer->write_array(10, m_tags);
			buffer->write_uint32(this->m_numCopiesMade);
			buffer->write_uint64(this->m_originID);
		}
	};

	class bdURL final : public bdTaskResult
	{
	public:
		std::string m_url = "";
		uint16_t m_serverType = 0;
		std::string m_serverIndex = "";
		uint64_t m_fileID = 0;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_string(this->m_url);
			buffer->write_uint16(this->m_serverType);
			buffer->write_string(this->m_serverIndex);
			buffer->write_uint64(this->m_fileID);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_string(&this->m_url);
			buffer->read_uint16(&this->m_serverType);
			buffer->read_string(&this->m_serverIndex);
			buffer->read_uint64(&this->m_fileID);
		}
	};
		
	class bdStructedDataBuffer final : public bdTaskResult
	{
	public:
		std::string structed_data_protobuffer = "";

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_struct(this->structed_data_protobuffer);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_struct(&this->structed_data_protobuffer);
		}
	};
}
