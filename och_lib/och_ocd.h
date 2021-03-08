#pragma once

#include "och_fio.h"
#include "och_fmt.h"

namespace och
{
	struct ocd_file
	{
		struct ocd_header
		{
			
		};

		struct ocd_stub
		{

		};

		struct ocd_data
		{

		};

		filehandle f;

		struct entry_idx
		{
			uint32_t offset;
		};

		entry_idx get_entry_idx_by_name(const och::stringview& id) const noexcept
		{


			return {};
		}

		och::type_union operator[](entry_idx idx) const noexcept
		{
			return {};
		}
	};
}
