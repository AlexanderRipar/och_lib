#include "och_fio.h"

#include "och_utf8.h"

#include <utility>

#if defined(_WIN32)

#include <Windows.h>

#include "och_err.h"

#include <cassert>

namespace och
{
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*////////////////////////////////////////////////Free functions/////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	uint32_t interp_openmode(fio::open existing_mode, fio::open new_mode) noexcept
	{
		switch (existing_mode)
		{
		case fio::open::normal:
		case fio::open::append:
			switch (new_mode)
			{
			case fio::open::normal:		return  OPEN_ALWAYS;
			case fio::open::append:		return  OPEN_ALWAYS;
			case fio::open::truncate:	return  OPEN_ALWAYS;
			case fio::open::fail:		return  OPEN_EXISTING;
			}
			break;

		case fio::open::truncate:
			switch (new_mode)
			{
			case fio::open::normal:		return  CREATE_ALWAYS;
			case fio::open::append:		return  CREATE_ALWAYS;
			case fio::open::truncate:	return  CREATE_ALWAYS;
			case fio::open::fail:		return  TRUNCATE_EXISTING;
			}

		case fio::open::fail:
			switch (new_mode)
			{
			case fio::open::normal:		return  CREATE_NEW;
			case fio::open::append:		return  CREATE_NEW;
			case fio::open::truncate:	return  CREATE_NEW;
			case fio::open::fail:		break;
			}
		}

		assert(false);

		return ~0u;
	}

	uint32_t interp_flags(fio::flag flags)
	{
		if ((flags | fio::all_flags) != fio::all_flags)
			return ~0u;

		uint32_t flagmode = 0;

		if ((flags & fio::flag::temporary) == fio::flag::temporary)
			flagmode |= FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE;

		if ((flags & fio::flag::hidden) == fio::flag::hidden)
			flagmode |= FILE_ATTRIBUTE_HIDDEN;

		if ((flags & fio::flag::async) == fio::flag::async)
			return ~0u; // TODO

		return flagmode;
	}

	uint32_t access_interp_open(fio::access access_rights, fio::open existing_mode) noexcept
	{
		uint32_t access;

		switch (access_rights)
		{
		case fio::access::read:      access = GENERIC_READ; break;
		case fio::access::write:     access = GENERIC_WRITE; break;
		case fio::access::read_write: access = GENERIC_READ | GENERIC_WRITE; break;
		default: return ~0u;
		}

		if (existing_mode == fio::open::append)
			access |= FILE_APPEND_DATA;

		return access;
	}

	uint32_t access_interp_page(fio::access access_rights) noexcept
	{
		switch (access_rights)
		{
		case och::fio::access::read:      return PAGE_READONLY;
		case och::fio::access::write:     return PAGE_READWRITE;
		case och::fio::access::read_write: return PAGE_READWRITE;
		}

		return ~0u;
	}

	uint32_t access_interp_fmap(fio::access access_rights) noexcept
	{
		switch (access_rights)
		{
		case och::fio::access::read:      return FILE_MAP_READ;
		case och::fio::access::write:     return FILE_MAP_WRITE;
		case och::fio::access::read_write: return FILE_MAP_WRITE;
		}

		return ~0u;
	}



	[[nodiscard]] status open_file(iohandle& out_handle, const char* filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode, fio::flag flags) noexcept
	{
		out_handle.invalidate_();

		uint32_t access = access_interp_open(access_rights, existing_mode);

		uint32_t openmode = interp_openmode(existing_mode, new_mode);

		uint32_t fileflags = interp_flags(flags);

		if (access == ~0u || openmode == ~0u || fileflags == ~0u)
			return to_status(error::invalid_argument);

		wchar_t wide_filename[32768]{ '\\', '\\', '?', '\\' };

		if (!MultiByteToWideChar(CP_UTF8, 0, filename, -1, wide_filename + 4, 32768 - 4))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		HANDLE h = CreateFileW(wide_filename, access, static_cast<uint32_t>(share_mode), nullptr, openmode, fileflags, nullptr);

		if (h == INVALID_HANDLE_VALUE)
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		out_handle.set_(h);

		return {};
	}

