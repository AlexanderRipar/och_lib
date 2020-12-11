#pragma once

#include "och_time.h"
namespace och
{
	struct timer
	{
	private:
		highres_time t = highres_time::now();

	public:

		highres_timespan read() const noexcept
		{
			return highres_time::now() - t;
		}

		highres_timespan reset() noexcept
		{
			highres_time beg = t;

			t = highres_time::now();

			return beg - t;
		}
	};
}
