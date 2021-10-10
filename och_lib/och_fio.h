#define OCH_FIO_PRESENT

#ifndef OCH_FIO_INCLUDE_GUARD
#define OCH_FIO_INCLUDE_GUARD

#include <cstdint>

#include "och_range.h"
#include "och_time.h"
#include "och_utf8.h"
#include "och_err.h"

namespace och
{
	namespace fio
	{
		enum class open : uint32_t
		{
			normal = 0,
			truncate = 1,
			fail = 2,
			append = 4,
		};

		enum class access : uint32_t
		{
			read = 2,
			write = 1,
			readwrite = 1 | 2,
		};

		enum class share : uint32_t
		{
			none = 0,
			read = 1,
			write = 2,
			erase = 4,
			read_write = 1 | 2,
			read_delete = 1 | 4,
			write_delete = 2 | 4,
			read_write_delete = 1 | 2 | 4,
		};

		enum class setptr : uint32_t
		{
			beg = 0,
			cur = 1,
			end = 2,
		};

		enum class search
		{
			files = 1,
			directories = 2,
			all = 1 | 2,
		};

		enum class flag
		{
			readonly = 1,
			hidden = 2,
			directory = 16,
			normal = 128,
			temporary = 256,
		};

		constexpr flag operator|(flag l, flag r) noexcept
		{
			return static_cast<flag>(static_cast<uint32_t>(l) | static_cast<uint32_t>(r));
		}

		constexpr flag operator&(flag l, flag r) noexcept
		{
			return static_cast<flag>(static_cast<uint32_t>(l) & static_cast<uint32_t>(r));
		}

		constexpr flag operator^(flag l, flag r) noexcept
		{
			return static_cast<flag>(static_cast<uint32_t>(l) ^ static_cast<uint32_t>(r));
		}
	}

	struct iohandle
	{
		iohandle() = default;

		explicit iohandle(void* h) noexcept : ptr{ h } {};

		void* ptr;
	};

	struct file_array_handle
	{
		file_array_handle() = default;

		explicit file_array_handle(void* h) noexcept : ptr{ h } {}

		void* ptr;
	};



	[[nodiscard]] status open_file(iohandle& out_handle, const char* filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode = fio::share::none, fio::flag flags = fio::flag::normal) noexcept;

	[[nodiscard]] status create_file_mapper(iohandle& out_handle, const iohandle file, uint64_t size, fio::access access_rights, const char* mapping_name = nullptr) noexcept;

	[[nodiscard]] status file_as_array(file_array_handle& out_handle, const iohandle file_mapper, fio::access access_rights, uint64_t beg, uint64_t end) noexcept;

	void close_file(const iohandle file) noexcept;

	void close_file_array(const file_array_handle file_array) noexcept;

	[[nodiscard]] status delete_file(const char* filename) noexcept;

	[[nodiscard]] status read_from_file(och::range<char>& out_read, const iohandle file, och::range<char> buf) noexcept;

	[[nodiscard]] status write_to_file(uint32_t& out_written, const iohandle file, const och::range<const char> buf) noexcept;

	[[nodiscard]] status file_seek(const iohandle file, int64_t set_to, fio::setptr setptr_mode) noexcept;

	[[nodiscard]] status get_filesize(uint64_t& out_size, const iohandle file) noexcept;

	[[nodiscard]] status set_filesize(const iohandle file, uint64_t bytes) noexcept;

	[[nodiscard]] status get_filepath(och::range<char>& out_path, const iohandle file, och::range<char> buf) noexcept;

	[[nodiscard]] status get_last_write_time(och::time& out_time, const iohandle file) noexcept;

	[[nodiscard]] status create_tempfile(iohandle& out_handle, fio::share share_mode = och::fio::share::none) noexcept;



	struct filehandle
	{
		iohandle handle;

		[[nodiscard]] status create(const char* filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode = fio::share::none) noexcept;

		[[nodiscard]] status create(const och::stringview& filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode = fio::share::none) noexcept;

		[[nodiscard]] status create(const och::string& filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode = fio::share::none) noexcept;

		[[nodiscard]] status create_temp(fio::share share_mode = fio::share::none) noexcept;

		void close() noexcept;

		template<typename T>
		[[nodiscard]] status read(och::range<T>& out_read, och::range<T> buf) const noexcept
		{
			och::range<char> ret;
			
			check(read_from_file(ret, handle, och::range<char>(reinterpret_cast<char*>(buf.beg), buf.bytes())));

			out_read = och::range<T>(reinterpret_cast<T*>(ret.beg), ret.bytes() / sizeof(T));

			return {};
		}

		template<typename T>
		[[nodiscard]] status write(uint32_t& out_written, const och::range<T> buf) const noexcept
		{
			check(write_to_file(out_written, handle, och::range<const char>(reinterpret_cast<const char*>(buf.beg), buf.bytes())));

			return {};
		}

		[[nodiscard]] status get_size(uint64_t& out_size) const noexcept;

