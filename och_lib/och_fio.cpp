#include "och_fio.h"

#include "och_utf8.h"

#ifdef _WIN32

#include <Windows.h>

namespace och
{
	iohandle::iohandle(void* h) noexcept : ptr{ h } {}

	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*////////////////////////////////////////////////Free functions/////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	uint32_t interp_openmode(uint32_t existing_mode, uint32_t new_mode) noexcept
	{
		switch (existing_mode)
		{
		case fio::open_normal:
		case fio::open_append:
			switch (new_mode)
			{
			case fio::open_normal:		return  4;
			case fio::open_append:		return  4;
			case fio::open_truncate:	return  4;
			case fio::open_fail:		return  3;
			}
			break;

		case fio::open_truncate:
			switch (new_mode)
			{
			case fio::open_normal:		return  2;
			case fio::open_append:		return  2;
			case fio::open_truncate:	return  2;
			case fio::open_fail:		return  5;
			}

		case fio::open_fail:
			switch (new_mode)
			{
			case fio::open_normal:		return  1;
			case fio::open_append:		return  1;
			case fio::open_truncate:	return  1;
			case fio::open_fail:		return ~0u;
			}
		}

		return ~0u;
	}

	uint32_t access_interp_open(uint32_t access_rights) noexcept
	{
		return access_rights << 30;
	}

	uint32_t access_interp_page(uint32_t access_rights) noexcept
	{
		return ((access_rights ^ 3) + ((access_rights == 3) << 1)) << 1;
	}

	uint32_t access_interp_fmap(uint32_t access_rights) noexcept
	{
		return (access_rights - ((access_rights == 3) << 1)) << 1;
	}

	iohandle open_file(const char* filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode, uint32_t flags) noexcept
	{
		iohandle file(CreateFileA(filename, access_interp_open(access_rights), share_mode, nullptr, interp_openmode(existing_mode, new_mode), flags, nullptr));

		if (existing_mode == fio::open_append)
			file_seek(file, 0, fio::setptr_end);

		return file.ptr == INVALID_HANDLE_VALUE ? iohandle(nullptr) : file;
	}

	iohandle create_file_mapper(const iohandle file, uint64_t size, uint32_t page_mode, const char* mapping_name) noexcept
	{
		if (!file.ptr)
			return iohandle(nullptr);

		LARGE_INTEGER _size;

		_size.QuadPart = size;

		return iohandle(CreateFileMappingA(file.ptr, nullptr, access_interp_page(page_mode), _size.HighPart, _size.LowPart, mapping_name));
	}

	iohandle file_as_array(const iohandle file_mapping, uint32_t filemap_mode, uint64_t beg, uint64_t end) noexcept
	{
		LARGE_INTEGER _beg;

		_beg.QuadPart = beg;

		return iohandle(MapViewOfFile(file_mapping.ptr, access_interp_fmap(filemap_mode), _beg.HighPart, _beg.LowPart, static_cast<SIZE_T>(end - beg)));
	}

	bool close_file(const iohandle file) noexcept
	{
		return CloseHandle(file.ptr);
	}

	bool close_file_array(const iohandle file_array) noexcept
	{
		return UnmapViewOfFile(file_array.ptr);
	}

	bool delete_file(const char* filename) noexcept
	{
		return DeleteFileA(filename);
	}

	och::range<char> read_from_file(const iohandle file, och::range<char> buf) noexcept
	{
		uint32_t bytes_read = 0;

		ReadFile(file.ptr, buf.beg, static_cast<DWORD>(buf.len()), reinterpret_cast<LPDWORD>(&bytes_read), nullptr);

		return och::range<char>(buf.beg, bytes_read);
	}

	uint32_t write_to_file(const iohandle file, const och::range<const char> buf) noexcept
	{
		uint32_t bytes_written = 0;

		WriteFile(file.ptr, reinterpret_cast<const void*>(buf.beg), static_cast<DWORD>(buf.len()), reinterpret_cast<LPDWORD>(&bytes_written), nullptr);

		return bytes_written;
	}

	bool file_seek(const iohandle file, int64_t set_to, uint32_t setptr_mode) noexcept
	{
		LARGE_INTEGER _set_to;

		_set_to.QuadPart = set_to;

		return SetFilePointerEx(file.ptr, _set_to, nullptr, static_cast<DWORD>(setptr_mode));
	}

	int64_t get_filesize(const iohandle file) noexcept
	{
		LARGE_INTEGER filesize;

		GetFileSizeEx(file.ptr, &filesize);

		return filesize.QuadPart;
	}

	bool set_filesize(const iohandle file, uint64_t bytes) noexcept
	{
		LARGE_INTEGER old_fileptr;

		LARGE_INTEGER _set_to;

		_set_to.QuadPart = 0;

		if (!SetFilePointerEx(file.ptr, _set_to, &old_fileptr, FILE_CURRENT))//Save current file-pointer-position into old_fileptr
			return false;

		_set_to.QuadPart = bytes;

		if (!SetFilePointerEx(file.ptr, _set_to, nullptr, FILE_BEGIN))//Set file-pointer to new EOF
			return false;

		SetEndOfFile(file.ptr);

		if (!SetFilePointerEx(file.ptr, old_fileptr, nullptr, FILE_BEGIN))//Restore initial file-pointer-position
			return false;

		return true;
	}

