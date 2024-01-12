#pragma once

namespace hashes
{
	enum hashes_file_format
	{
		HFF_COMMON = 0,
		HFF_STRING,
		HFF_SERIOUS_COMPILER,
		HFF_COUNT
	};
	const char* lookup(uint64_t hash);
	const char* lookup_tmp(const char* type, uint64_t hash);
	void add_hash(uint64_t hash, const char* value);
	const char* get_format_name(hashes_file_format format);
	hashes_file_format get_format_idx(const char* name);
	bool load_file(std::filesystem::path& file, hashes_file_format format);
}