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

	// 32767 - 4 chars for the \\?\ prefix for long paths. 
	// This is still only approximate according to the MSDN documentation at
	// https://docs.microsoft.com/en-us/windows/win32/fileio/maximum-file-path-limitation?tabs=cmd
	//
	// The maximum path of 32,767 characters is approximate, because the "\\?\" prefix may be 
	// expanded to a longer string by the system at run time, and this expansion applies 
	// to the total length.
	constexpr size_t MAX_FILENAME_CHARS = 32763;

	struct file_search_result
	{
	private:

		uint8_t m_win32_find_dataw[592] alignas(8);

	public:

		[[nodiscard]] utf8_string name() const noexcept;

		[[nodiscard]] bool is_directory() const noexcept;

		[[nodiscard]] bool is_file() const noexcept;

		[[nodiscard]] bool is_hidden() const noexcept;

		[[nodiscard]] uint64_t size() const noexcept;

		[[nodiscard]] time creation_time() const noexcept;

		[[nodiscard]] time modification_time() const noexcept;
	};

#elif defined(__linux__)

	using iohandle_rep = int;

	constexpr iohandle_rep invalid_iohandle = -1;

	constexpr size_t MAX_FILENAME_CHARS = 4095;

	struct file_search_result
	{
	private:

		void* m_dirent_ptr;

		uint64_t m_stat[512] alignas(8);

	public:

		[[nodiscard]] utf8_string name() const noexcept;

		[[nodiscard]] bool is_directory() const noexcept;

		[[nodiscard]] bool is_file() const noexcept;

		[[nodiscard]] bool is_hidden() const noexcept;

		[[nodiscard]] uint64_t size() const noexcept;

		[[nodiscard]] time creation_time() const noexcept;

		[[nodiscard]] time modification_time() const noexcept;

		void* get_dirent_ptr_() noexcept
		{
			return m_dirent_ptr;
		}

		void set_dirent_ptr_raw_(void* ptr) noexcept
		{
			m_dirent_ptr = ptr;
		}

		void* get_stat_ptr_raw_() noexcept
		{
			return static_cast<void*>(&m_stat);
		}
	};