	[[nodiscard]] status create_file_mapper(file_mapper_handle& out_handle, const iohandle& file, uint64_t size, fio::access page_mode, const char* mapping_name) noexcept
	{
		out_handle.invalidate_();

		LARGE_INTEGER _size;

		_size.QuadPart = size;

		uint32_t access = access_interp_page(page_mode);

		if (access == ~0u)
			return to_status(error::invalid_argument);

		wchar_t path_buf[32768];

		const wchar_t* final_mapping_name = nullptr;

		if (mapping_name)
		{
			if (!MultiByteToWideChar(CP_UTF8, 0, mapping_name, -1, path_buf, 32768))
				return to_status(HRESULT_FROM_WIN32(GetLastError()));

			final_mapping_name = path_buf;
		}

		HANDLE h = CreateFileMappingW(file.get_(), nullptr, access, _size.HighPart, _size.LowPart, final_mapping_name);

		if (!h)
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		out_handle.set_(h);

		return {};
	}

	[[nodiscard]] status file_as_array(file_array_handle& out_handle, const file_mapper_handle& file_mapping, fio::access filemap_mode, uint64_t beg, uint64_t end) noexcept
	{
		out_handle.invalidate_();

		LARGE_INTEGER _beg;

		_beg.QuadPart = beg;

		uint32_t access = access_interp_fmap(filemap_mode);

		if (access == ~0u)
			return to_status(error::invalid_argument);

		void* ptr = MapViewOfFile(file_mapping.get_(), access, _beg.HighPart, _beg.LowPart, static_cast<SIZE_T>(end - beg));

		if(!ptr)
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		MEMORY_BASIC_INFORMATION mem_info;

		if (!VirtualQuery(ptr, &mem_info, sizeof(mem_info)))
		{
			status rst = to_status(HRESULT_FROM_WIN32(GetLastError()));

			UnmapViewOfFile(ptr);

			return rst;
		}

		out_handle.set_(ptr, mem_info.RegionSize);

		return {};
	}

	[[nodiscard]] status close_file(iohandle& file) noexcept
	{
		if (file.get_() && !CloseHandle(file.get_()))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		file.invalidate_();

		return {};
	}

	[[nodiscard]] status close_file_mapper(file_mapper_handle& mapper) noexcept
	{
		if (mapper.get_() && !CloseHandle(mapper.get_()))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		mapper.invalidate_();

		return {};
	}

	[[nodiscard]] status close_file_array(file_array_handle& file_array) noexcept
	{
		if (file_array.ptr() && !UnmapViewOfFile(file_array.ptr()))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		file_array.invalidate_();

		return {};
	}

	[[nodiscard]] status delete_file(const char* filename) noexcept
	{
		wchar_t path_buf[32768];

		if (!MultiByteToWideChar(CP_UTF8, 0, filename, -1, path_buf, 32768))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		if (!DeleteFileW(path_buf))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		return {};
	}

	[[nodiscard]] status delete_file(const iohandle& file) noexcept
	{
		FILE_DISPOSITION_INFO disp_info{ TRUE };

		if (!SetFileInformationByHandle(file.get_(), FileDispositionInfo, &disp_info, sizeof(disp_info)))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		return {};
	}

	[[nodiscard]] status read_from_file(och::range<uint8_t>& out_read, const iohandle& file, och::range<uint8_t> buf) noexcept
	{
		out_read = och::range<uint8_t>(nullptr, nullptr);

		uint32_t bytes_read = 0;

		if (!ReadFile(file.get_(), buf.beg, static_cast<DWORD>(buf.len()), reinterpret_cast<LPDWORD>(&bytes_read), nullptr))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		out_read = och::range<uint8_t>(buf.beg, bytes_read);

		return {};
	}

