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
	/*////////////////////////////////////////////////////Helpers////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	using filename_buf = wchar_t[MAX_FILENAME_CHARS];

	[[nodiscard]] static int32_t utf8_str_to_utf16_filename(const char* str, filename_buf buf)
	{
		buf[0] = buf[1] = buf[3] = '\\';

		buf[2] = '?';

		int32_t chars_written = MultiByteToWideChar(CP_UTF8, 0, str, -1, buf + 4, MAX_FILENAME_CHARS - 4);

		return chars_written + ((chars_written != 0) << 2);
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*//////////////////////////////////////////////file_search_result///////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	static const WIN32_FIND_DATAW* get_fsr_data_ptr(const file_search_result* result)
	{
		return reinterpret_cast<const WIN32_FIND_DATAW*>(result);
	}

	static WIN32_FIND_DATAW* get_fsr_data_ptr(file_search_result* result)
	{
		return reinterpret_cast<WIN32_FIND_DATAW*>(result);
	}

	[[nodiscard]] utf8_string file_search_result::name() const noexcept
	{
		const WIN32_FIND_DATAW* data = get_fsr_data_ptr(this);

		char utf8_name_buf[260 * 4];

		int chars_written = WideCharToMultiByte(CP_UTF8, 0, data->cFileName, -1, utf8_name_buf, sizeof(utf8_name_buf), nullptr, nullptr);

		if (chars_written == 0)
			return utf8_string("");

		return utf8_string(utf8_name_buf);
	}

	[[nodiscard]] bool file_search_result::is_directory() const noexcept
	{
		const WIN32_FIND_DATAW* data = get_fsr_data_ptr(this);

		return data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY;
	}

	[[nodiscard]] bool file_search_result::is_file() const noexcept
	{
		const WIN32_FIND_DATAW* data = get_fsr_data_ptr(this);

		return !(data->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	}

	[[nodiscard]] bool file_search_result::is_hidden() const noexcept
	{
		const WIN32_FIND_DATAW* data = get_fsr_data_ptr(this);

		return data->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN;
	}

	[[nodiscard]] uint64_t file_search_result::size() const noexcept
	{
		const WIN32_FIND_DATAW* data = get_fsr_data_ptr(this);

		return data->nFileSizeLow | (static_cast<uint64_t>(data->nFileSizeHigh) << 32);
	}

	[[nodiscard]] time file_search_result::creation_time() const noexcept
	{
		const WIN32_FIND_DATAW* data = get_fsr_data_ptr(this);

		return och::time(data->ftCreationTime.dwLowDateTime | (static_cast<uint64_t>(data->ftCreationTime.dwHighDateTime) << 32));
	}

	[[nodiscard]] time file_search_result::modification_time() const noexcept
	{
		const WIN32_FIND_DATAW* data = get_fsr_data_ptr(this);

		return och::time(data->ftLastWriteTime.dwLowDateTime | (static_cast<uint64_t>(data->ftLastWriteTime.dwHighDateTime) << 32));
	}



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
			return to_status(error::argument_invalid);

		filename_buf wide_filename;

		if (!utf8_str_to_utf16_filename(filename, wide_filename))
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
			return to_status(error::argument_invalid);

		wchar_t path_buf[MAX_FILENAME_CHARS];

		const wchar_t* final_mapping_name = nullptr;

		if (mapping_name)
		{
			if (!MultiByteToWideChar(CP_UTF8, 0, mapping_name, -1, path_buf, MAX_FILENAME_CHARS))
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
			return to_status(error::argument_invalid);

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

	[[nodiscard]] status create_file_search(file_search_handle& out_handle, file_search_result& out_result, const char* directory) noexcept
	{
		out_handle.invalidate_();

		WIN32_FIND_DATAW* rst = get_fsr_data_ptr(&out_result);

		filename_buf wide_directory;

		int32_t dir_chars = utf8_str_to_utf16_filename(directory, wide_directory);
		
		if (!dir_chars)
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		if (dir_chars < 2)
			return to_status(error::argument_invalid);

		if (wide_directory[dir_chars - 2] == '\\' || wide_directory[dir_chars - 2] == '//')
		{
			if (dir_chars + 1 > MAX_FILENAME_CHARS)
				return to_status(error::argument_too_large);

			wide_directory[dir_chars - 1] = L'*';

			wide_directory[dir_chars] = L'\0';
		}
		else
		{
			if (dir_chars + 2 > MAX_FILENAME_CHARS)
				return to_status(error::argument_too_large);

			wide_directory[dir_chars - 1] = L'\\';

			wide_directory[dir_chars] = L'*';

			wide_directory[dir_chars + 1] = L'\0';
		}

		HANDLE h = FindFirstFileW(wide_directory, rst);

		if (h == INVALID_HANDLE_VALUE)
		{
			DWORD err = GetLastError();

			if (err == ERROR_FILE_NOT_FOUND)
				return status(error::no_more_data);
			else
				return to_status(HRESULT_FROM_WIN32(err));
		}

		out_handle.set_(h);

		if (rst->cFileName[0] == L'.' && (rst->cFileName[1] == L'\0' || (rst->cFileName[1] == L'.' && rst->cFileName[2] == L'\0')))
			check(advance_file_search(out_result, out_handle));

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

	[[nodiscard]] status close_file_search(file_search_handle& file_search) noexcept
	{
		if (file_search.get_() && !FindClose(file_search.get_()))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		file_search.invalidate_();

		return {};
	}

	[[nodiscard]] status delete_file(const char* filename) noexcept
	{
		filename_buf wide_filename;

		if (!utf8_str_to_utf16_filename(filename, wide_filename))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		if (!DeleteFileW(wide_filename))
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
			return to_status(error::argument_invalid);

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
			return to_status(error::argument_invalid);

		if (buf.len() == 0)
			return to_status(error::insufficient_buffer);

		wchar_t path_buf[MAX_FILENAME_CHARS];

		DWORD utf16_cus = GetFinalPathNameByHandleW(file.get_(), path_buf, MAX_FILENAME_CHARS, 0);

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

		wchar_t filename[MAX_PATH + 1];

		if (!GetTempFileNameW(L".", L"och", 0, filename))
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		HANDLE h = CreateFileW(filename, GENERIC_READ | GENERIC_WRITE, 0, nullptr, CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);

		if (h == INVALID_HANDLE_VALUE)
			return to_status(HRESULT_FROM_WIN32(GetLastError()));

		out_handle.set_(h);

		return {};
	}

	[[nodiscard]] status advance_file_search(file_search_result& out_result, const file_search_handle& file_search) noexcept
	{
		while (true)
		{
			WIN32_FIND_DATAW* rst = get_fsr_data_ptr(&out_result);

			if (!FindNextFileW(file_search.get_(), rst))
			{
				DWORD err = GetLastError();

				if (err == ERROR_NO_MORE_FILES)
					return status(error::no_more_data);
				else
					return to_status(HRESULT_FROM_WIN32(err));
			}

			if (!(rst->cFileName[0] == L'.' && (rst->cFileName[1] == L'\0' || (rst->cFileName[1] == L'.' && rst->cFileName[2] == L'\0'))))
				break;
		}

		return {};
	}

	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////file_search///////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	static bool extension_matches(const wchar_t* name, const wchar_t ext_filters[file_search::MAX_EXTENSION_FILTER_CNT][file_search::MAX_EXTENSION_FILTER_CUNITS]) noexcept
	{
		const wchar_t* dot = name;

		while (*dot != L'.')
			if (*dot++ == L'\0')
				return false;

		++dot;

		for (int i = 0; i != file_search::MAX_EXTENSION_FILTER_CNT && ext_filters[i * file_search::MAX_EXTENSION_FILTER_CUNITS]; ++i)
			for (int j = 0; j != file_search::MAX_EXTENSION_FILTER_CUNITS; ++j)
			{
				if (dot[j] == L'\0')
					return ext_filters[i][j] == L'\0';

				if (ext_filters[i][j] != dot[j])
					break;

				if (j == file_search::MAX_EXTENSION_FILTER_CUNITS - 1)
					if (dot[j + 1] == L'\0')
						return true;
			}

		return false;
	}

	static bool is_relevant_file(const file_search_result& data, fio::search search_mode, const wchar_t ext_filters[file_search::MAX_EXTENSION_FILTER_CNT][file_search::MAX_EXTENSION_FILTER_CUNITS])
	{
		if (search_mode == fio::search::directories && !data.is_directory())
			return false;

		if (search_mode == fio::search::files && !data.is_file())
			return false;

		if (!ext_filters[0][0])
			return true;

		return !extension_matches(get_fsr_data_ptr(&data)->cFileName, ext_filters);
	}

	[[nodiscard]] status file_search::create(const char* directory, fio::search search_mode, const char* ext_filters) noexcept
	{
		m_search_mode = search_mode;

		for (int i = 0; i != MAX_EXTENSION_FILTER_CNT; ++i)
			for (int j = 0; j != MAX_EXTENSION_FILTER_CUNITS; ++j)
				m_ext_filters[i][j] = L'\0';

		wchar_t path_buf[MAX_FILENAME_CHARS];

		if (ext_filters)
		{
			DWORD ext_cus = MultiByteToWideChar(CP_UTF8, 0, ext_filters, -1, path_buf, MAX_FILENAME_CHARS);

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
					if (curr - prev >= MAX_EXTENSION_FILTER_CUNITS)
						return to_status(error::argument_too_large);

					m_ext_filters[i][curr - prev] = *curr;

					++curr;
				}

				if (*curr == L'\0')
					break;
			}
		}

		m_path = directory;

		uint32_t path_cunits = m_path.get_codeunits();

		char* path_cstr = m_path.raw_begin();

		for (uint32_t i = 0; i != path_cunits; ++i)
			if (path_cstr[i] == '/')
				path_cstr[i] = '\\';

		if(path_cstr[path_cunits - 1] != '\\')
			m_path += '\\';

		if(status rst = create_file_search(m_handle, m_result, directory))
			if (rst == error::no_more_data)
			{
				m_search_mode = static_cast<fio::search>((1 << 31) | static_cast<uint32_t>(m_search_mode));

				return {};
			}
			else
				return to_status(rst);

		while (!is_relevant_file(m_result, m_search_mode, m_ext_filters))
			if (status rst = advance_file_search(m_result, m_handle))
				if (rst == error::no_more_data)
				{
					m_search_mode = static_cast<fio::search>((1 << 31) | static_cast<uint32_t>(m_search_mode));

					return {};
				}
				else
					return to_status(rst);

		return {};
	}

	[[nodiscard]] status file_search::close() noexcept
	{
		check(close_file_search(m_handle));

		return {};
	}

	[[nodiscard]] status file_search::advance() noexcept
	{
		do
		{
			if (status rst = advance_file_search(m_result, m_handle))
				if (rst == error::no_more_data)
				{
					m_search_mode = static_cast<fio::search>((1 << 31) | static_cast<uint32_t>(m_search_mode));

					return {};
				}
				else
					return to_status(rst);
		} 
		while (!is_relevant_file(m_result, m_search_mode, m_ext_filters));

		return {};
	}

	[[nodiscard]] bool file_search::has_more() const noexcept
	{
		return !(static_cast<uint32_t>(m_search_mode) >> 31);
	}

	[[nodiscard]] utf8_string file_search::curr_name() const noexcept
	{
		return m_result.name();
	}

	[[nodiscard]] utf8_string file_search::curr_path() const noexcept
	{
		utf8_string rst = m_path;

		rst += m_result.name();

		return std::move(rst);
	}

	[[nodiscard]] bool file_search::curr_is_directory() const noexcept
	{
		return m_result.is_directory();
	}

	[[nodiscard]] bool file_search::curr_is_file() const noexcept
	{
		return m_result.is_file();
	}

	[[nodiscard]] bool file_search::curr_is_hidden() const noexcept
	{
		return m_result.is_hidden();
	}

	[[nodiscard]] och::time file_search::curr_creation_time() const noexcept
	{
		return m_result.creation_time();
	}

	[[nodiscard]] och::time file_search::curr_modification_time() const noexcept
	{
		return m_result.modification_time();
	}
	
	[[nodiscard]] uint64_t file_search::curr_size() const noexcept
	{
		return m_result.size();
	}

	file_search::~file_search() noexcept
	{
		ignore_status(close());
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////recursive_file_search/////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	static void remove_last_path_elem(utf8_string& path) noexcept
	{
		const char* backslash = path.raw_cend() - 2;

		while (*backslash != '\\')
			--backslash;

		++backslash;

		path.pop(static_cast<uint32_t>(path.raw_cend() - backslash));
	}

	[[nodiscard]] status recursive_file_search::create(const char* directory, fio::search search_mode, const char* ext_filters, uint32_t max_recursion_level) noexcept
	{
		if(max_recursion_level > MAX_RECURSION_DEPTH)
			return to_status(error::argument_too_large);
	
		if (max_recursion_level == 0)
			m_max_recursion_level = MAX_RECURSION_DEPTH + 1;
		else
			m_max_recursion_level = max_recursion_level;

		m_curr_recursion_level = 0;

		m_search_mode = search_mode;

		for (int i = 0; i != MAX_EXTENSION_FILTER_CNT; ++i)
			for (int j = 0; j != MAX_EXTENSION_FILTER_CUNITS; ++j)
				m_ext_filters[i][j] = L'\0';

		wchar_t path_buf[MAX_FILENAME_CHARS];

		if (ext_filters)
		{
			DWORD ext_cus = MultiByteToWideChar(CP_UTF8, 0, ext_filters, -1, path_buf, MAX_FILENAME_CHARS);

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
					if (curr - prev >= MAX_EXTENSION_FILTER_CUNITS)
						return to_status(error::argument_too_large);

					m_ext_filters[i][curr - prev] = *curr;

					++curr;
				}

				if (*curr == L'\0')
					break;
			}
		}

		m_path = directory;

		uint32_t path_cunits = m_path.get_codeunits();

		char* path_cstr = m_path.raw_begin();

		for (uint32_t i = 0; i != path_cunits; ++i)
			if (path_cstr[i] == '/')
				path_cstr[i] = '\\';

		if (path_cstr[path_cunits - 1] != '\\')
			m_path += '\\';

		if (status rst = create_file_search(m_handle_stack[0], m_result, directory))
			if (rst == error::no_more_data)
			{
				m_search_mode = static_cast<fio::search>((1 << 31) | static_cast<uint32_t>(m_search_mode));

				return {};
			}
			else
				return to_status(rst);

		if (!is_relevant_file(m_result, m_search_mode, m_ext_filters))
			check(advance());

		return {};
	}

	[[nodiscard]] status recursive_file_search::close() noexcept
	{
		status first_error;

		for (uint32_t i = 0; i != m_curr_recursion_level + 1; ++i)
		{
			if (!first_error)
				first_error = close_file_search(m_handle_stack[i]);
		}

		if (first_error)
			return to_status(first_error);

		return {};
	}

	[[nodiscard]] status recursive_file_search::advance() noexcept
	{
		const WIN32_FIND_DATAW* debug_helper_data = get_fsr_data_ptr(&m_result);

		do
		{
			if (curr_is_directory() && m_curr_recursion_level + 1 < m_max_recursion_level)
			{
				if (m_curr_recursion_level + 1 >= MAX_RECURSION_DEPTH)
					return to_status(error::insufficient_buffer);

				m_path += curr_name();

				m_path += '\\';

				if (status rst_push = create_file_search(m_handle_stack[++m_curr_recursion_level], m_result, m_path.raw_cbegin()))
				{
					if (rst_push != error::no_more_data && rst_push.errcode() != HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED))
						return to_status(rst_push);

					check(close_file_search(m_handle_stack[m_curr_recursion_level--]));

					remove_last_path_elem(m_path);

					while (status rst_next = advance_file_search(m_result, m_handle_stack[m_curr_recursion_level]))
					{
						if (rst_next != error::no_more_data)
							return to_status(rst_next);

						if (!m_curr_recursion_level)
						{
							m_search_mode = static_cast<fio::search>((1 << 31) | static_cast<uint32_t>(m_search_mode));

							return {};
						}

						check(close_file_search(m_handle_stack[m_curr_recursion_level--]));

						remove_last_path_elem(m_path);
					}
				}
			}
			else
			{
				while (status rst_next = advance_file_search(m_result, m_handle_stack[m_curr_recursion_level]))
				{
					if (rst_next != error::no_more_data)
						return to_status(rst_next);

					if (!m_curr_recursion_level)
					{
						m_search_mode = static_cast<fio::search>((1 << 31) | static_cast<uint32_t>(m_search_mode));

						return {};
					}

					check(close_file_search(m_handle_stack[m_curr_recursion_level--]));

					remove_last_path_elem(m_path);
				}
			}
		}
		while (!is_relevant_file(m_result, m_search_mode, m_ext_filters));

		return {};
	}

	[[nodiscard]] bool recursive_file_search::has_more() const noexcept
	{
		return !(static_cast<uint32_t>(m_search_mode) >> 31);
	}

	[[nodiscard]] utf8_string recursive_file_search::curr_name() const noexcept
	{
		return m_result.name();
	}

	[[nodiscard]] utf8_string recursive_file_search::curr_path() const noexcept
	{
		utf8_string rst = m_path;

		rst += m_result.name();

		return std::move(rst);
	}

	[[nodiscard]] bool recursive_file_search::curr_is_directory() const noexcept
	{
		return m_result.is_directory();
	}

	[[nodiscard]] bool recursive_file_search::curr_is_file() const noexcept
	{
		return m_result.is_file();
	}

	[[nodiscard]] bool recursive_file_search::curr_is_hidden() const noexcept
	{
		return m_result.is_hidden();
	}

	[[nodiscard]] och::time recursive_file_search::curr_creation_time() const noexcept
	{
		return m_result.creation_time();
	}

	[[nodiscard]] och::time recursive_file_search::curr_modification_time() const noexcept
	{
		return m_result.modification_time();
	}

	[[nodiscard]] uint64_t recursive_file_search::curr_size() const noexcept
	{
		return m_result.size();
	}

	recursive_file_search::~recursive_file_search() noexcept
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
#include <dirent.h>

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
			return to_status(error::argument_invalid);

		if ((flags | fio::all_flags) != fio::all_flags)
			return to_status(error::argument_invalid);

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
				return to_status(error::argument_too_large);

			name_buf[i] = '\0';

			final_filename = name_buf;
		}

		if ((flags & fio::flag::async) == fio::flag::async)
			return to_status(error::function_unavailable);

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
			return to_status(error::argument_invalid);

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

	[[nodiscard]] status create_file_search(file_search_handle& out_handle, file_search_result& out_result, const char* directory) noexcept
	{
		out_handle.invalidate_();

		DIR* dir = opendir(directory);

		if (!dir)
			return to_status(errno);

		out_handle.set_(dir);

		check(advance_file_search(out_handle, out_result));

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

	[[nodiscard]] status close_file_search(file_search_handle& file_search) noexcept
	{
		if (closedir(static_cast<DIR*>(file_search.get_())))
			return to_status(errno);

		file_search.invalidate_();

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
		default: return to_status(error::argument_invalid);
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
			return to_status(error::argument_invalid);

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

		uint64_t time_ns100 = (static_cast<uint64_t>(fs.st_mtime) + 11'644'473'600ll) * 10'000'000ll;

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700
		time_ns100 += fs.st_mtim.tv_nsec / 100;
#else
		time_ns100 += fs.mtimensec / 100;
#endif // defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700

		out_time = och::time(time_ns100);

		return {};
	}

	[[nodiscard]] status get_creation_time(och::time& out_time, const iohandle& file) noexcept
	{
		out_time = och::time(0ull);

		return to_status(error::function_unavailable);
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

	[[nodiscard]] status advance_file_search(file_search_result& out_result, const file_search_handle& file_search) noexcept
	{
		errno = 0;

		dirent* result = readdir(file_search.get_());

		if (!result)
			if (errno == 0)
				return status(error::no_more_data);
			else
				return to_status(errno);

		struct stat* st_ptr = static_cast<struct stat*>(out_result.get_stat_ptr_());

		if (stat(result.d_name, st_ptr) == -1)
			return to_status(errno);

		out_result.set_(result);

		return {};
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////file_search///////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	bool is_valid_file_linux(dirent* find_data, const struct stat* fs, fio::search search_mode, const char ext_filter[file_search::MAX_EXTENSION_FILTER_CNT][file_search::MAX_EXTENSION_FILTER_CUNITS]) noexcept
	{
		if (S_ISDIR(*fs) && search_mode == fio::search::files)
			return false;

		if (!S_ISDIR(*fs) && search_mode == fio::search::directories)
			return false;

		if (find_data->d_name[0] == '.' && (find_data->d_name[1] == '\0' || (find_data->d_name[1] == '.' && find_data->d_name[2] == '\0')))
			return false;

		if (!ext_filters[0][0])
			return true;

		const char* dot = find_data->d_name;

		while (*dot != '.')
			if (*dot++ == '\0')
				return false;

		++dot;

		for (int i = 0; i != file_search::MAX_EXTENSION_FILTER_CNT && ext_filters[i * file_search::MAX_EXTENSION_FILTER_CUNITS]; ++i)
			for (int j = 0; j != file_search::MAX_EXTENSION_FILTER_CUNITS; ++j)
			{
				if (dot[j] == '\0')
					return ext_filters[i][j] == '\0';

				if (ext_filters[i][j] != dot[j])
					break;

				if (j == file_search::MAX_EXTENSION_FILTER_CUNITS - 1)
					if (dot[j + 1] == '\0')
						return true;
			}

		return false;
	}

	[[nodiscard]] status file_search::create(const char* directory, fio::search search_mode, const char* ext_filters) noexcept
	{
		m_search_handle = nullptr;

		DIR* search_ptr = opendir(directory);

		if (!search_ptr)
			return to_status(errno);

		m_search_handle = static_cast<void*>(search_ptr);

		for (int i = 0; i != MAX_EXTENSION_FILTER_CNT; ++i)
			for (int j = 0; j != MAX_EXTENSION_FILTER_CUNITS; ++j)
				m_ext_filters_utf8[i][j] = '\0';

		if (ext_filters)
		{
			const char* curr = ext_filters;

			for (int i = 0; i != MAX_EXTENSION_FILTER_CNT; ++i)
			{
				if (*curr == '.')
					++curr;

				const char* prev = curr;

				while (*curr != '.' && *curr != '\0')
				{
					if(curr - prev >= MAX_EXTENSION_FILTER_CUNITS)
						return to_status(error::argument_too_large)

					m_ext_filters_utf8[i][curr - prev] = *curr;

					++curr;
				}

				if (*curr == '\0')
					break;
			}
		}

		m_path = directory;

		uint32_t path_cunits = m_path.get_codeunits();

		char* path_cstr = path.raw_begin();

		for (uint32_t i = 0; i != path_cunits; ++i)
			if (path_cstr[i] == '\\')
				path_cstr[i] = '/';

		if (m_path.back() != utf8_char('/'))
			m_path += '/';

		check(advance());

		return {};
	}

	[[nodiscard]] status file_search::close() noexcept
	{
		if (closedir(static_cast<DIR*>(m_search_handle)))
			return to_status(errno);

		m_search_handle = nullptr;

		return {};
	}

	[[nodiscard]] status file_search::advance() noexcept
	{
		errno = 0;

		struct stat* fs_ptr = reinterpret_cast<struct stat*>(m_internal_buf + 1);

		char full_path_buf[4096];

		const uint32_t path_cunits = m_path.get_codeunits();

		if (path_cunits >= 4095)
			return to_status(error::insufficient_buffer);

		const char* path_data = m_path.raw_cbegin();

		for (uint32_t i = 0; i != path_cunits; ++i)
			full_path_buf[i] = path_data[i];

		do
		{
			m_internal_buf_ptr = static_cast<void*>(readdir(m_search_ptr));

			if (!m_internal_buf_ptr)
				break;

			uint32_t name_idx = 0;

			for (; static_cast<dirent*>(m_internal_buf_ptr)->d_name[name_idx] && path_cunits + name_idx != 4095; ++name_idx)
				full_path_buf[path_cunits + name_idx] = static_cast<dirent*>(m_internal_buf_ptr)->d_name[name_idx];

			if (path_cunits + name_idx == 4095 && static_cast<dirent*>(m_internal_buf_ptr)->d_name[name_idx])
				return to_status(error::insufficient_buffer);

			full_path_buf[path_cunits + name_idx] = '\0';

			if (lstat(m_path.raw_cbegin(), fs_ptr))
				return to_status(errno);
		}
		while(!is_valid_file_linux(reinterpret_cast<dirent*>(m_internal_buf_ptr), fs_ptr, m_search_mode, m_ext_filters_utf8);

		if (!m_internal_buf_ptr)
		{
			m_search_mode = static_cast<fio::search>((1 << 31) | static_cast<uint32_t>(m_search_mode));

			if (errno)
				return to_status(errno);
		}

		return {};
	}

	[[nodiscard]] bool file_search::has_more() const noexcept
	{
		return !(static_cast<uint32_t>(m_search_mode) >> 31);
	}

	[[nodiscard]] utf8_string file_search::curr_name() const noexcept
	{
		return m_result.name();
	}

	[[nodiscard]] utf8_string file_search::curr_path() const noexcept
	{
		utf8_string path = m_path;

		path += m_result.name();

		return std::move(path);
	}

	[[nodiscard]] bool file_search::curr_is_directory() const noexcept
	{
		const struct stat* fs = reinterpret_cast<const struct stat*>(m_internal_buffer + 1);

		return S_ISDIR(*fs);
	}

	[[nodiscard]] och::time file_search::curr_creation_time() const noexcept
	{
		return och::time(0ull);
	}

	[[nodiscard]] och::time file_search::curr_modification_time() const noexcept
	{
		const struct stat* fs = reinterpret_cast<const struct stat*>(m_internal_buffer + 1);

		uint64_t time_ns100 = (static_cast<uint64_t>(fs.st_mtime) + 11'644'473'600ll) * 10'000'000ll;

#if defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700
		time_ns100 += fs.st_mtim.tv_nsec / 100;
#else
		time_ns100 += fs.mtimensec / 100;
#endif // defined(_BSD_SOURCE) || defined(_SVID_SOURCE) || _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 700

		return  och::time(time_ns100);
	}

	[[nodiscard]] uint64_t file_search::curr_size() const noexcept
	{
		const struct stat* fs = reinterpret_cast<const struct stat*>(m_internal_buffer + 1);

		return fs->st_size;
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
