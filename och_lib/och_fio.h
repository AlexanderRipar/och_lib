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

			search_for_all = 3,
			search_for_files = 1,
			search_for_directories = 2,
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

		template<typename T>
		[[nodiscard]] och::range<T> read(och::range<T> buf) const noexcept
		{
			och::range<char> ret = read_from_file(handle, och::range<char>(reinterpret_cast<char*>(buf.beg), buf.bytes()));

			return och::range<T>(reinterpret_cast<T*>(ret.beg), ret.bytes() / sizeof(T));
		}

		template<typename T>
		uint32_t write(const och::range<T> buf) const noexcept
		{
			return write_to_file(handle, och::range<const char>(reinterpret_cast<const char*>(buf.beg), buf.bytes()));
		}

		[[nodiscard]] uint64_t get_size() const noexcept;

		bool set_size(uint64_t bytes) const noexcept;

		[[nodiscard]] och::range<char> path(och::range<char> buf) const noexcept;

		bool seek(int64_t set_to, uint32_t setptr_mode = fio::setptr_beg) const noexcept;

		[[nodiscard]] och::time last_write_time() const noexcept;

		void close() const noexcept;

		[[nodiscard]] operator bool() const noexcept;

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

		const iohandle m_file;
		const iohandle m_mapper;
		const iohandle m_data;

		const uint32_t m_bytes;

	public:

		mapped_file(const char* filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t mapping_size = 0, uint32_t mapping_offset = 0, uint32_t share_mode = och::fio::share_none) noexcept :
			m_file{ open_file(filename, access_rights, existing_mode, new_mode, share_mode) },
			m_mapper{ create_file_mapper(m_file, (uint64_t)mapping_size + mapping_offset, access_rights) },
			m_data{ file_as_array(m_mapper, access_rights, mapping_offset, (uint64_t)mapping_offset + mapping_size) },
			m_bytes{ m_data.ptr ? mapping_size == 0 ? (uint32_t)get_filesize(m_file) : mapping_size : 0 }
		{}

		mapped_file(const och::utf8_string& filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t mapping_size = 0, uint32_t mapping_offset = 0) noexcept :
			mapped_file(filename.raw_cbegin(), access_rights, existing_mode, new_mode, mapping_size, mapping_offset) {}

		mapped_file(const och::stringview& filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t mapping_size = 0, uint32_t mapping_offset = 0) noexcept :
			mapped_file(filename.raw_cbegin(), access_rights, existing_mode, new_mode, mapping_size, mapping_offset) {}

		~mapped_file() noexcept
		{
			och::close_file_array(m_data);
			och::close_file(m_mapper);
			och::close_file(m_file);
		}

		[[nodiscard]] uint32_t bytes() const noexcept { return m_bytes; }

		[[nodiscard]] T* data() const noexcept { return static_cast<T*>(m_data.ptr); }

		[[nodiscard]] T& operator[](uint32_t idx) noexcept { return static_cast<T*>(m_data.ptr)[idx]; }

		[[nodiscard]] const T& operator[](uint32_t idx) const noexcept { return static_cast<T*>(m_data.ptr)[idx]; }

		[[nodiscard]] range<T> range() const noexcept { return range<T>(static_cast<T*>(m_data.ptr), m_bytes / sizeof(T)); }

		[[nodiscard]] och::range<char> path(och::range<char> buf) const noexcept { return get_filepath(m_file, buf); }

		[[nodiscard]] uint32_t size() const noexcept { return m_bytes / sizeof(T); }

		operator bool() const noexcept { return m_data.ptr; }
	};

	struct file_search
	{
		struct file_info_data
		{
			uint32_t flags_and_padding alignas(8);
			uint32_t attributes;
			och::time creation_time;
			och::time last_access_time;
			och::time last_write_time;
			uint64_t size;
			uint64_t _reserved;
			char name[260];//MAX_PATH
			char alt_name[14];
		};

		struct file_info
		{
		private:

			const file_search& m_data;

		public:

			file_info(const file_search& data) noexcept;

			och::stringview name() const noexcept;

			och::time creation_time() const noexcept;

			och::time last_access_time() const noexcept;

			och::time last_modification_time() const noexcept;

			uint64_t size() const noexcept;

			bool is_directory() const noexcept;

			och::stringview ending() const noexcept;

			och::utf8_string absolute_name() const noexcept;
		};

		struct file_iterator
		{
		private:

			file_search* m_search;

		public:

			file_iterator(file_search* search) noexcept;

			void operator++() noexcept;

			bool operator!=(const file_iterator& rhs) const noexcept;

			file_info operator*() const noexcept;
		};

	private:

		iohandle search_handle;

		file_info_data m_info_data;

		char m_ending_filters[8][8];

		char m_search_path[260];

	public:

		file_search(const char* path, uint32_t search_mode = fio::search_for_all, const char* ending_filters = nullptr) noexcept;

		file_search(const och::utf8_string& path, uint32_t search_mode = fio::search_for_all, const char* ending_filters = nullptr) noexcept;

		file_search(const och::stringview& path, uint32_t search_mode = fio::search_for_all, const char* ending_filters = nullptr) noexcept;
		
		file_search(const file_search& rhs) = delete;

		~file_search() noexcept;

		void advance() noexcept;

		bool has_next() const noexcept;

		file_info get_info() const noexcept;

		file_iterator begin() noexcept;

		file_iterator end() noexcept;

		operator bool() const noexcept;

	private:

		bool matches_ending_filter(const char* ending) const noexcept;
	};

	

	[[nodiscard]] iohandle get_stdout() noexcept;
	[[nodiscard]] iohandle get_stdin() noexcept;
	[[nodiscard]] iohandle get_stderr() noexcept;

	const iohandle standard_out = get_stdout();
	const iohandle standard_in = get_stdin();
	const iohandle standard_err = get_stderr();
}
