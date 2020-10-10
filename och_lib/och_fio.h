#pragma once

#include <cstdint>
#include "och_memrun.h"

namespace och
{
	namespace fio
	{
		enum _fio : uint32_t
		{
			open_normal = 0,
			open_truncate = 1,
			open_fail = 2,

			access_read = 2,
			access_write = 1,
			access_readwrite = 3,

			share_none = 0,
			share_read = 1,
			share_write = 2,
			share_delete = 4,

			setptr_beg = 0,
			setptr_cur = 1,
			setptr_end = 2,
		};

	}

	using iohandle = void*;

	iohandle open_file(const och::string filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode = fio::share_none);

	iohandle create_file_mapper(const iohandle file, uint64_t size, uint32_t access_rights, const char* mapping_name = nullptr);

	iohandle file_as_array(const iohandle file_mapper, uint32_t access_rights, uint64_t beg, uint64_t end);

	int64_t get_filesize(const iohandle file);

	uint32_t freadbytes(const iohandle file, char* dst, uint32_t bytes);

	template<typename T>
	och::memrun<T> read_from_file(const iohandle file, och::memrun<T> dst)
	{
		return och::memrun(dst.begin(), dst.begin() + freadbytes(file, reinterpret_cast<char*>(dst.begin()), static_cast<uint32_t>(dst.len() * sizeof(T))));
	}

	uint32_t fwritebytes(const iohandle file, const char* src, uint32_t bytes);

	template<typename T>
	bool write_to_file(const iohandle file, och::memrun<T> src)
	{
		return fwritebytes(file, src.begin(), static_cast<uint32_t>(src.len() * sizeof(T)));
	}

	bool close_file(const iohandle file);

	bool close_file_array(const iohandle file_array);

	bool delete_file(const och::string filename);

	bool set_fileptr(const iohandle file, int64_t set_to, uint32_t setptr_mode);

	struct filehandle
	{
		const iohandle handle;

		filehandle(const och::string filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode = fio::share_none) : handle{ open_file(filename, access_rights, existing_mode, new_mode, share_mode) } {}

		filehandle(iohandle handle) : handle{ handle } {}

		~filehandle() { close_file(handle); }

		template<typename T> och::memrun<T> read(och::memrun<T> dst) const { return read_from_file(handle, dst); }

		template<typename T> bool write(const och::memrun<T> src) const { return write_to_file(handle, src); }

		bool set_fileptr(int64_t set_to, uint32_t setptr_mode = fio::setptr_beg) const { return och::set_fileptr(handle, set_to, setptr_mode); }
	};

	struct tempfilehandle
	{
		const iohandle handle;

		tempfilehandle();

		~tempfilehandle();

		template<typename T> och::memrun<T> read(och::memrun<T> dst) const { return read_from_file(handle, dst); }

		template<typename T> bool write(const och::memrun<T> src) const { return write_to_file(handle, src); }

		bool set_fileptr(int64_t set_to, uint32_t setptr_mode = fio::setptr_beg) const { return och::set_fileptr(handle, set_to, setptr_mode); }
	};

	template<typename T>
	struct file_array
	{
	private:

		iohandle file;
		iohandle mapper;
		iohandle data;

	public:

		const uint32_t size;

		file_array(const och::string filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t mapping_size, uint32_t mapping_offset = 0) :
			file{ open_file(filename, access_rights, existing_mode, new_mode) },
			mapper{ create_file_mapper(file, (uint64_t)mapping_size + mapping_offset, access_rights) },
			data{ file_as_array(mapper, access_rights, mapping_offset, (uint64_t) mapping_offset + mapping_size) },
			size{ mapping_size == 0 ? (uint32_t) get_filesize(file) : mapping_size }
		{}

		file_array(const iohandle file, uint32_t access_rights, uint32_t mapping_size, uint32_t mapping_offset = 0) :
			file{ file },
			mapper{ create_file_mapper(file, (uint64_t)mapping_size + mapping_offset, access_rights) },
			data{ file_as_array(mapper, access_rights, mapping_offset, (uint64_t)mapping_offset + mapping_size) },
			size{ mapping_size == 0 ? (uint32_t)get_filesize(file) : mapping_size }
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

		const T& operator[](size_t i) const
		{
			return reinterpret_cast<T*>(data)[i];
		}

		bool is_valid()
		{
			return data;
		}
	};
}