		[[nodiscard]] status set_size(uint64_t bytes) const noexcept;

		[[nodiscard]] status path(och::range<char>& out_path, och::range<char> buf) const noexcept;

		[[nodiscard]] status seek(int64_t set_to, fio::setptr setptr_mode = fio::setptr::beg) const noexcept;

		[[nodiscard]] status last_write_time(och::time& out_time) const noexcept;

		~filehandle() noexcept;
	};



	template<typename T = uint8_t>
	struct mapped_file
	{
	private:

		iohandle m_file = iohandle(nullptr);
		iohandle m_mapper = iohandle(nullptr);
		file_array_handle m_data = file_array_handle(nullptr);

		uint64_t m_bytes = 0;

	public:

		status create(const char* filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, uint32_t mapping_size = 0, uint32_t mapping_offset = 0, fio::share share_mode = fio::share::none) noexcept
		{
			check(open_file(m_file, filename, access_rights, existing_mode, new_mode, share_mode));

			check(create_file_mapper(m_mapper, m_file, (uint64_t)mapping_size + mapping_offset, access_rights));

			check(file_as_array(m_data, m_mapper, access_rights, mapping_offset, (uint64_t)mapping_offset + mapping_size));

			if (mapping_size)
				m_bytes = mapping_size;
			else
				check(get_filesize(m_bytes, m_file));

			return {};
		}
		
		status create(const och::stringview& filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, uint32_t mapping_size = 0, uint32_t mapping_offset = 0, fio::share share_mode = fio::share::none) noexcept
		{
			check(create(filename.raw_cbegin(), access_rights, existing_mode, new_mode, mapping_size, mapping_offset, share_mode));

			return {};
		}
		
		status create(const och::string& filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, uint32_t mapping_size = 0, uint32_t mapping_offset = 0, fio::share share_mode = fio::share::none) noexcept
		{
			check(create(filename.raw_cbegin(), access_rights, existing_mode, new_mode, mapping_size, mapping_offset, share_mode));

			return {};
		}

		void close() noexcept
		{
			close_file_array(m_data);
			
			close_file(m_mapper);

			close_file(m_file);

			m_data = file_array_handle(nullptr);

			m_mapper = iohandle(nullptr);

			m_file = iohandle(nullptr);
		}

		[[nodiscard]] uint32_t bytes() const noexcept
		{
			return static_cast<uint32_t>(m_bytes);
		}

		[[nodiscard]] T* data() const noexcept
		{
			return static_cast<T*>(m_data.ptr);
		}

		[[nodiscard]] T& operator[](uint32_t idx) noexcept
		{
			return static_cast<T*>(m_data.ptr)[idx];
		}

		[[nodiscard]] const T& operator[](uint32_t idx) const noexcept
		{
			return static_cast<T*>(m_data.ptr)[idx];
		}

		[[nodiscard]] range<T> range() const noexcept
		{
			return range<T>(static_cast<T*>(m_data.ptr), m_bytes / sizeof(T));
		}

		[[nodiscard]] status path(och::range<char>& out_path, och::range<char> buf) const noexcept
		{
			check(get_filepath(out_path, m_file, buf));

			return {};
		}

		[[nodiscard]] uint32_t size() const noexcept
		{
			return m_bytes / sizeof(T);
		}

		~mapped_file() noexcept
		{
			close();
		}
	};



	struct file_search
	{
		struct file_info_data
		{
			uint32_t flags_and_padding alignas(8);
			fio::flag attributes;
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

			och::string name() const noexcept;

			och::time creation_time() const noexcept;

			och::time last_access_time() const noexcept;

			och::time last_modification_time() const noexcept;

			uint64_t size() const noexcept;

			bool is_directory() const noexcept;

			och::string ending() const noexcept;

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

		status create(const char* path, fio::search search_mode = fio::search::all, const char* ending_filters = nullptr) noexcept;

		status create(const och::utf8_string& path, fio::search search_mode = fio::search::all, const char* ending_filters = nullptr) noexcept;

		status create(const och::stringview& path, fio::search search_mode = fio::search::all, const char* ending_filters = nullptr) noexcept;
		
		void close() noexcept;

		file_search(const file_search& rhs) = delete;

		status advance() noexcept;

		bool has_next() const noexcept;

		file_info get_info() const noexcept;

		file_iterator begin() noexcept;

		file_iterator end() noexcept;

		~file_search() noexcept;

	private:

		uint32_t single_advance() noexcept;

		bool matches_ending_filter(const char* ending) const noexcept;
	};

	

	[[nodiscard]] iohandle get_stdout() noexcept;
	[[nodiscard]] iohandle get_stdin() noexcept;
	[[nodiscard]] iohandle get_stderr() noexcept;

	const iohandle standard_out = get_stdout();
	const iohandle standard_in = get_stdin();
	const iohandle standard_err = get_stderr();
}

#endif // !OCH_FIO_INCLUDE_GUARD
