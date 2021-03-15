#pragma once

#include "och_fio.h"
#include "och_fmt.h"
#include "och_utf8.h"
#include "och_range.h"


namespace och
{
	struct ocd_file
	{
		filehandle m_filehandle;
		iohandle m_optional_header_handle = nullptr;

		ocd_file(const och::stringview& filename, uint32_t access_rights, uint32_t existing_mode, uint32_t new_mode) noexcept : m_filehandle{ filename, access_rights, existing_mode, new_mode }
		{
			
		}
	};
}