	[[nodiscard]] status write_to_file(uint32_t& out_written, const iohandle& file, const och::range<const uint8_t> buf) noexcept
	{
		out_written = 0;

		uint32_t bytes_written = 0;

		if (!WriteFile(file.get_(), reinterpret_cast<const void*>(buf.beg), static_cast<DWORD>(buf.len()), reinterpret_cast<LPDWORD>(&bytes_written), nullptr))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		out_written = bytes_written;

		return {};
	}

	[[nodiscard]] status file_seek(const iohandle& file, int64_t set_to, fio::setptr setptr_mode) noexcept
	{
		if (static_cast<uint32_t>(setptr_mode) > 2)
			return to_status(error::invalid_argument);

		LARGE_INTEGER _set_to;

		_set_to.QuadPart = set_to;

		if (!SetFilePointerEx(file.get_(), _set_to, nullptr, static_cast<uint32_t>(setptr_mode)))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		return {};
	}

	[[nodiscard]] status get_filesize(uint64_t& out_size, const iohandle& file) noexcept
	{
		out_size = 0ull;

		LARGE_INTEGER filesize;

		if (!GetFileSizeEx(file.get_(), &filesize))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		out_size = filesize.QuadPart;

		return {};
	}

	[[nodiscard]] status set_filesize(const iohandle& file, uint64_t bytes) noexcept
	{
		FILE_END_OF_FILE_INFO eof_info;

		eof_info.EndOfFile.QuadPart = bytes;

		if (!SetFileInformationByHandle(file.get_(), FileEndOfFileInfo, &eof_info, sizeof(eof_info)))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		return {};
	}

	[[nodiscard]] status get_filepath(och::range<char>& out_path, const iohandle& file, och::range<char> buf) noexcept
	{
		out_path = och::range<char>(nullptr, nullptr);

		if (!file)
			return to_status(error::null_argument);

		if (buf.len() == 0)
			return to_status(error::insufficient_buffer);

		wchar_t path_buf[32768];

		DWORD utf16_cus = GetFinalPathNameByHandleW(file.get_(), path_buf, 32768, 0);

		if (utf16_cus == 0 || utf16_cus > 32767)
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		DWORD utf8_cus = WideCharToMultiByte(CP_UTF8, 0, path_buf, -1, buf.beg, static_cast<int>(buf.len()), nullptr, nullptr);

		if (utf8_cus == 0)
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		out_path = och::range<char>(buf.beg, utf8_cus + 1);

		return {};
	}

	[[nodiscard]] status get_modification_time(och::time& out_time, const iohandle& file) noexcept
	{
		out_time = och::time(0ull);

		FILE_BASIC_INFO info;

		if (!GetFileInformationByHandleEx(file.get_(), FileBasicInfo, &info, sizeof(info)))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		out_time = och::time(static_cast<uint64_t>(info.LastWriteTime.QuadPart));

		return {};
	}

	[[nodiscard]] status get_creation_time(och::time& out_time, const iohandle& file) noexcept
	{
		out_time = och::time(0ull);

		FILE_BASIC_INFO info;

		if (!GetFileInformationByHandleEx(file.get_(), FileBasicInfo, &info, sizeof(info)))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		out_time = och::time(static_cast<uint64_t>(info.CreationTime.QuadPart));

		return {};
	}

	[[nodiscard]] status create_tempfile(iohandle& out_handle) noexcept
	{
		out_handle.invalidate_();

		char filename[MAX_PATH + 1];

		if (!GetTempFileNameA(".", "och", 0, filename))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		check(open_file(out_handle, filename, fio::access::read_write, fio::open::normal, fio::open::fail, fio::share::none, fio::flag::temporary));

		return {};
	}

	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////file_search///////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	void append_wildcard_to_search_path(wchar_t* buf, uint32_t cus) noexcept
	{
		if (buf[cus - 2] == '*' && (buf[cus - 3] == '\\' || buf[cus - 3] == '/'))
			return;

		if (buf[cus - 2] == '\\')
		{
			buf[cus - 1] = '*';

			buf[cus] = '\0';

			return;
		}

		buf[cus - 1] = '\\';

		buf[cus] = '*';

		buf[cus + 1] = '\0';
	}

