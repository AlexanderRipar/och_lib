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

	[[nodiscard]] iohandle open_file(const och::string filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode = fio::share_none) noexcept;

	[[nodiscard]] iohandle create_file_mapper(iohandle file, uint64_t size, uint32_t access_rights, const char* mapping_name = nullptr) noexcept;

	[[nodiscard]] iohandle file_as_array(iohandle file_mapper, uint32_t access_rights, uint64_t beg, uint64_t end) noexcept;

	[[nodiscard]] int64_t get_filesize(iohandle file) noexcept;

	[[nodiscard]] och::memrun<char> read_from_file(iohandle file, och::memrun<char> buf) noexcept;

	uint32_t write_to_file(iohandle file, const och::string buf) noexcept;

	bool close_file(iohandle file) noexcept;

	bool close_file_array(iohandle file_array) noexcept;

	bool delete_file(const och::string filename) noexcept;

	bool file_seek(iohandle file, int64_t set_to, uint32_t setptr_mode) noexcept;

	bool set_filesize(iohandle file, uint64_t bytes) noexcept;

	och::memrun<char> get_filepath(iohandle file, och::memrun<char> buf) noexcept;

	[[nodiscard]] uint64_t get_last_write_time(iohandle file) noexcept;

	struct filehandle
	{
		const iohandle handle;

		filehandle(const och::string filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode = fio::share_none) noexcept : handle{ open_file(filename, access_rights, existing_mode, new_mode, share_mode) } {}

		filehandle(iohandle handle) noexcept : handle{ handle } {}

		~filehandle() noexcept { close_file(handle); }

		[[nodiscard]] och::memrun<char> read(och::memrun<char> buf) const noexcept { return read_from_file(handle, buf); }

		uint32_t write(const och::string buf) const noexcept { return write_to_file(handle, buf); }

		uint32_t write(const och::memrun<char> buf) const noexcept { return write_to_file(handle, { buf.beg, buf.end }); }

		[[nodiscard]] uint64_t get_size() noexcept { return get_filesize(handle); }

		bool set_size(uint64_t bytes) const noexcept { return set_filesize(handle, bytes); }

		och::memrun<char> path(och::memrun<char> buf) const noexcept { return get_filepath(handle, buf); }

		bool seek(int64_t set_to, uint32_t setptr_mode = fio::setptr_beg) const noexcept { return file_seek(handle, set_to, setptr_mode); }

		[[nodiscard]] bool operator!() { return !handle; }
	};

	struct tempfilehandle : public filehandle
	{
		tempfilehandle(uint32_t share_mode = 0) noexcept;

		~tempfilehandle() noexcept;

		[[nodiscard]] och::memrun<char> read(och::memrun<char> buf) const noexcept { return read_from_file(handle, buf); }

		uint32_t write(const och::string buf) const noexcept { return write_to_file(handle, buf); }

		[[nodiscard]] uint64_t get_size() noexcept { return och::get_filesize(handle); }

		bool set_size(uint64_t bytes) noexcept { return och::set_filesize(handle, bytes); }

		och::memrun<char> path(och::memrun<char> buf) noexcept { return get_filepath(handle, buf); }

		bool set_fileptr(int64_t set_to, uint32_t setptr_mode = fio::setptr_beg) const noexcept { return och::file_seek(handle, set_to, setptr_mode); }
	};

	struct mapped_file
	{
	private:

		iohandle file;
		iohandle mapper;
		iohandle data;

	public:

		const uint32_t bytes;

		mapped_file(const och::string filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t mapping_size = 0, uint32_t mapping_offset = 0) noexcept :
			file{ open_file(filename, access_rights, existing_mode, new_mode) },
			mapper{ create_file_mapper(file, (uint64_t)mapping_size + mapping_offset, access_rights) },
			data{ file_as_array(mapper, access_rights, mapping_offset, (uint64_t) mapping_offset + mapping_size) },
			bytes{ mapping_size == 0 ? (uint32_t) get_filesize(file) : mapping_size }
		{}

		mapped_file(const iohandle file, uint32_t access_rights, uint32_t mapping_size = 0, uint32_t mapping_offset = 0) noexcept :
			file{ file },
			mapper{ create_file_mapper(file, (uint64_t)mapping_size + mapping_offset, access_rights) },
			data{ file_as_array(mapper, access_rights, mapping_offset, (uint64_t)mapping_offset + mapping_size) },
			bytes{ mapping_size == 0 ? (uint32_t)get_filesize(file) : mapping_size }
		{}

		~mapped_file() noexcept
		{
			och::close_file_array(data);
			och::close_file(mapper);
			och::close_file(file);
		}

		och::memrun<char> path(och::memrun<char> buf) const noexcept { return get_filepath(file, buf); }

		[[nodiscard]] bool is_valid() const noexcept { return data; }
	};
}
