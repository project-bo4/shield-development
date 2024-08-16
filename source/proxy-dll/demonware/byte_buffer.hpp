#pragma once

namespace demonware
{
	class byte_buffer final
	{
	public:
		byte_buffer() = default;

		explicit byte_buffer(std::string buffer) : buffer_(std::move(buffer))
		{
		}

		bool read_bool(bool* output);
		bool read_byte(char* output);
		bool read_ubyte(unsigned char* output);
		bool read_int16(short* output);
		bool read_uint16(unsigned short* output);
		bool read_int32(int* output);
		bool read_uint32(unsigned int* output);
		bool read_int64(__int64* output);
		bool read_uint64(unsigned __int64* output);
		bool read_float(float* output);
		bool read_string(char** output);
		bool read_string(char* output, int length);
		bool read_string(std::string* output);
		bool read_blob(char** output, int* length);
		bool read_blob(std::string* output);
		bool read_struct(char** output, int* length);
		bool read_struct(std::string* output);
		bool read_data_type(char expected);

		bool read_array_header(const unsigned char expected
			, unsigned int* element_count, unsigned int* element_size);

		template <typename T>
		bool read_array(const unsigned char expected, std::vector<T>* vec)
		{
			const auto using_types = this->is_using_data_types();
			this->set_use_data_types(false);

			uint32_t ItemCount, itemSize;

			if (!this->read_array_header(expected, &ItemCount, &itemSize)) return false;
			if (itemSize != sizeof(T)) return false;

			for (size_t i = 0; i < ItemCount; i++)
			{
				T item{};

				if (!this->read(sizeof(T), &item)) return false;

				vec->push_back(item);
			}

			this->set_use_data_types(using_types);
			return true;
		}

		template <typename T>
		bool read_array(const unsigned char expected, std::map<T, T>* map)
		{
			const auto using_types = this->is_using_data_types();
			this->set_use_data_types(false);

			uint32_t ItemCount, itemSize;

			if (!this->read_array_header(expected, &ItemCount, &itemSize)) return false;
			if (itemSize != sizeof(T)) return false;

			for (size_t i = 0; i < ItemCount / 2; i++)
			{
				T key{}, value{};

				if (!this->read(sizeof(T), &key)
					|| !this->read(sizeof(T), &value)
					) return false;

				map->insert({ key, value });
			}

			this->set_use_data_types(using_types);
			return true;
		}


		bool write_bool(bool data);
		bool write_byte(char data);
		bool write_ubyte(unsigned char data);
		bool write_int16(short data);
		bool write_uint16(unsigned short data);
		bool write_int32(int data);
		bool write_uint32(unsigned int data);
		bool write_int64(__int64 data);
		bool write_uint64(unsigned __int64 data);
		bool write_data_type(char data);
		bool write_float(float data);
		bool write_string(const char* data);
		bool write_string(const std::string& data);
		bool write_blob(const char* data, int length);
		bool write_blob(const std::string& data);
		bool write_struct(const char* data, int length);
		bool write_struct(const std::string& data);

		bool write_array_header(const unsigned char type,
			const unsigned int element_count, const unsigned int element_size);

		template <typename T>
		bool write_array(const unsigned char type, const std::vector<T>& vec)
		{
			const auto using_types = this->is_using_data_types();
			this->set_use_data_types(false);

			auto result = write_array_header(type, INT(vec.size()), sizeof(T));

			for (size_t i = 0; i < vec.size(); i++)
			{
				result &= this->write(sizeof(T), &vec[i]);
			}

			this->set_use_data_types(using_types);
			return result;
		}

		template <typename T>
		bool write_array(const unsigned char type, const std::map<T, T>& map)
		{
			const auto using_types = this->is_using_data_types();
			this->set_use_data_types(false);

			auto result = write_array_header(type, INT(map.size()) * 2, sizeof(T));

			for (const auto& item : map)
			{
				result &= this->write(sizeof(T), &item.first);
				result &= this->write(sizeof(T), &item.second);
			}

			this->set_use_data_types(using_types);
			return result;
		}


		bool read(int bytes, void* output);
		bool write(int bytes, const void* data);
		bool write(const std::string& data);

		void set_use_data_types(bool use_data_types);
		size_t size() const;

		bool is_using_data_types() const;

		std::string& get_buffer();
		std::string get_remaining();

		bool has_more_data() const;

	private:
		std::string buffer_;
		size_t current_byte_ = 0;
		bool use_data_types_ = true;
	};
}
