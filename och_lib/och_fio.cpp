#include "och_fio.h"

#ifdef _WIN32

#include <Windows.h>

namespace och
{
	uint32_t interp_openmode(uint32_t existing_mode, uint32_t new_mode)
	{
		switch (existing_mode)
		{
		case(fio::open_normal):
			switch (new_mode)
			{
			case(fio::open_normal):		return  4;
			case fio::open_truncate:	return  4;
			case fio::open_fail:		return  3;
			default:					__assume(0);
			}
			break;

		case fio::open_truncate:
			switch (new_mode)
			{
			case(fio::open_normal):		return  2;
			case fio::open_truncate:	return  2;
			case fio::open_fail:		return  5;
			default:					__assume(0);
			}

		case fio::open_fail:
			switch (new_mode)
			{
			case(fio::open_normal):		return  1;
			case fio::open_truncate:	return  1;
			case fio::open_fail:		return -1;
			default:					__assume(0);
			}
		default:					__assume(0);
		}
	}

	uint32_t access_interp_open(uint32_t access_rights)
	{
		return access_rights << 30;
	}

	uint32_t access_interp_page(uint32_t access_rights)
	{
		return ((access_rights ^ 3) + ((access_rights == 3) << 1)) << 1;
	}

	uint32_t access_interp_fmap(uint32_t access_rights)
	{
		return (access_rights - ((access_rights == 3) << 1)) << 1;
	}

	iohandle open_file(const och::string filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode)
	{
		iohandle file = CreateFileA(filename.begin(), access_interp_open(access_rights), share_mode, nullptr, interp_openmode(existing_mode, new_mode), FILE_ATTRIBUTE_NORMAL, nullptr);

		return file == INVALID_HANDLE_VALUE ? nullptr : file;
	}

	iohandle create_file_mapper(const iohandle file, uint64_t size, uint32_t page_mode, const char* mapping_name)
	{
		if (!file)
			return nullptr;

		LARGE_INTEGER _size;

		_size.QuadPart = size;

		return CreateFileMappingA(file, nullptr, access_interp_page(page_mode), _size.HighPart, _size.LowPart, mapping_name);
	}

	iohandle file_as_array(const iohandle file_mapping, uint32_t filemap_mode, uint64_t beg, uint64_t end)
	{
		LARGE_INTEGER _beg;

		_beg.QuadPart = beg;

		return MapViewOfFile(file_mapping, access_interp_fmap(filemap_mode), _beg.HighPart, _beg.LowPart, end - beg);
	}

	bool set_fileptr(const iohandle file, int64_t set_to, uint32_t setptr_mode)
	{
		LARGE_INTEGER _set_to;

		_set_to.QuadPart = set_to;

		return SetFilePointerEx(file, _set_to, nullptr, static_cast<DWORD>(setptr_mode));
	}

	uint32_t freadbytes(const iohandle file, char* dst, uint32_t bytes)
	{
		uint32_t bytes_read = 0;

		ReadFile(file, dst, bytes, reinterpret_cast<LPDWORD>(&bytes_read), nullptr);

		return bytes_read;
	}

	uint32_t fwritebytes(const iohandle file, const char* src, uint32_t bytes)
	{
		uint32_t bytes_written = 0;

		WriteFile(file, reinterpret_cast<const void*>(src), bytes, reinterpret_cast<LPDWORD>(&bytes_written), nullptr);

		return bytes_written;
	}

	int64_t get_filesize(const iohandle file)
	{
		LARGE_INTEGER filesize;

		GetFileSizeEx(file, &filesize);

		return filesize.QuadPart;
	}

	bool close_file(const iohandle file)
	{
		return CloseHandle(file);
	}

	bool close_file_array(const iohandle file_array)
	{
		return UnmapViewOfFile(file_array);
	}

	bool delete_file(const och::string filename)
	{
		return DeleteFileA(filename.begin());
	}

	int32_t get_filepath(const iohandle file, och::memrun<char> buf)
	{
		return GetFinalPathNameByHandleA(file, buf.begin(), (DWORD) buf.len(), 0);
	}

	uint64_t get_last_write_time(const iohandle file)
	{
		FILE_BASIC_INFO info;

		if (!GetFileInformationByHandleEx(file, FileBasicInfo, &info, sizeof(info)))
			return 0;

		return info.LastWriteTime.QuadPart;
	}

	iohandle create_tempfile(uint32_t share_mode)
	{
		char filename[MAX_PATH];

		if (!GetTempFileNameA(".", "och", 0, filename))
			return nullptr;

		return open_file(filename, fio::access_readwrite, fio::open_normal, fio::open_fail, share_mode);
	}

	tempfilehandle::tempfilehandle(uint32_t share_mode) : handle{ create_tempfile(share_mode) } {}

	tempfilehandle::~tempfilehandle()
	{
		char buf[MAX_PATH];

		GetFinalPathNameByHandleA(handle, buf, MAX_PATH, 0);

		close_file(handle);

		delete_file(buf);
	}
}

#endif // _WIN32
