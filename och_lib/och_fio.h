#pragma once

#include <cstdint>

namespace och
{
	namespace fio
	{
		constexpr uint32_t open_normal			= 0;
		constexpr uint32_t open_truncate		= 1;
		constexpr uint32_t open_fail			= 2;

		constexpr uint32_t access_read			= 2;
		constexpr uint32_t access_write			= 1;
		constexpr uint32_t access_readwrite		= 3;

		constexpr uint32_t share_none			= 0;
		constexpr uint32_t share_read			= 1;
		constexpr uint32_t share_write			= 2;
		constexpr uint32_t share_delete			= 4;
	}

	void* open_file(const char* filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode = fio::share_none);

	void* create_file_mapper(void* file, uint64_t size, uint32_t access_rights, const char* mapping_name = nullptr);

	void* file_as_array(void* file_mapper, uint32_t access_rights, uint64_t beg, uint64_t end);

	int64_t get_filesize(void* file);

	bool close_file(void* file);

	bool close_file_array(void* file_array);

	bool delete_file(const char* filename);

	template<typename T>
	struct file_array
	{
	private:

		void* file;
		void* mapper;
		void* data;

	public:

		const uint32_t size;

		file_array(const char* filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t mapping_size, uint32_t mapping_offset = 0) :
			file{ open_file(filename, access_rights, existing_mode, new_mode) },
			mapper{ create_file_mapper(file, (uint64_t)mapping_size + mapping_offset, access_rights) },
			data{ file_as_array(mapper, access_rights, mapping_offset, (uint64_t) mapping_offset + mapping_size) },
			size{ mapping_size == 0 ? (uint32_t) get_filesize(file) : mapping_size }
		{}

		~file_array()
		{
			och::close_file_array(data);
			och::close_file(mapper);
			och::close_file(file);
		}

		T& operator[](size_t i)
		{
			return reinterpret_cast<T*>(data)[i];
		}

		bool is_valid()
		{
			return data;
		}
	};
}
