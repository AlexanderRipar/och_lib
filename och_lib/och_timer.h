#pragma once

#include "och_time.h"
namespace och
{
	struct highres_timer
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

	struct timer
	{
	private:
		time t = time::now();

	public:

		timespan read() const noexcept
		{
			return time::now() - t;
		}

		timespan reset() noexcept
		{
			time beg = t;

			t = time::now();

			return beg - t;
		}
	};
}