#endif

	struct iohandle
	{
	private:

		iohandle_rep m_val;

	public:

		iohandle() : m_val{ invalid_iohandle } {};

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

	struct file_array_handle
	{
	private:

		void* m_ptr;

		uint64_t m_bookkeeping;

	public:

		file_array_handle() : m_ptr{ nullptr }, m_bookkeeping{ 0ull } {}

		explicit file_array_handle(void* ptr, uint64_t bytes) noexcept : m_ptr{ ptr }, m_bookkeeping{ bytes } {};

		operator bool() const noexcept
		{
			return m_ptr != nullptr;
		}

		void* ptr() const noexcept
		{
			return m_ptr;
		}

		void set_(void* ptr, uint64_t bookkeeping) noexcept
		{
			m_ptr = ptr;

			m_bookkeeping = bookkeeping;
		}

		uint64_t bookkeeping_() const noexcept
		{
			return m_bookkeeping;
		}

		void invalidate_() noexcept
		{
			set_(nullptr, 0ull);
		}
	};

	struct file_search_handle
	{
	private:

		void* m_val;

	public:

		file_search_handle() : m_val{ nullptr } {}

		explicit file_search_handle(void* val) noexcept : m_val{ val } {};

		operator bool() const noexcept
		{
			return m_val != nullptr;
		}

		void* get_() const noexcept
		{
			return m_val;
		}

		void set_(void* ptr) noexcept
		{
			m_val = ptr;
		}

		void invalidate_() noexcept
		{
			set_(nullptr);
		}
	};



	[[nodiscard]] status open_file(iohandle& out_handle, const char* filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode = fio::share::none, fio::flag flags = fio::flag::normal) noexcept;

	[[nodiscard]] status file_as_array(file_array_handle& out_handle, const iohandle& file, fio::access access_rights, uint64_t beg, uint64_t end) noexcept;

	[[nodiscard]] status create_file_search(file_search_handle& out_handle, file_search_result& out_result, const char* directory) noexcept;

	[[nodiscard]] status close_file(iohandle& file) noexcept;

	[[nodiscard]] status close_file_array(file_array_handle& file_array) noexcept;

	[[nodiscard]] status close_file_search(file_search_handle& file_search) noexcept;

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

	[[nodiscard]] status advance_file_search(file_search_result& out_result, const file_search_handle& file_search) noexcept;

	[[nodiscard]] status get_current_directory(och::utf8_string& out_directory) noexcept;

	[[nodiscard]] status get_application_directory(och::utf8_string& out_directory) noexcept;

	[[nodiscard]] status set_current_directory(const char* new_directory) noexcept;

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

		iohandle m_file;
		file_array_handle m_data;
		uint64_t m_bytes;

	public:

		status create(const char* filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, uint64_t mapping_offset = 0, uint64_t mapping_size = 0, fio::share share_mode = fio::share::none) noexcept
		{
			check(open_file(m_file, filename, access_rights, existing_mode, new_mode, share_mode));

			check(file_as_array(m_data, m_file, access_rights, mapping_offset, mapping_offset + mapping_size));

			if (mapping_size == 0)
				check(get_filesize(m_bytes, m_file));
			else
				m_bytes = mapping_size;

			return {};
		}
		
		void close() noexcept
		{
			ignore_status(close_file_array(m_data));
			
			ignore_status(close_file(m_file));
		}

		[[nodiscard]] T* data() const noexcept
		{
			return static_cast<T*>(m_data.ptr());
		}

		[[nodiscard]] T& operator[](uint64_t idx) noexcept
		{
			return static_cast<T*>(m_data.ptr())[idx];
		}

		[[nodiscard]] const T& operator[](uint64_t idx) const noexcept
		{
			return static_cast<T*>(m_data.ptr())[idx];
		}

		[[nodiscard]] range<T> range() const noexcept
		{
			return range<T>(static_cast<T*>(m_data.ptr()), m_bytes / sizeof(T));
		}

		[[nodiscard]] status path(och::range<char>& out_path, och::range<char> buf) const noexcept
		{
			check(get_filepath(out_path, m_file, buf));

			return {};
		}

		[[nodiscard]] uint64_t size() const noexcept
		{
			return m_bytes / sizeof(T);
		}

		[[nodiscard]] uint64_t bytes() const noexcept
		{
			return m_bytes;
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

		file_search_handle m_handle;

		file_search_result m_result;

		fio::search m_search_mode;

		utf8_string m_path;

#if defined(__linux__)
		char m_ext_filters[MAX_EXTENSION_FILTER_CNT][MAX_EXTENSION_FILTER_CUNITS];
#elif defined(_WIN32)
		wchar_t m_ext_filters[MAX_EXTENSION_FILTER_CNT][MAX_EXTENSION_FILTER_CUNITS];
#endif

	public:

		file_search() noexcept = default;

		file_search(const file_search&) = delete;
		
		file_search(file_search&&) = delete;

		[[nodiscard]] status create(const char* directory, fio::search search_mode, const char* ext_filters) noexcept;

		[[nodiscard]] status close() noexcept;

		[[nodiscard]] status advance() noexcept;

		[[nodiscard]] bool has_more() const noexcept;

		[[nodiscard]] utf8_string curr_name() const noexcept;

		[[nodiscard]] utf8_string curr_path() const noexcept;

		[[nodiscard]] bool curr_is_directory() const noexcept;

		[[nodiscard]] bool curr_is_file() const noexcept;

		[[nodiscard]] bool curr_is_hidden() const noexcept;

		[[nodiscard]] och::time curr_creation_time() const noexcept;

		[[nodiscard]] och::time curr_modification_time() const noexcept;

		[[nodiscard]] uint64_t curr_size() const noexcept;

		~file_search() noexcept;
	};

	struct recursive_file_search
	{
		static constexpr size_t MAX_EXTENSION_FILTER_CNT = 4;

		static constexpr size_t MAX_EXTENSION_FILTER_CUNITS = 4;

		static constexpr size_t MAX_RECURSION_DEPTH = 32;

	private:

		file_search_result m_result;

		uint32_t m_curr_recursion_level;

		uint32_t m_max_recursion_level;

		fio::search m_search_mode;

		utf8_string m_path;

#if defined(__linux__)
		char m_ext_filters[MAX_EXTENSION_FILTER_CNT][MAX_EXTENSION_FILTER_CUNITS];
#elif defined(_WIN32)
		wchar_t m_ext_filters[MAX_EXTENSION_FILTER_CNT][MAX_EXTENSION_FILTER_CUNITS];
#endif

		file_search_handle m_handle_stack[MAX_RECURSION_DEPTH];

	public:

		recursive_file_search() noexcept = default;

		recursive_file_search(const recursive_file_search&) = delete;

		recursive_file_search(recursive_file_search&&) = delete;

		[[nodiscard]] status create(const char* directory, fio::search search_mode, const char* ext_filters, uint32_t max_recursion_level) noexcept;

		[[nodiscard]] status close() noexcept;

		[[nodiscard]] status advance() noexcept;

		[[nodiscard]] bool has_more() const noexcept;

		[[nodiscard]] utf8_string curr_name() const noexcept;

		[[nodiscard]] utf8_string curr_path() const noexcept;

		[[nodiscard]] bool curr_is_directory() const noexcept;

		[[nodiscard]] bool curr_is_file() const noexcept;

		[[nodiscard]] bool curr_is_hidden() const noexcept;

		[[nodiscard]] och::time curr_creation_time() const noexcept;

		[[nodiscard]] och::time curr_modification_time() const noexcept;

		[[nodiscard]] uint64_t curr_size() const noexcept;

		~recursive_file_search() noexcept;
	};

	

	[[nodiscard]] iohandle get_stdout() noexcept;

	[[nodiscard]] iohandle get_stdin() noexcept;

	[[nodiscard]] iohandle get_stderr() noexcept;
}

#endif // !OCH_FIO_INCLUDE_GUARD
