#pragma once

#include <cstdint>

#include "och_range.h"
#include "och_time.h"
#include "och_utf8.h"

namespace och
{
	namespace fio
	{
		enum _fio : uint32_t
		{
			open_normal = 0,
			open_truncate = 1,
			open_fail = 2,
			open_append = 4,

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

			flag_normal = 128,
			flag_directory = 16,
			flag_hidden = 2,
			flag_readonly = 1,
			flag_temporary = 256,
		};
	}

	struct iohandle
	{
		iohandle() = default;

		explicit iohandle(void* h) noexcept;

		void* ptr;
	};

	[[nodiscard]] iohandle open_file(const char* filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode = fio::share_none, uint32_t flags = fio::flag_normal) noexcept;

	[[nodiscard]] iohandle create_file_mapper(const iohandle file, uint64_t size, uint32_t access_rights, const char* mapping_name = nullptr) noexcept;

	[[nodiscard]] iohandle file_as_array(const iohandle file_mapper, uint32_t access_rights, uint64_t beg, uint64_t end) noexcept;

	[[nodiscard]] int64_t get_filesize(const iohandle file) noexcept;

	[[nodiscard]] och::range<char> read_from_file(const iohandle file, och::range<char> buf) noexcept;

	uint32_t write_to_file(const iohandle file, const och::range<const char> buf) noexcept;

	bool close_file(const iohandle file) noexcept;

	bool close_file_array(const iohandle file_array) noexcept;

	bool delete_file(const char* filename) noexcept;

	bool file_seek(const iohandle file, int64_t set_to, uint32_t setptr_mode) noexcept;

	bool set_filesize(const iohandle file, uint64_t bytes) noexcept;

	[[nodiscard]] och::range<char> get_filepath(const iohandle file, och::range<char> buf) noexcept;

	[[nodiscard]] och::time get_last_write_time(const iohandle file) noexcept;

	struct filehandle
	{
		const iohandle handle;

		filehandle(const och::stringview& filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode = fio::share_none) noexcept;

		filehandle(const char* filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode = fio::share_none) noexcept;

		filehandle(const och::utf8_string& filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode = fio::share_none) noexcept;

		~filehandle() noexcept;

		filehandle(const filehandle&) = delete;

		[[nodiscard]] och::range<char> read(och::range<char> buf) const noexcept;

		uint32_t write(const och::range<const char> buf) const noexcept;

		uint32_t write(const och::range<char> buf) const noexcept;

		[[nodiscard]] uint64_t get_size() const noexcept;

		bool set_size(uint64_t bytes) const noexcept;

		[[nodiscard]] och::range<char> path(och::range<char> buf) const noexcept;

		bool seek(int64_t set_to, uint32_t setptr_mode = fio::setptr_beg) const noexcept;

		[[nodiscard]] och::time last_write_time() const noexcept;

		void close() const noexcept;

		[[nodiscard]] bool operator!() const noexcept;

	protected:

		filehandle(iohandle handle);
	};

	struct tempfilehandle : public filehandle
	{
		tempfilehandle(uint32_t share_mode = 0) noexcept;

		~tempfilehandle() noexcept;
	};

	template<typename T = uint8_t>
	struct mapped_file
	{
	private:

		iohandle file;
		iohandle mapper;
		iohandle data;

	public:

		const uint32_t bytes;

		mapped_file(const char* filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t mapping_size = 0, uint32_t mapping_offset = 0) noexcept :
			file{ open_file(filename, access_rights, existing_mode, new_mode) },
			mapper{ create_file_mapper(file, (uint64_t)mapping_size + mapping_offset, access_rights) },
			data{ file_as_array(mapper, access_rights, mapping_offset, (uint64_t)mapping_offset + mapping_size) },
			bytes{ mapping_size == 0 ? (uint32_t)get_filesize(file) : mapping_size }
		{}

		mapped_file(const och::utf8_string& filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t mapping_size = 0, uint32_t mapping_offset = 0) noexcept :
			mapped_file(filename.raw_cbegin(), access_rights, existing_mode, new_mode, mapping_size, mapping_offset) {}

		mapped_file(const och::stringview& filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t mapping_size = 0, uint32_t mapping_offset = 0) noexcept :
			mapped_file(filename.raw_cbegin(), access_rights, existing_mode, new_mode, mapping_size, mapping_offset) {}

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

		[[nodiscard]] range<T> get_data() const noexcept { return range<T>(reinterpret_cast<T*>(data.ptr), bytes / sizeof(T)); }

		[[nodiscard]] T& operator[](uint32_t idx) { return reinterpret_cast<T*>(data.ptr)[idx]; }

		template<typename U>
		[[nodiscard]] U& get_at(uint32_t idx) { return *reinterpret_cast<U*>(reinterpret_cast<uint8_t*>(data.ptr) + idx); }

		[[nodiscard]] och::range<char> path(och::range<char> buf) const noexcept { return get_filepath(file, buf); }

		[[nodiscard]] bool is_valid() const noexcept { return data.ptr; }

		[[nodiscard]] bool operator!() const noexcept { return !is_valid(); }
	};

	struct file_search
	{
		iohandle search_handle;

		struct
		{
			uint32_t _padding;
			uint32_t attributes;
			och::time creation_time;
			och::time last_access_time;
			och::time last_write_time;
			uint64_t size;
			uint64_t _reserved;
			char name[260];//MAX_PATH
			char alt_name[14];
		}
		curr_data;

		static constexpr int size = sizeof(curr_data);

		file_search(const char* path) noexcept;

		file_search(const och::utf8_string& path) noexcept;

		file_search(const och::stringview& path) noexcept;

		~file_search() noexcept;

		bool next() noexcept;

		och::stringview name() const noexcept;

		bool is_dir() const noexcept;

		filehandle open(uint32_t access_rights, uint32_t share_mode = fio::share_none) const noexcept;
	};

	[[nodiscard]] iohandle get_stdout();
	[[nodiscard]] iohandle get_stdin();
	[[nodiscard]] iohandle get_stderr();

	const iohandle standard_out = get_stdout();
	const iohandle standard_in = get_stdin();
	const iohandle standard_err = get_stderr();
}