	och::range<char> get_filepath(const iohandle file, och::range<char> buf) noexcept
	{
		return och::range<char>(buf.beg, GetFinalPathNameByHandleA(file.ptr, buf.beg, (DWORD)buf.len(), 0));
	}

	och::time get_last_write_time(const iohandle file) noexcept
	{
		FILE_BASIC_INFO info;

		if (!GetFileInformationByHandleEx(file.ptr, FileBasicInfo, &info, sizeof(info)))
			return { 0 };

		return { static_cast<uint64_t>(info.LastWriteTime.QuadPart) };
	}

	iohandle create_tempfile(uint32_t share_mode) noexcept
	{
		char filename[MAX_PATH + 1];

		if (!GetTempFileNameA(".", "och", 0, filename))
			return iohandle(nullptr);

		return open_file(filename, fio::access_readwrite, fio::open_normal, fio::open_fail, share_mode, fio::flag_temporary);
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*//////////////////////////////////////////////////filehandle///////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	filehandle::filehandle(const och::stringview& filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode) noexcept : filehandle(filename.raw_cbegin(), access_rights, existing_mode, new_mode, share_mode) {};

	filehandle::filehandle(const char* filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode) noexcept : handle{ open_file(filename, access_rights, existing_mode, new_mode, share_mode) } {}

	filehandle::filehandle(const och::utf8_string& filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode) noexcept : filehandle(filename.raw_cbegin(), access_rights, existing_mode, new_mode, share_mode) {};

	filehandle::~filehandle() noexcept { close_file(handle); }

	[[nodiscard]] uint64_t filehandle::get_size() const noexcept { return get_filesize(handle); }

	bool filehandle::set_size(uint64_t bytes) const noexcept { return set_filesize(handle, bytes); }

	[[nodiscard]] och::range<char> filehandle::path(och::range<char> buf) const noexcept { return get_filepath(handle, buf); }

	bool filehandle::seek(int64_t set_to, uint32_t setptr_mode) const noexcept { return file_seek(handle, set_to, setptr_mode); }

	[[nodiscard]] och::time filehandle::last_write_time() const noexcept { return get_last_write_time(handle); }

	void filehandle::close() const noexcept { close_file(handle); }

	filehandle::operator bool() const noexcept { return handle.ptr; }

	filehandle::filehandle(iohandle handle) : handle{ handle } {}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*////////////////////////////////////////////////tempfilehandle/////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	tempfilehandle::tempfilehandle(uint32_t share_mode) noexcept : filehandle{ create_tempfile(share_mode) } {}

	tempfilehandle::~tempfilehandle() noexcept
	{
		char buf[MAX_PATH + 1];

		GetFinalPathNameByHandleA(handle.ptr, buf, sizeof(buf), 0);

		close_file(handle);

		delete_file(buf);
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////file_search///////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	file_search::file_info::file_info(const file_search& data) noexcept : m_data{ data } {}

	och::stringview file_search::file_info::name() const noexcept
	{
		return och::utf8_view(m_data.m_info_data.name);
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
		return m_data.m_info_data.attributes & fio::flag_directory;
	}

	och::stringview file_search::file_info::ending() const noexcept
	{
		if (m_data.m_info_data.attributes & fio::flag_directory)
			return och::stringview("");
		
		const char* beg = m_data.m_info_data.name;

		while (*beg != '.')
			if (!*beg++)
				return och::stringview("");
		
		return och::stringview(beg + 1);
	}

	och::utf8_string file_search::file_info::absolute_name() const noexcept
	{
		och::string str(m_data.m_search_path);

		str += och::stringview(m_data.m_info_data.name);

		return std::move(str);
	}



	void file_search::file_iterator::operator++() noexcept
	{
		m_search->next();
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



	file_search::file_search(const char* path) noexcept : file_search(och::stringview(path)) {}

	file_search::file_search(const och::utf8_string& path) noexcept : file_search(path.raw_cbegin()) {}

	file_search::file_search(const och::stringview& path) noexcept
	{
		char* curr = m_search_path;

		if (path.get_codeunits() + 3 <= sizeof(m_search_path))
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

		search_handle.ptr = FindFirstFileA(m_search_path, reinterpret_cast<WIN32_FIND_DATAA*>(reinterpret_cast<char*>(&m_info_data) + 4));

		while (get_info().name() == ".." || get_info().name() == ".")
			next();

		*(curr - 1) = '\0';
	}

	file_search::~file_search() noexcept
	{
		FindClose(search_handle.ptr);
	}

	bool file_search::next() noexcept
	{
		return m_info_data._padding = FindNextFileA(search_handle.ptr, reinterpret_cast<WIN32_FIND_DATAA*>(reinterpret_cast<char*>(&m_info_data) + 4));
	}

	bool file_search::has_next() const noexcept
	{
		return m_info_data._padding;
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
