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

	class bdTimeStamp final : public bdTaskResult
	{
	public:
		uint32_t unix_time;

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
		std::string country_code; // Char [3]
		std::string country; // Char [65]
		std::string region; // Char [65]
		std::string city; // Char [129]
		float latitude;
		float longitude;
		uint32_t asn; // Autonomous System Number.
		std::string timezone;

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
		uint32_t m_tier0;
		uint32_t m_tier1;
		uint32_t m_tier2;
		uint32_t m_tier3;
		uint32_t m_confidence;

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

	class bdStructedDataBuffer final : public bdTaskResult
	{
	public:
		std::string structed_data_protobuffer;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_struct(this->structed_data_protobuffer);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_struct(&this->structed_data_protobuffer);
		}
	};

	class bdMarketplaceInventory final : public bdTaskResult
	{
	public:
		uint64_t m_userID;
		std::string m_accountType;
		uint32_t m_itemId;
		uint32_t m_itemQuantity;
		uint32_t m_itemXp;
		std::string m_itemData;
		uint32_t m_expireDateTime;
		int64_t m_expiryDuration;
		uint16_t m_collisionField;
		uint32_t m_modDateTime;
		uint32_t m_customSourceType;

		void serialize(byte_buffer* buffer) override
		{
			buffer->write_uint64(this->m_userID);
			buffer->write_string(this->m_accountType);
			buffer->write_uint32(this->m_itemId);
			buffer->write_uint32(this->m_itemQuantity);
			buffer->write_uint32(this->m_itemXp);
			buffer->write_blob(this->m_itemData);
			buffer->write_uint32(this->m_expireDateTime);
			buffer->write_int64(this->m_expiryDuration);
			buffer->write_uint16(this->m_collisionField);
			buffer->write_uint32(this->m_modDateTime);
			buffer->write_uint32(this->m_customSourceType);
		}

		void deserialize(byte_buffer* buffer) override
		{
			buffer->read_uint64(&this->m_userID);
			buffer->read_string(&this->m_accountType);
			buffer->read_uint32(&this->m_itemId);
			buffer->read_uint32(&this->m_itemQuantity);
			buffer->read_uint32(&this->m_itemXp);
			buffer->read_blob(&this->m_itemData);
			buffer->read_uint32(&this->m_expireDateTime);
			buffer->read_int64(&this->m_expiryDuration);
			buffer->read_uint16(&this->m_collisionField);
			buffer->read_uint32(&this->m_modDateTime);
			buffer->read_uint32(&this->m_customSourceType);
		}
	};

	class bdPublicProfileInfo final : public bdTaskResult
	{
	public:
		uint64_t m_entityID;
		int32_t m_VERSION;
		std::string m_ddl;

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
}
