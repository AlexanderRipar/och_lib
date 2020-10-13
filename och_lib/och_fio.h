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
			share_read_write = 3,
			share_read_delete = 5,
			share_write_delete = 6,
			share_read_write_delete = 7,

			setptr_beg = 0,
			setptr_cur = 1,
			setptr_end = 2,
		};
	}

	using iohandle = void*;

	
	[[nodiscard]] iohandle open_file(const och::string filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode = fio::share_none);

	[[nodiscard]] iohandle create_file_mapper(const iohandle file, uint64_t size, uint32_t access_rights, const char* mapping_name = nullptr);

	[[nodiscard]] iohandle file_as_array(const iohandle file_mapper, uint32_t access_rights, uint64_t beg, uint64_t end);

	[[nodiscard]] int64_t get_filesize(const iohandle file);

	[[nodiscard]] och::memrun<char> read_from_file(const iohandle file, och::memrun<char> buf);

	uint32_t write_to_file(const iohandle file, och::string buf);

	bool close_file(const iohandle file);

	bool close_file_array(const iohandle file_array);

	bool delete_file(const och::string filename);

	bool set_fileptr(const iohandle file, int64_t set_to, uint32_t setptr_mode);

	bool set_filesize(const iohandle file, uint64_t bytes);

	och::memrun<char> get_filepath(const iohandle file, och::memrun<char> buf);

	[[nodiscard]] uint64_t get_last_write_time(const iohandle file);

	struct filehandle
	{
		const iohandle handle;

		filehandle(const och::string filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode = fio::share_none) : handle{ open_file(filename, access_rights, existing_mode, new_mode, share_mode) } {}

		filehandle(iohandle handle) : handle{ handle } {}

		~filehandle() { close_file(handle); }

		[[nodiscard]] och::memrun<char> read(och::memrun<char> buf) const { return read_from_file(handle, buf); }

		uint32_t write(const och::string buf) const { return write_to_file(handle, buf); }

		[[nodiscard]] uint64_t get_size() { return och::get_filesize(handle); }

		bool set_size(uint64_t bytes) { return och::set_filesize(handle, bytes); }

		och::memrun<char> path(och::memrun<char> buf) { return get_filepath(handle, buf); }

		bool set_fileptr(int64_t set_to, uint32_t setptr_mode = fio::setptr_beg) const { return och::set_fileptr(handle, set_to, setptr_mode); }
	};

	struct tempfilehandle : public filehandle
	{
		tempfilehandle(uint32_t share_mode = 0);

		~tempfilehandle();

		[[nodiscard]] och::memrun<char> read(och::memrun<char> buf) const { return read_from_file(handle, buf); }

		uint32_t write(const och::string buf) const { return write_to_file(handle, buf); }

		[[nodiscard]] uint64_t get_size() { return och::get_filesize(handle); }

		bool set_size(uint64_t bytes) { return och::set_filesize(handle, bytes); }

		och::memrun<char> path(och::memrun<char> buf) { return get_filepath(handle, buf); }

		bool set_fileptr(int64_t set_to, uint32_t setptr_mode = fio::setptr_beg) const { return och::set_fileptr(handle, set_to, setptr_mode); }
	};

	struct mapped_file
	{
	private:

		iohandle file;
		iohandle mapper;
		iohandle data;

	public:

		const uint32_t bytes;

		mapped_file(const och::string filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t mapping_size = 0, uint32_t mapping_offset = 0) :
			file{ open_file(filename, access_rights, existing_mode, new_mode) },
			mapper{ create_file_mapper(file, (uint64_t)mapping_size + mapping_offset, access_rights) },
			data{ file_as_array(mapper, access_rights, mapping_offset, (uint64_t) mapping_offset + mapping_size) },
			bytes{ mapping_size == 0 ? (uint32_t) get_filesize(file) : mapping_size }
		{}

		mapped_file(const iohandle file, uint32_t access_rights, uint32_t mapping_size = 0, uint32_t mapping_offset = 0) :
			file{ file },
			mapper{ create_file_mapper(file, (uint64_t)mapping_size + mapping_offset, access_rights) },
			data{ file_as_array(mapper, access_rights, mapping_offset, (uint64_t)mapping_offset + mapping_size) },
			bytes{ mapping_size == 0 ? (uint32_t)get_filesize(file) : mapping_size }
		{}

		~mapped_file()
		{
			och::close_file_array(data);
			och::close_file(mapper);
			och::close_file(file);
		}

		och::memrun<char> path(och::memrun<char> buf) const { return get_filepath(file, buf); }

		[[nodiscard]] bool is_valid() const { return data; }
	};
}
