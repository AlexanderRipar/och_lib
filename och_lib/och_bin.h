#pragma once

#include "och_fio.h"
#include "och_fmt.h"

namespace och
{
	struct binary_log_file
	{
		filehandle f;

		struct entry_idx
		{
			uint32_t offset;
		};

		struct entry
		{
			union
			{
				float f32;
				double f64;
				uint64_t u64;
				uint32_t u32;
			};
		};

		uint32_t get_entry_by_name(const och::stringview& id)
		{

		}

		uint32_t operator[]()
	};
}
