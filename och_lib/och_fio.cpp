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

	iohandle open_file(const char* filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode, uint32_t share_mode)
	{
		void* file = CreateFileA(filename, access_interp_open(access_rights), share_mode, nullptr, interp_openmode(existing_mode, new_mode), FILE_ATTRIBUTE_NORMAL, nullptr);

		return file == INVALID_HANDLE_VALUE ? nullptr : file;
	}

	iohandle create_file_mapper(iohandle file, uint64_t size, uint32_t page_mode, const char* mapping_name)
	{
		if (!file._ptr)
			return iohandle{ nullptr };

		LARGE_INTEGER _size;

		_size.QuadPart = size;

		return CreateFileMappingA(file._ptr, nullptr, access_interp_page(page_mode), _size.HighPart, _size.LowPart, mapping_name);
	}

	iohandle file_as_array(iohandle file_mapping, uint32_t filemap_mode, uint64_t beg, uint64_t end)
	{
		LARGE_INTEGER _beg;

		_beg.QuadPart = beg;

		return MapViewOfFile(file_mapping._ptr, access_interp_fmap(filemap_mode), _beg.HighPart, _beg.LowPart, end - beg);
	}

	uint32_t readbytes(iohandle file, char* dst, uint32_t bytes)
	{
		uint32_t bytes_read = 0;

		ReadFile(file._ptr, dst, bytes, reinterpret_cast<LPDWORD>(&bytes_read), nullptr);

		return bytes_read;
	}

	int64_t get_filesize(iohandle file)
	{
		LARGE_INTEGER filesize;

		GetFileSizeEx(file._ptr, &filesize);

		return filesize.QuadPart;
	}

	bool close_file(iohandle file)
	{
		return CloseHandle(file._ptr);
	}

	bool close_file_array(iohandle file_array)
	{
		return UnmapViewOfFile(file_array._ptr);
	}

	bool delete_file(const char* filename)
	{
		return DeleteFileA(filename);
	}
}

#endif // _WIN32