	bool is_valid(WIN32_FIND_DATA* find_data, fio::search mode, const wchar_t ext_filters[file_search::MAX_EXTENSION_FILTER_CNT][file_search::MAX_EXTENSION_FILTER_CUNITS]) noexcept
	{
		if ((find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && mode == fio::search::files)
			return false;

		if (!(find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && mode == fio::search::directories)
			return false;

		if (find_data->cFileName[0] == L'.' && (find_data->cFileName[1] == L'\0' || (find_data->cFileName[1] == L'.' && find_data->cFileName[2] == L'\0')))
			return false;

		if (!ext_filters[0][0])
			return true;

		wchar_t* dot = find_data->cFileName;

		while (*dot != L'.')
			if (*dot++ == L'\0')
				return false;

		++dot;

		wchar_t* end = dot;

		while (*end)
			++end;

		for (int i = 0; i != file_search::MAX_EXTENSION_FILTER_CNT && ext_filters[i * file_search::MAX_EXTENSION_FILTER_CUNITS]; ++i)
			for (int j = 0; j != file_search::MAX_EXTENSION_FILTER_CUNITS; ++j)
			{
				if (dot[j] == L'\0')
					return ext_filters[i][j] == L'\0';

				if(ext_filters[i][j] != dot[j])
					break;

				if (j == file_search::MAX_EXTENSION_FILTER_CUNITS - 1)
					if (dot[j + 1] == L'\0')
						return true;
			}

		return false;
	}

	[[nodiscard]] DWORD advance_until_valid(HANDLE search_handle, WIN32_FIND_DATAW* find_data, fio::search mode, const wchar_t ext_filter[file_search::MAX_EXTENSION_FILTER_CNT][file_search::MAX_EXTENSION_FILTER_CUNITS]) noexcept
	{
		while (!is_valid(find_data, mode, ext_filter))
			if (!FindNextFileW(search_handle, find_data))
				return GetLastError();

		return S_OK;
	}

	[[nodiscard]] status file_search::create(const char* directory, fio::search search_mode, const char* ext_filters) noexcept
	{
		m_search_handle.invalidate_();

		m_search_mode = search_mode;

		wchar_t path_buf[32768];


		for (int i = 0; i != MAX_EXTENSION_FILTER_CNT; ++i)
			for (int j = 0; j != MAX_EXTENSION_FILTER_CUNITS; ++j)
				m_ext_filters[i][j] = L'\0';

		if (ext_filters)
		{
			DWORD ext_cus = MultiByteToWideChar(CP_UTF8, 0, ext_filters, -1, path_buf, 32768);

			if (ext_cus == 0)
				return to_status(HRESULT_FROM_WIN32(GetLastError()));

			const wchar_t* curr = path_buf;

			for (int i = 0; i != MAX_EXTENSION_FILTER_CNT; ++i)
			{
				if (*curr == L'.')
					++curr;

				const wchar_t* prev = curr;

				while (*curr != L'.' && *curr != L'\0')
				{
					m_ext_filters[i][curr - prev] = *curr;

					++curr;
				}

				if (*curr == L'\0')
					break;
			}

			if (*curr != L'\0')
				return to_status(error::too_large);
		}

		m_path = directory;

		size_t dir_len = m_path.get_codeunits();

		if(m_path.back() != utf8_char('\\'))
			m_path += '\\';

		int32_t buf_offset;

		if (dir_len > 3 && (directory[dir_len - 1] == '\\' || directory[dir_len - 1] == '/') && directory[dir_len - 2] == ':')
		{
			buf_offset = 0;
		}
		else
		{
			path_buf[0] = path_buf[1] = path_buf[3] = L'\\';

			path_buf[2] = L'?';

			buf_offset = 4;
		}

		DWORD path_cus = MultiByteToWideChar(CP_UTF8, 0, directory, -1, path_buf + buf_offset, 32768 - buf_offset);
			
		if(path_cus == 0)
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		path_cus += buf_offset;

		if (path_cus > 32766)
			return to_status(error::insufficient_buffer);

		append_wildcard_to_search_path(path_buf, path_cus);

		HANDLE h = FindFirstFileW(path_buf, reinterpret_cast<LPWIN32_FIND_DATAW>(m_internal_buf));

		if (h == INVALID_HANDLE_VALUE)
		{
			DWORD err = GetLastError();

			if (err == ERROR_FILE_NOT_FOUND)
				return error::unavailable;

			return to_status(HRESULT_FROM_WIN32(err));
		}

		m_search_handle.set_(h);

		DWORD err = advance_until_valid(m_search_handle.get_(), reinterpret_cast<WIN32_FIND_DATAW*>(m_internal_buf), m_search_mode, m_ext_filters);

		if (err == ERROR_NO_MORE_FILES)
		{
			m_search_mode = static_cast<fio::search>((1 << 31) | static_cast<uint32_t>(m_search_mode));

			return {};
		}

		if (err)
			return to_status(HRESULT_FROM_WIN32(err));
		
		return {};
	}

	[[nodiscard]] status file_search::close() noexcept
	{
		if (m_search_handle.get_() && !FindClose(m_search_handle.get_()))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		m_search_handle.invalidate_();

		return {};
	}

	[[nodiscard]] status file_search::advance() noexcept
	{
		if (!FindNextFileW(m_search_handle.get_(), reinterpret_cast<WIN32_FIND_DATAW*>(m_internal_buf)))
		{
			DWORD err = GetLastError();

			if (err == ERROR_NO_MORE_FILES)
			{
				m_search_mode = static_cast<fio::search>((1 << 31) | static_cast<uint32_t>(m_search_mode));

				return {};
			}
			
			return to_status(HRESULT_FROM_WIN32(err));
		}

		DWORD err = advance_until_valid(m_search_handle.get_(), reinterpret_cast<WIN32_FIND_DATAW*>(m_internal_buf), m_search_mode, m_ext_filters);

		if (err == S_OK)
			return {};

		if (err == ERROR_NO_MORE_FILES)
		{
			m_search_mode = static_cast<fio::search>((1 << 31) | static_cast<uint32_t>(m_search_mode));

			return {};
		}

		return to_status(HRESULT_FROM_WIN32(err));
	}

	[[nodiscard]] bool file_search::has_more() const noexcept
	{
		return !(static_cast<uint32_t>(m_search_mode) >> 31);
	}

	[[nodiscard]] utf8_string file_search::curr_name() const noexcept
	{
		const WIN32_FIND_DATAW* find_data = reinterpret_cast<const WIN32_FIND_DATAW*>(m_internal_buf);

		char name_buf[260 * 4];

		if (!WideCharToMultiByte(CP_UTF8, 0, find_data->cFileName, -1, name_buf, 260 * 4, nullptr, nullptr))
			return utf8_string();

		return utf8_string(name_buf);
	}

	[[nodiscard]] utf8_string file_search::curr_path() const noexcept
	{
		och::utf8_string path = m_path;

		path += curr_name();

		return std::move(path);
	}

	[[nodiscard]] bool file_search::curr_is_directory() const noexcept
	{
		const WIN32_FIND_DATAW* find_data = reinterpret_cast<const WIN32_FIND_DATAW*>(m_internal_buf);

		return find_data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	}

	[[nodiscard]] och::time file_search::curr_creation_time() const noexcept
	{
		const WIN32_FIND_DATAW* find_data = reinterpret_cast<const WIN32_FIND_DATAW*>(m_internal_buf);

		return och::time((static_cast<uint64_t>(find_data->ftCreationTime.dwHighDateTime) << 32) | find_data->ftCreationTime.dwLowDateTime);
	}

	[[nodiscard]] och::time file_search::curr_modification_time() const noexcept
	{
		const WIN32_FIND_DATAW* find_data = reinterpret_cast<const WIN32_FIND_DATAW*>(m_internal_buf);

		return och::time((static_cast<uint64_t>(find_data->ftLastWriteTime.dwHighDateTime) << 32) | find_data->ftLastWriteTime.dwLowDateTime);
	}
	
	[[nodiscard]] uint64_t file_search::curr_size() const noexcept
	{
		const WIN32_FIND_DATAW* find_data = reinterpret_cast<const WIN32_FIND_DATAW*>(m_internal_buf);

		return (static_cast<uint64_t>(find_data->nFileSizeHigh) << 32) | find_data->nFileSizeLow;
	}

	file_search::~file_search() noexcept
	{
		ignore_status(close());
	}




	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////Standard I/O interop//////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	iohandle get_stdout() noexcept
	{
		return iohandle(GetStdHandle(STD_OUTPUT_HANDLE));
	}

	iohandle get_stdin() noexcept
	{
		return iohandle(GetStdHandle(STD_INPUT_HANDLE));
	}

	iohandle get_stderr() noexcept
	{
		return iohandle(GetStdHandle(STD_ERROR_HANDLE));
	}
}

#elif defined(__linux__)

#include <unistd.h>

namespace och
{
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*////////////////////////////////////////////////Free functions/////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	int32_t interp_openmode(fio::open existing_mode, fio::open new_mode) noexcept
	{
		switch (existing_mode)
		{
		case fio::open::normal:
			switch (new_mode)
			{
			case fio::open::normal:        return O_CREAT;
			case fio::open::append:        return O_CREAT;
			case fio::open::truncate:      return O_CREAT;
			case fio::open::fail:          return 0;
			}
			break;

		case fio::open::append:
			switch (new_mode)
			{
			case fio::open::normal:        return O_CREAT | O_APPEND;
			case fio::open::append:        return O_CREAT | O_APPEND;
			case fio::open::truncate:      return O_CREAT | O_APPEND;
			case fio::open::fail:          return O_APPEND;
			}
			break;

		case fio::open::truncate:
			switch (new_mode)
			{
			case fio::open::normal:        return O_CREAT | O_TRUNC;
			case fio::open::append:        return O_CREAT | O_TRUNC;
			case fio::open::truncate:      return O_CREAT | O_TRUNC;
			case fio::open::fail:          return O_TRUNC;
			}
			break;

		case fio::open::fail:
			switch (new_mode)
			{
			case fio::open::normal:       return O_CREAT | O_EXCL;
			case fio::open::append:       return O_CREAT | O_EXCL;
			case fio::open::truncate:     return O_CREAT | O_EXCL;
			case fio::open::fail:         break;
			}
			break;
		}

		return -1;
	}

	int32_t access_interp_open(fio::access access_rights) noexcept
	{
		switch (access_rights)
		{
		case och::fio::access::read:       return O_RDONLY;
		case och::fio::access::write:      return O_WRONLY;
		case och::fio::access::read_write: return O_RDWR;
		}

		return -1;
	}

	int32_t access_interp_mmap(fio::access access_rights) noexcept
	{
		switch (access_rights)
		{
		case och::fio::access::read:       return PROT_READ;
		case och::fio::access::write:      return PROT_WRITE;
		case och::fio::access::read_write: return PROT_READ | PROT_WRITE;
		}

		return -1;
	}

	[[nodiscard]] status open_file(iohandle& out_handle, const char* filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode, fio::flag flags) noexcept
	{
		int32_t access = access_interp_open(access_rights);

		int32_t openmode = interp_openmode(existing_mode, new_mode);

		int32_t fileflags = access | openmode;

		if (fileflags == -1)
			return to_status(error::invalid_argument);

		if ((flags | fio::all_flags) != fio::all_flags)
			return to_status(error::invalid_argument);

		if ((flags & fio::flag::temporary) == fio::flag::temporary)
			fileflags |= O_TMPFILE;

		const char* final_filename = filename;

		char name_buf[4096];

		if ((flags & fio::flag::hidden) == fio::flag::hidden && filename[0] != '.')
		{
			name_buf[0] = '.';

			size_t i = 1;

			for (; i != 4095 && filename[i]; ++i)
				name_buf[i] = filename[i - 1];

			if (filename[i])
				return to_status(error::too_large);

			name_buf[i] = '\0';

			final_filename = name_buf;
		}

		if ((flags & fio::flag::async) == fio::flag::async)
			return to_status(error::unavailable);

		int32_t fd = iohandle(open(final_filename, fileflags, S_IRUSR | S_IWUSR));

		if (fd == -1)
			return to_status(errno);

		out_handle.set_(fd);

		return {};
	}

	[[nodiscard]] status create_file_mapper(file_mapper_handle& out_handle, const iohandle& file, uint64_t size, fio::access access_rights, const char* mapping_name = nullptr) noexcept
	{
		out_handle.set_(file.get_());

		return {};
	}

	[[nodiscard]] status file_as_array(file_array_handle& out_handle, const file_mapper_handle& file_mapper, fio::access access_rights, uint64_t beg, uint64_t end) noexcept
	{
		out_handle.invalidate();

		int32_t access = access_interp_mmap(access_rights);

		if (access == -1)
			return to_status(error::invalid_argument);

		uint64_t len = end - beg;

		if (len == 0)
		{
			struct stat fs;

			if (fstat(file_mapper.get_(), &fs))
				return to_status(errno);

			len = fs.st_size;
		}

		void* ptr = mmap(nullptr, len, access, 0, file_mapper.get_(), beg);

		if (ptr == MAP_FAILED)
			return to_status(errno);

		out_handle.set_(ptr, len);

		return {};
	}


	[[nodiscard]] status close_file(iohandle& file) noexcept
	{
		if (close(file.get_()))
			return to_status(errno);

		return {};
	}

	[[nodiscard]] status close_file_mapper(file_mapper_handle& handle) noexcept
	{
		return {};
	}

	[[nodiscard]] status close_file_array(file_array_handle& file_array) noexcept
	{
		if (munmap(file_array.ptr(), file_array.bytes()))
			return to_status(errno);

		return {};
	}

	[[nodiscard]] status delete_file(const char* filename) noexcept
	{
		if (unlink(filename))
			return to_status(errno);

		return {};
	}

	[[nodiscard]] status delete_file(const iohandle& file) noexcept
	{
		char path_buf[1024];

		och::range buf(path_buf);

		och::range path;

		check(get_filepath(path, file, buf));

		if (unlink(path))
			return to_status(errno);

		return {};
	}

	[[nodiscard]] status read_from_file(och::range<uint8_t>& out_read, const iohandle& file, och::range<uint8_t> buf) noexcept
	{
		out_read = och::range<uint8_t>(nullptr, nullptr);

		int64_t bytes = read(file.get_(), buf.beg, buf.len());

		if (bytes == -1ll)
			return to_status(errno);

		out_read = och::range<uint8_t>(buf.beg, bytes);

		return {};
	}

	[[nodiscard]] status write_to_file(uint32_t& out_written, const iohandle& file, const och::range<const uint8_t> buf) noexcept
	{
		int64_t bytes = write(file.get_(), buf.beg, buf.len());

		if (bytes == -1ll)
			return to_status(errno);

		out_written = bytes;
	}

	[[nodiscard]] status file_seek(const iohandle& file, int64_t set_to, fio::setptr setptr_mode) noexcept
	{
		int whence;

		switch (setptr_mode)
		{
		case fio::setptr::beg: whence = SEEK_SET; break;
		case fio::setptr::cur: whence = SEEK_CUR; break;
		case fio::setptr::end: whence = SEEK_END; break;
		default: return to_status(error::invalid_argument);
		}

		if (lseek(file.get_(), set_to, whence) == -1)
			return to_status(errno);

		return {};
	}

	[[nodiscard]] status get_filesize(uint64_t& out_size, const iohandle& file) noexcept
	{
		out_size = 0ull;

		struct stat fs;

		if (fstat(file.get_(), &fs))
			return to_status(errno);

		out_size = fs.st_size;

		return {};
	}

	[[nodiscard]] status set_filesize(const iohandle& file, uint64_t bytes) noexcept
	{
		if (ftruncate(file.get_(), bytes))
			return to_status(errno);

		return {};
	}

	[[nodiscard]] status get_filepath(och::range<char>& out_path, const iohandle& file, och::range<char> buf) noexcept
	{
		out_path = och::range<char>(nullptr, nullptr);

		if (!file)
			return to_status(error::null_argument);

		if (buf.len() == 0)
			return to_status(error::insufficient_buffer);

		char buf[64];

		int curr = 0;

		path_buf[curr++] = '/';
		path_buf[curr++] = 'p';
		path_buf[curr++] = 'r';
		path_buf[curr++] = 'o';
		path_buf[curr++] = 'c';
		path_buf[curr++] = '/';
		path_buf[curr++] = 's';
		path_buf[curr++] = 'e';
		path_buf[curr++] = 'l';
		path_buf[curr++] = 'f';
		path_buf[curr++] = '/';
		path_buf[curr++] = 'f';
		path_buf[curr++] = 'd';
		path_buf[curr++] = '/';

		int32_t fd = file.get_();

		while (fd >= 10)
		{
			path_buf[curr++] = '0' + static_cast<char>(fd % 10);

			fd /= 10;
		}

		path_buf[curr++] = '0' + static_cast<char>(fd);

		path_buf[curr] = '\0';

		int64_t bytes = readlink(path_buf, buf.beg, buf.len());

		if (bytes == -1)
		{
			buf[buf.len() - 1] = '\0';

			return to_status(errno);
		}
		else if (bytes < buf.len())
		{
			buf[bytes] = '\0';
		}
		else
		{
			buf[buf.len() - 1] = '\0';
		}

		out_path = och::range<char>(buf.beg, bytes);

		return {};
	}

	[[nodiscard]] status get_modification_time(och::time& out_time, const iohandle& file) noexcept
	{
		out_time = och::time(0ull);

		struct stat fs;

		if (fstat(file.get_(), &fs))
			return to_status(errno);

		out_time = och::time((static_cast<uint64_t>(fs.st_mtime) + 11'644'473'600ll) * 10'000'000ll);

		return {};
	}

	[[nodiscard]] status get_creation_time(och::time& out_time, const iohandle& file) noexcept
	{
		out_time = och::time(0ull);

		return to_status(error::unavailable);
	}

	[[nodiscard]] status create_tempfile(iohandle& out_handle) noexcept
	{
		out_handle.invalidate();

		char name_buf[]{ 'o', 'c', 'h', 'X', 'X', 'X', 'X', 'X', 'X', '\0' };

		int fd = mkstemp(name_buf);

		if (fd == -1)
			return to_status(errno);

		out_handle.set_(fd);

		if (unlink(name_buf))
			return to_status(errno);

		return {};
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////Standard I/O interop//////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	iohandle get_stdout() noexcept
	{
		return iohandle(STDOUT_FILENO);
	}

	iohandle get_stdin() noexcept
	{
		return iohandle(STDIN_FILENO);
	}

	iohandle get_stderr() noexcept
	{
		return iohandle(STDERR_FILENO);
	}
}

#endif // OS-Selection
