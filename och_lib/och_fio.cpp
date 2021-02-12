#include "och_fio.h"

#ifdef _WIN32

#include <Windows.h>

namespace och
{
	uint32_t interp_openmode(uint32_t existing_mode, uint32_t new_mode) noexcept
	{
		switch (existing_mode)
		{
		case(fio::open_normal):
			switch (new_mode)
			{
			case(fio::open_normal):		return  4;
			case fio::open_truncate:	return  4;
			case fio::open_fail:		return  3;
			}
			break;

		case fio::open_truncate:
			switch (new_mode)
			{
			case(fio::open_normal):		return  2;
			case fio::open_truncate:	return  2;
			case fio::open_fail:		return  5;
			}

		case fio::open_fail:
			switch (new_mode)
			{
			case(fio::open_normal):		return  1;
			case fio::open_truncate:	return  1;
			case fio::open_fail:		return -1;
			}
		}
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

	iohandle open_file(const och::string filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode, uint32_t flags) noexcept
	{
		iohandle file = CreateFileA(filename.beg, access_interp_open(access_rights), share_mode, nullptr, interp_openmode(existing_mode, new_mode), flags, nullptr);

		return file.ptr == INVALID_HANDLE_VALUE ? nullptr : file;
	}

	iohandle create_file_mapper(iohandle file, uint64_t size, uint32_t page_mode, const char* mapping_name) noexcept
	{
		if (!file.ptr)
			return nullptr;

		LARGE_INTEGER _size;

		_size.QuadPart = size;

		return CreateFileMappingA(file.ptr, nullptr, access_interp_page(page_mode), _size.HighPart, _size.LowPart, mapping_name);
	}

	iohandle file_as_array(iohandle file_mapping, uint32_t filemap_mode, uint64_t beg, uint64_t end) noexcept
	{
		LARGE_INTEGER _beg;

		_beg.QuadPart = beg;

		return MapViewOfFile(file_mapping.ptr, access_interp_fmap(filemap_mode), _beg.HighPart, _beg.LowPart, static_cast<SIZE_T>(end - beg));
	}

	bool close_file(iohandle file) noexcept
	{
		return CloseHandle(file.ptr);
	}

	bool close_file_array(iohandle file_array) noexcept
	{
		return UnmapViewOfFile(file_array.ptr);
	}

	bool delete_file(const och::string filename) noexcept
	{
		return DeleteFileA(filename.beg);
	}

	och::memrun<char> read_from_file(iohandle file, och::memrun<char> buf) noexcept
	{
		uint32_t bytes_read = 0;

		ReadFile(file.ptr, buf.beg, static_cast<DWORD>(buf.len()), reinterpret_cast<LPDWORD>(&bytes_read), nullptr);

		return och::memrun<char>(buf.beg, bytes_read);
	}

	uint32_t write_to_file(iohandle file, const och::string buf) noexcept
	{
		uint32_t bytes_written = 0;

		WriteFile(file.ptr, reinterpret_cast<const void*>(buf.beg), static_cast<DWORD>(buf.len()), reinterpret_cast<LPDWORD>(&bytes_written), nullptr);

		return bytes_written;
	}

	bool file_seek(iohandle file, int64_t set_to, uint32_t setptr_mode) noexcept
	{
		LARGE_INTEGER _set_to;

		_set_to.QuadPart = set_to;

		return SetFilePointerEx(file.ptr, _set_to, nullptr, static_cast<DWORD>(setptr_mode));
	}

	int64_t get_filesize(iohandle file) noexcept
	{
		LARGE_INTEGER filesize;

		GetFileSizeEx(file.ptr, &filesize);

		return filesize.QuadPart;
	}

	bool set_filesize(iohandle file, uint64_t bytes) noexcept
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

	och::memrun<char> get_filepath(iohandle file, och::memrun<char> buf) noexcept
	{
		return och::memrun<char>(buf.beg, GetFinalPathNameByHandleA(file.ptr, buf.beg, (DWORD) buf.len(), 0));
	}

	och::time get_last_write_time(iohandle file) noexcept
	{
		FILE_BASIC_INFO info;

		if (!GetFileInformationByHandleEx(file.ptr, FileBasicInfo, &info, sizeof(info)))
			return { 0 };

		return { static_cast<uint64_t>(info.LastWriteTime.QuadPart) };
	}

	iohandle create_tempfile(uint32_t share_mode) noexcept
	{
		char filename[MAX_PATH];

		if (!GetTempFileNameA(".", "och", 0, filename))
			return nullptr;

		return open_file(filename, fio::access_readwrite, fio::open_normal, fio::open_fail, share_mode, fio::flag_temporary);
	}

	tempfilehandle::tempfilehandle(uint32_t share_mode) noexcept : filehandle{ create_tempfile(share_mode) } {}

	tempfilehandle::~tempfilehandle() noexcept
	{
		char buf[MAX_PATH + 1];

		GetFinalPathNameByHandleA(handle.ptr, buf, sizeof(buf), 0);

		close_file(handle);

		delete_file(buf);
	}

	file_search::file_search(const och::string path) noexcept : search_handle{FindFirstFileA(path.beg, reinterpret_cast<WIN32_FIND_DATAA*>(reinterpret_cast<char*>(&curr_data) + 4))} {}

	file_search::~file_search() noexcept { FindClose(search_handle.ptr); }

	bool file_search::next() noexcept { return FindNextFileA(search_handle.ptr, reinterpret_cast<WIN32_FIND_DATAA*>(reinterpret_cast<char*>(&curr_data) + 4)); }

	och::string file_search::name() const noexcept { return { curr_data.name }; }

	bool file_search::is_dir() const noexcept { return curr_data.attributes & fio::flag_directory; }

	filehandle file_search::open(uint32_t access_rights, uint32_t share_mode) const noexcept { return filehandle(curr_data.name, access_rights, fio::open_normal, fio::open_fail, share_mode); }

	iohandle get_stdout()
	{
		return GetStdHandle(STD_OUTPUT_HANDLE);
	}

	iohandle get_stdin()
	{
		return GetStdHandle(STD_INPUT_HANDLE);
	}

	iohandle get_stderr()
	{
		return GetStdHandle(STD_ERROR_HANDLE);
	}
}

#endif // _WIN32
