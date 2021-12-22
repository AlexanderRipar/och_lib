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
			read_write = 1 | 2,
		};

		enum class share : uint32_t
		{
			none = 0,
			read = 1,
			write = 2,
			remove = 4,
			read_write = 1 | 2,
			read_remove= 1 | 4,
			write_remove = 2 | 4,
			read_write_remove = 1 | 2 | 4,
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
			normal = 0,
			temporary = 1,
			async = 2,
			hidden = 4,
			// readonly = 1,
			// hidden = 2,
			// directory = 16,
			// normal = 128,
			// temporary = 256,
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

		constexpr flag all_flags = fio::flag::normal | fio::flag::temporary | fio::flag::async | fio::flag::hidden;
	}

#if defined(_WIN32)
	using iohandle_rep = void*;
	constexpr iohandle_rep invalid_iohandle = nullptr;
	constexpr void* invalid_file_array_ptr = nullptr;
#elif defined(__linux__)
	using iohandle_rep = int;
	constexpr iohandle_rep invalid_iohandle = -1;
	constexpr void* invalid_file_array_ptr = nullptr;
#endif

	struct iohandle
	{
	private:

		iohandle_rep m_val;

	public:

		iohandle() = default;

		iohandle(const iohandle&) = delete;

		iohandle(iohandle&&) = delete;

		explicit iohandle(iohandle_rep h) noexcept : m_val{ h } {};

		operator bool() const noexcept
		{
			return m_val != invalid_iohandle;
		}

		iohandle_rep get_() const noexcept
		{
			return m_val;
		}

		void set_(iohandle_rep val) noexcept
		{
			m_val = val;
		}

		void invalidate_() noexcept
		{
			set_(invalid_iohandle);
		}
	};

	struct file_mapper_handle
	{
	private:

		iohandle_rep m_val;

	public:

		file_mapper_handle() = default;

		file_mapper_handle(const file_mapper_handle&) = delete;

		file_mapper_handle(file_mapper_handle&&) = delete;

		explicit file_mapper_handle(iohandle_rep h) noexcept : m_val{ h } {};

		operator bool() const noexcept
		{
			return m_val != invalid_iohandle;
		}

		iohandle_rep get_() const noexcept
		{
			return m_val;
		}

		void set_(iohandle_rep val) noexcept
		{
			m_val = val;
		}

		void invalidate_() noexcept
		{
			set_(invalid_iohandle);
		}
	};

	struct file_array_handle
	{
	private:

		void* m_ptr;

		uint64_t m_bytes;

	public:

		file_array_handle() = default;

		file_array_handle(const file_array_handle&) = delete;

		file_array_handle(file_array_handle&&) = delete;

		explicit file_array_handle(void* ptr, uint64_t bytes) noexcept : m_ptr{ ptr }, m_bytes{ bytes } {};

		operator bool() const noexcept
		{
			return m_ptr != invalid_file_array_ptr;
		}

		void* ptr() const noexcept
		{
			return m_ptr;
		}

		uint64_t bytes() const noexcept
		{
			return m_bytes;
		}

		void set_(void* ptr, uint64_t bytes) noexcept
		{
			m_ptr = ptr;

			m_bytes = bytes;
		}

		void invalidate_() noexcept
		{
			set_(nullptr, 0ull);
		}
	};



	[[nodiscard]] status open_file(iohandle& out_handle, const char* filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode = fio::share::none, fio::flag flags = fio::flag::normal) noexcept;

	[[nodiscard]] status create_file_mapper(file_mapper_handle& out_handle, const iohandle& file, uint64_t size, fio::access access_rights, const char* mapping_name = nullptr) noexcept;

	[[nodiscard]] status file_as_array(file_array_handle& out_handle, const file_mapper_handle& file_mapper, fio::access access_rights, uint64_t beg, uint64_t end) noexcept;

	[[nodiscard]] status close_file(iohandle& file) noexcept;

	[[nodiscard]] status close_file_mapper(file_mapper_handle& mapper) noexcept;

	[[nodiscard]] status close_file_array(file_array_handle& file_array) noexcept;

	[[nodiscard]] status delete_file(const char* filename) noexcept;

	[[nodiscard]] status delete_file(const iohandle& file) noexcept;

	[[nodiscard]] status read_from_file(och::range<uint8_t>& out_read, const iohandle& file, och::range<uint8_t> buf) noexcept;

	[[nodiscard]] status write_to_file(uint32_t& out_written, const iohandle& file, const och::range<const uint8_t> buf) noexcept;

	[[nodiscard]] status file_seek(const iohandle& file, int64_t set_to, fio::setptr setptr_mode) noexcept;

	[[nodiscard]] status get_filesize(uint64_t& out_size, const iohandle& file) noexcept;

	[[nodiscard]] status set_filesize(const iohandle& file, uint64_t bytes) noexcept;

	[[nodiscard]] status get_filepath(och::range<char>& out_path, const iohandle& file, och::range<char> buf) noexcept;

	[[nodiscard]] status get_modification_time(och::time& out_time, const iohandle& file) noexcept;

	[[nodiscard]] status get_creation_time(och::time& out_time, const iohandle& file) noexcept;

	[[nodiscard]] status create_tempfile(iohandle& out_handle) noexcept;



	struct filehandle
	{
	private:

		iohandle m_file;

	public:

		filehandle() noexcept = default;

		filehandle(const filehandle&) = delete;

		filehandle(filehandle&&) = delete;

		[[nodiscard]] status create(const char* filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode) noexcept
		{
			check(open_file(m_file, filename, access_rights, existing_mode, new_mode, share_mode));

			return {};
		}

		[[nodiscard]] status create(const och::stringview& filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode) noexcept
		{
			check(create(filename.raw_cbegin(), access_rights, existing_mode, new_mode, share_mode));

			return {};
		}

		[[nodiscard]] status create(const och::string& filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode) noexcept
		{
			check(create(filename.raw_cbegin(), access_rights, existing_mode, new_mode, share_mode));

			return {};
		}

		[[nodiscard]] status create_temp() noexcept
		{
			check(create_tempfile(m_file));

			return {};
		}

		[[nodiscard]] status close() noexcept
		{
			check(close_file(m_file));

			return {};
		}

		template<typename T>
		[[nodiscard]] status read(och::range<T>& out_read, och::range<T> buf) const noexcept
		{
			och::range<char> ret;

			check(read_from_file(ret, m_file, och::range<char>(reinterpret_cast<char*>(buf.beg), buf.bytes())));

			out_read = och::range<T>(reinterpret_cast<T*>(ret.beg), ret.bytes() / sizeof(T));

			return {};
		}

		template<typename T>
		[[nodiscard]] status write(uint32_t& out_written, const och::range<T> buf) const noexcept
		{
			check(write_to_file(out_written, m_file, och::range<const char>(reinterpret_cast<const char*>(buf.beg), buf.bytes())));

			return {};
		}

		[[nodiscard]] status get_size(uint64_t& out_size) const noexcept
		{
			check(get_filesize(out_size, m_file));

			return {};
		}

		[[nodiscard]] status set_size(uint64_t bytes) const noexcept
		{
			check(set_filesize(m_file, bytes));

			return {};
		}

		[[nodiscard]] status path(och::range<char>& out_path, och::range<char> buf) const noexcept
		{
			check(get_filepath(out_path, m_file, buf));

			return {};
		}

		[[nodiscard]] status seek(int64_t set_to, fio::setptr setptr_mode) const noexcept
		{
			check(file_seek(m_file, set_to, setptr_mode));

			return {};
		}

		[[nodiscard]] status modification_time(och::time& out_time) const noexcept
		{
			check(get_modification_time(out_time, m_file));

			return {};
		}

		[[nodiscard]] status creation_time(och::time& out_time) const noexcept
		{
			check(get_creation_time(out_time, m_file));

			return {};
		}

		~filehandle() noexcept
		{
			ignore_status(close());
		}

		[[nodiscard]] iohandle get_handle_() const noexcept
		{
			return iohandle(m_file.get_());
		}
	};



	template<typename T = uint8_t>
	struct mapped_file
	{
	private:

		iohandle m_file = iohandle(nullptr);
		file_mapper_handle m_mapper = iohandle(nullptr);
		file_array_handle m_data = file_array_handle(nullptr);

	public:

		status create(const char* filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, uint64_t mapping_size = 0, uint64_t mapping_offset = 0, fio::share share_mode = fio::share::none) noexcept
		{
			check(open_file(m_file, filename, access_rights, existing_mode, new_mode, share_mode));

			check(create_file_mapper(m_mapper, m_file, mapping_size + mapping_offset, access_rights));

			check(file_as_array(m_data, m_mapper, access_rights, mapping_offset, mapping_offset + mapping_size));

			return {};
		}
		
		status create(const och::stringview& filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, uint64_t mapping_size = 0, uint64_t mapping_offset = 0, fio::share share_mode = fio::share::none) noexcept
		{
			check(create(filename.raw_cbegin(), access_rights, existing_mode, new_mode, mapping_size, mapping_offset, share_mode));

			return {};
		}
		
		status create(const och::string& filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, uint64_t mapping_size = 0, uint64_t mapping_offset = 0, fio::share share_mode = fio::share::none) noexcept
		{
			check(create(filename.raw_cbegin(), access_rights, existing_mode, new_mode, mapping_size, mapping_offset, share_mode));

			return {};
		}

		void close() noexcept
		{
			close_file_array(m_data);
			
			close_file_mapper(m_mapper);

			close_file(m_file);
		}

		[[nodiscard]] T* data() const noexcept
		{
			return static_cast<T*>(m_data.ptr);
		}

		[[nodiscard]] T& operator[](uint64_t idx) noexcept
		{
			return static_cast<T*>(m_data.ptr)[idx];
		}

		[[nodiscard]] const T& operator[](uint64_t idx) const noexcept
		{
			return static_cast<T*>(m_data.ptr)[idx];
		}

		[[nodiscard]] range<T> range() const noexcept
		{
			return range<T>(static_cast<T*>(m_data.ptr()), m_data.bytes() / sizeof(T));
		}

		[[nodiscard]] status path(och::range<char>& out_path, och::range<char> buf) const noexcept
		{
			check(get_filepath(out_path, m_file, buf));

			return {};
		}

		[[nodiscard]] uint64_t size() const noexcept
		{
			return m_data.bytes() / sizeof(T);
		}

		[[nodiscard]] uint64_t bytes() const noexcept
		{
			return m_data.bytes();
		}

		~mapped_file() noexcept
		{
			close();
		}
	};



	struct file_search
	{
		static constexpr size_t MAX_EXTENSION_FILTER_CNT = 4;

		static constexpr size_t MAX_EXTENSION_FILTER_CUNITS = 4;

	private:

		iohandle m_search_handle;

		fio::search m_search_mode;

		utf8_string m_path;

		wchar_t m_ext_filters[MAX_EXTENSION_FILTER_CNT][MAX_EXTENSION_FILTER_CUNITS];

		uint64_t m_internal_buf[74];

	public:

		file_search() noexcept = default;

		file_search(const file_search&) = delete;

		file_search(file_search&&) = delete;

		[[nodiscard]] status create(const char* directory, fio::search search_mode, const char* ext_filter) noexcept;

		[[nodiscard]] status close() noexcept;

		[[nodiscard]] status advance() noexcept;

		[[nodiscard]] bool has_more() const noexcept;

		[[nodiscard]] utf8_string curr_name() const noexcept;

		[[nodiscard]] utf8_string curr_path() const noexcept;

		[[nodiscard]] bool curr_is_directory() const noexcept;

		[[nodiscard]] och::time curr_creation_time() const noexcept;

		[[nodiscard]] och::time curr_modification_time() const noexcept;

		[[nodiscard]] uint64_t curr_size() const noexcept;

		~file_search() noexcept;
	};

	

	[[nodiscard]] iohandle get_stdout() noexcept;

	[[nodiscard]] iohandle get_stdin() noexcept;

	[[nodiscard]] iohandle get_stderr() noexcept;
}

#endif // !OCH_FIO_INCLUDE_GUARD
