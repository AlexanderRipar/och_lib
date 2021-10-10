#include "och_fio.h"

#include "och_utf8.h"

#include <utility>

#ifdef _WIN32

#include <Windows.h>
#include "och_err.h"

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
			case fio::open::normal:		return  4;
			case fio::open::append:		return  4;
			case fio::open::truncate:	return  4;
			case fio::open::fail:		return  3;
			}
			break;

		case fio::open::truncate:
			switch (new_mode)
			{
			case fio::open::normal:		return  2;
			case fio::open::append:		return  2;
			case fio::open::truncate:	return  2;
			case fio::open::fail:		return  5;
			}

		case fio::open::fail:
			switch (new_mode)
			{
			case fio::open::normal:		return  1;
			case fio::open::append:		return  1;
			case fio::open::truncate:	return  1;
			case fio::open::fail:		return ~0u;
			}
		}

		return ~0u;
	}

	uint32_t access_interp_open(fio::access access_rights) noexcept
	{
		return static_cast<uint32_t>(access_rights) << 30;
	}

	uint32_t access_interp_page(fio::access access_rights) noexcept
	{
		return ((static_cast<uint32_t>(access_rights) ^ 3) + ((static_cast<uint32_t>(access_rights) == 3) << 1)) << 1;
	}

	uint32_t access_interp_fmap(fio::access access_rights) noexcept
	{
		return (static_cast<uint32_t>(access_rights) - ((static_cast<uint32_t>(access_rights) == 3) << 1)) << 1;
	}



	[[nodiscard]] status open_file(iohandle& out_handle, const char* filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode, fio::flag flags) noexcept
	{
		out_handle = iohandle(CreateFileA(filename, access_interp_open(access_rights), static_cast<uint32_t>(share_mode), nullptr, interp_openmode(existing_mode, new_mode), static_cast<uint32_t>(flags), nullptr));

		if (out_handle.ptr == INVALID_HANDLE_VALUE)
		{
			out_handle = iohandle(nullptr);

			return make_status(HRESULT_FROM_WIN32(GetLastError()));
		}

		if (existing_mode == fio::open::append)
			check(file_seek(out_handle, 0, fio::setptr::end));

		return {};
	}

	[[nodiscard]] status create_file_mapper(iohandle& out_handle, const iohandle file, uint64_t size, fio::access page_mode, const char* mapping_name) noexcept
	{
		LARGE_INTEGER _size;

		_size.QuadPart = size;

		out_handle = iohandle(CreateFileMappingA(file.ptr, nullptr, access_interp_page(page_mode), _size.HighPart, _size.LowPart, mapping_name));

		if (!out_handle.ptr)
			return make_status(HRESULT_FROM_WIN32(GetLastError()));

		return {};
	}

	[[nodiscard]] status file_as_array(file_array_handle& out_handle, const iohandle file_mapping, fio::access filemap_mode, uint64_t beg, uint64_t end) noexcept
	{
		LARGE_INTEGER _beg;

		_beg.QuadPart = beg;

		out_handle = file_array_handle(MapViewOfFile(file_mapping.ptr, access_interp_fmap(filemap_mode), _beg.HighPart, _beg.LowPart, static_cast<SIZE_T>(end - beg)));

		if(!out_handle.ptr)
			return make_status(HRESULT_FROM_WIN32(GetLastError()));

		return {};
	}

	void close_file(const iohandle file) noexcept
	{
		CloseHandle(file.ptr);
	}

	void close_file_array(const file_array_handle file_array) noexcept
	{
		UnmapViewOfFile(file_array.ptr);
	}

	[[nodiscard]] status delete_file(const char* filename) noexcept
	{
		if (!DeleteFileA(filename))
			return make_status(HRESULT_FROM_WIN32(GetLastError()));

		return {};
	}

	[[nodiscard]] status read_from_file(och::range<char>& out_read, const iohandle file, och::range<char> buf) noexcept
	{
		uint32_t bytes_read = 0;

		if (!ReadFile(file.ptr, buf.beg, static_cast<DWORD>(buf.len()), reinterpret_cast<LPDWORD>(&bytes_read), nullptr))
			return make_status(HRESULT_FROM_WIN32(GetLastError()));

		out_read = och::range<char>(buf.beg, bytes_read);

		return {};
	}

	[[nodiscard]] status write_to_file(uint32_t& out_written, const iohandle file, const och::range<const char> buf) noexcept
	{
		uint32_t bytes_written = 0;

		if (!WriteFile(file.ptr, reinterpret_cast<const void*>(buf.beg), static_cast<DWORD>(buf.len()), reinterpret_cast<LPDWORD>(&bytes_written), nullptr))
			return make_status(HRESULT_FROM_WIN32(GetLastError()));

		out_written = bytes_written;

		return {};
	}

	[[nodiscard]] status file_seek(const iohandle file, int64_t set_to, fio::setptr setptr_mode) noexcept
	{
		LARGE_INTEGER _set_to;

		_set_to.QuadPart = set_to;

		if (!SetFilePointerEx(file.ptr, _set_to, nullptr, static_cast<uint32_t>(setptr_mode)))
			return make_status(HRESULT_FROM_WIN32(GetLastError()));

		return {};
	}

	[[nodiscard]] status get_filesize(uint64_t& out_size, const iohandle file) noexcept
	{
		LARGE_INTEGER filesize;

		if (!GetFileSizeEx(file.ptr, &filesize))
			return make_status(HRESULT_FROM_WIN32(GetLastError()));

		out_size = filesize.QuadPart;

		return {};
	}

	[[nodiscard]] status set_filesize(const iohandle file, uint64_t bytes) noexcept
	{
		LARGE_INTEGER old_fileptr;

		LARGE_INTEGER _set_to;

		_set_to.QuadPart = 0;

		if (!SetFilePointerEx(file.ptr, _set_to, &old_fileptr, FILE_CURRENT))//Save current file-pointer-position into old_fileptr
			return make_status(HRESULT_FROM_WIN32(GetLastError()));

		_set_to.QuadPart = bytes;

		if (!SetFilePointerEx(file.ptr, _set_to, nullptr, FILE_BEGIN))//Set file-pointer to new EOF
			return make_status(HRESULT_FROM_WIN32(GetLastError()));

		SetEndOfFile(file.ptr);

		if (!SetFilePointerEx(file.ptr, old_fileptr, nullptr, FILE_BEGIN))//Restore initial file-pointer-position
			return make_status(HRESULT_FROM_WIN32(GetLastError()));

		return {};
	}

	[[nodiscard]] status get_filepath(och::range<char>& out_path, const iohandle file, och::range<char> buf) noexcept
	{
		DWORD path_chars = GetFinalPathNameByHandleA(file.ptr, buf.beg, (DWORD)buf.len(), 0);

		if (path_chars > buf.len())
			return make_status(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
		
		if (!path_chars)
			return make_status(HRESULT_FROM_WIN32(GetLastError()));

		out_path = och::range<char>(buf.beg, path_chars);

		return {};
	}

	[[nodiscard]] status get_last_write_time(och::time& out_time, const iohandle file) noexcept
	{
		FILE_BASIC_INFO info;

		if (!GetFileInformationByHandleEx(file.ptr, FileBasicInfo, &info, sizeof(info)))
			return make_status(HRESULT_FROM_WIN32(GetLastError()));

		out_time = { static_cast<uint64_t>(info.LastWriteTime.QuadPart) };

		return {};
	}

	[[nodiscard]] status create_tempfile(iohandle& out_handle, fio::share share_mode) noexcept
	{
		out_handle = iohandle(nullptr);

		char filename[MAX_PATH + 1];

		if (!GetTempFileNameA(".", "och", 0, filename))
			return make_status(HRESULT_FROM_WIN32(GetLastError()));

		check(open_file(out_handle, filename, fio::access::readwrite, fio::open::normal, fio::open::fail, share_mode, fio::flag::temporary));

		return {};
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*//////////////////////////////////////////////////filehandle///////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	[[nodiscard]] status filehandle::create(const char* filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode) noexcept
	{
		check(open_file(handle, filename, access_rights, existing_mode, new_mode, share_mode));

		return {};
	}

	[[nodiscard]] status filehandle::create(const och::stringview& filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode) noexcept
	{
		check(create(filename.raw_cbegin(), access_rights, existing_mode, new_mode, share_mode));

		return {};
	}

	[[nodiscard]] status filehandle::create(const och::string& filename, fio::access access_rights, fio::open existing_mode, fio::open new_mode, fio::share share_mode) noexcept
	{
		check(create(filename.raw_cbegin(), access_rights, existing_mode, new_mode, share_mode));

		return {};
	}

	[[nodiscard]] status filehandle::create_temp(fio::share share_mode) noexcept
	{
		check(create_tempfile(handle, share_mode));

		return {};
	}

	void filehandle::close() noexcept
	{
		close_file(handle);

		handle = iohandle(nullptr);
	}

	[[nodiscard]] status filehandle::get_size(uint64_t& out_size) const noexcept
	{
		check(get_filesize(out_size, handle));

		return {};
	}

	[[nodiscard]] status filehandle::set_size(uint64_t bytes) const noexcept
	{
		check(set_filesize(handle, bytes));

		return {};
	}

	[[nodiscard]] status filehandle::path(och::range<char>& out_path, och::range<char> buf) const noexcept
	{
		check(get_filepath(out_path, handle, buf));

		return {};
	}

	[[nodiscard]] status filehandle::seek(int64_t set_to, fio::setptr setptr_mode) const noexcept
	{
		check(file_seek(handle, set_to, setptr_mode));

		return {};
	}

	[[nodiscard]] status filehandle::last_write_time(och::time& out_time) const noexcept
	{
		check(get_last_write_time(out_time, handle));

		return {};
	}

	filehandle::~filehandle() noexcept
	{
		close();
	}


	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////file_search///////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	file_search::file_info::file_info(const file_search& data) noexcept : m_data{ data } {}

	och::string file_search::file_info::name() const noexcept
	{
		return och::string(m_data.m_info_data.name);
	}

	och::time file_search::file_info::creation_time() const noexcept
	{
		return m_data.m_info_data.creation_time;
	}

	och::time file_search::file_info::last_access_time() const noexcept
	{
		return m_data.m_info_data.last_access_time;
	}

	och::time file_search::file_info::last_modification_time() const noexcept
	{
		return m_data.m_info_data.last_write_time;
	}

	uint64_t file_search::file_info::size() const noexcept
	{
		return m_data.m_info_data.size;
	}

	bool file_search::file_info::is_directory() const noexcept
	{
		return static_cast<uint32_t>(m_data.m_info_data.attributes & fio::flag::directory);
	}

	och::string file_search::file_info::ending() const noexcept
	{
		if (static_cast<uint32_t>(m_data.m_info_data.attributes & fio::flag::directory))
			return och::string("");
		
		const char* beg = m_data.m_info_data.name;

		while (*beg != '.')
			if (!*beg++)
				return och::string("");
		
		return och::string(beg + 1);
	}

	och::string file_search::file_info::absolute_name() const noexcept
	{
		och::string str(m_data.m_search_path);

		str += och::stringview(m_data.m_info_data.name);

		return std::move(str);
	}



	void file_search::file_iterator::operator++() noexcept
	{
		m_search->advance();
	}

	bool file_search::file_iterator::operator!=(const file_iterator& rhs) const noexcept
	{
		rhs; return m_search->has_next();
	}

	file_search::file_info file_search::file_iterator::operator*() const noexcept
	{
		return file_info(*m_search);
	}

	file_search::file_iterator::file_iterator(file_search* search) noexcept : m_search{ search } {}



	och::status file_search::create(const char* path, fio::search search_mode, const char* ending_filters) noexcept
	{
		check(create(och::stringview(path), search_mode, ending_filters));

		return {};
	}

	och::status file_search::create(const och::utf8_string& path, fio::search search_mode, const char* ending_filters) noexcept
	{
		check(create(och::stringview(path), search_mode, ending_filters));

		return {};
	}

	och::status file_search::create(const och::stringview& path, fio::search search_mode, const char* ending_filters) noexcept
	{
		// Process Path

		char* curr = m_search_path;

		if (path.get_codeunits() + 2 - ((*(path.end()) == och::utf8_char('\\')) || (*(path.end()) == och::utf8_char('/'))) < static_cast<uint32_t>(sizeof(m_search_path)))
		{
			for (uint32_t i = 0; i != path.get_codeunits(); ++i)
			{
				char c = path.raw_cbegin()[i];

				if (c == '+')
					break;

				if (c == '/')
					c = '\\';

				*curr++ = c;
			}

			if(*(curr - 1) != '\\')
				*curr++ = '\\';

			*curr++ = '*';
		}

		*curr = '\0';


		// Process filters

		uint32_t filter_idx = 0;

		if (ending_filters)
		{
			const char* f = ending_filters;

			while (*f && filter_idx != sizeof(m_ending_filters) / sizeof(m_ending_filters[0]))
			{
				if (*f == '.')
					++f;

				uint32_t char_idx = 0;

				while (*f && *f != '\\' && *f != '/' && char_idx < sizeof(m_ending_filters[0]))
					m_ending_filters[filter_idx][char_idx++] = *f++;

				if (char_idx != sizeof(m_ending_filters[0]) && char_idx != 0 && !(char_idx == 1 && m_ending_filters[filter_idx][0] == '?'))
					m_ending_filters[filter_idx++][char_idx] = '\0';
				else
					m_ending_filters[filter_idx][0] = '\0';

				if (*f == '\\' || *f == '/')
					++f;
			}
		}

		m_ending_filters[filter_idx][0] = '\0';


		// Write search mode and 

		m_info_data.flags_and_padding = 1 | (static_cast<uint32_t>(search_mode) << 1);


		// Check if the search handle can actually be created

		search_handle.ptr = FindFirstFileA(m_search_path, reinterpret_cast<WIN32_FIND_DATAA*>(reinterpret_cast<char*>(&m_info_data) + 4));

		if (search_handle.ptr == INVALID_HANDLE_VALUE)
		{
			search_handle.ptr = nullptr;

			m_info_data.flags_and_padding &= ~1;

			return make_status(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Skip to first valid file.

		while (has_next() && get_info().name() == ".." || get_info().name() == ".")
			advance();

		*(curr - 1) = '\0';

		return {};
	}

	void file_search::close() noexcept
	{
		FindClose(search_handle.ptr);

		search_handle.ptr = nullptr;
	}

	och::status file_search::advance() noexcept
	{
		check(single_advance());
		
		const fio::search mode = static_cast<fio::search>(m_info_data.flags_and_padding >> 1);

		if (mode == fio::search::directories)
		{
			while (has_next() && (!static_cast<uint32_t>(m_info_data.attributes & fio::flag::directory) || !matches_ending_filter(get_info().ending().raw_cbegin())))
				check(single_advance());
		}
		else if (mode == fio::search::files)
		{
			while (has_next() && (static_cast<uint32_t>(m_info_data.attributes & fio::flag::directory) || !matches_ending_filter(get_info().ending().raw_cbegin())))
				check(HRESULT_FROM_WIN32(single_advance()));
		}

		return {};
	}

	bool file_search::has_next() const noexcept
	{
		return m_info_data.flags_and_padding & 1;
	}

	file_search::file_info file_search::get_info() const noexcept
	{
		return file_info(*this);
	}

	file_search::file_iterator file_search::begin() noexcept
	{
		return file_iterator(this);
	}

	file_search::file_iterator file_search::end() noexcept
	{
		return file_iterator(nullptr);
	}

	file_search::~file_search() noexcept
	{
		close();
	}

	uint32_t file_search::single_advance() noexcept
	{
		BOOL find_result = FindNextFileA(search_handle.ptr, reinterpret_cast<WIN32_FIND_DATAA*>(reinterpret_cast<char*>(&m_info_data) + 4));

		if (!find_result)
		{
			m_info_data.flags_and_padding &= ~1u;

			DWORD last_error = GetLastError();

			if (last_error != ERROR_NO_MORE_FILES)
				return last_error;
		}

		return 0;
	}

	bool file_search::matches_ending_filter(const char* ending) const noexcept
	{
		if (!m_ending_filters[0][0])
			return true;

		for (uint32_t i = 0; i != sizeof(m_ending_filters) / sizeof(m_ending_filters[0]); ++i)
		{
			if (!m_ending_filters[i][0])
				break;

			if (m_ending_filters[i][0] == '?')
				for (uint32_t j = 0; ending[j] && j != sizeof(m_ending_filters[0] - 1); ++j)
				{
					char f = m_ending_filters[i][j + 1];
					char e = ending[j];

					if (f >= 'a' && f <= 'z')
						f += 'A' - 'a';

					if (e >= 'a' && e <= 'z')
						e += 'A' - 'a';

					if(f != e)
						goto NO_MATCH;
				}
			else
				for (uint32_t j = 0; ending[j] && j != sizeof(m_ending_filters[0]); ++j)
					if (ending[j] != m_ending_filters[i][j])
						goto NO_MATCH;

			return true;

		NO_MATCH:;
		}

		return false;
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

#endif // _WIN32
