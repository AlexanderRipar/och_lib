#include "och_time.h"

#include "och_fmt.h"
#include <Windows.h>

namespace och
{
	bool timezone_initialized = false;
	DYNAMIC_TIME_ZONE_INFORMATION timezone_info;
	uint64_t highres_clock_ticks_per_second;

	void init_time()
	{
		if (timezone_initialized)
			return;

		timezone_initialized = true;

		GetDynamicTimeZoneInformation(&timezone_info);

		QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&highres_clock_ticks_per_second));
	}



	timepoint::timepoint(const date& utc) noexcept
	{
		SystemTimeToFileTime(reinterpret_cast<const SYSTEMTIME*>(&utc), reinterpret_cast<FILETIME*>(this));
	}

	timepoint timepoint::now() noexcept
	{
		timepoint t;

		GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&t));

		return t;
	}

	timediff timepoint::since_epoch() const noexcept
	{
		return { val };
	}

	timediff timepoint::operator-(timepoint rhs) const noexcept
	{
		return { val - rhs.val };
	}



	date::date(const local_date& local) noexcept
	{
		TzSpecificLocalTimeToSystemTimeEx(&timezone_info, reinterpret_cast<const SYSTEMTIME*>(&local), reinterpret_cast<SYSTEMTIME*>(this));
	}

	date::date(timepoint tp) noexcept
	{
		FileTimeToSystemTime(reinterpret_cast<FILETIME*>(&tp), reinterpret_cast<SYSTEMTIME*>(this));
	}

	date date::now() noexcept
	{
		date d;

		GetSystemTime(reinterpret_cast<SYSTEMTIME*>(&d));

		return d;
	}



	local_date::local_date(const date& utc) noexcept
	{
		SystemTimeToTzSpecificLocalTimeEx(&timezone_info, reinterpret_cast<const SYSTEMTIME*>(&utc), reinterpret_cast<SYSTEMTIME*>(this));
	}

	local_date local_date::now() noexcept
	{
		local_date t;

		GetLocalTime(reinterpret_cast<SYSTEMTIME*>(&t));

		return t;
	}



	highres_timepoint highres_timepoint::now() noexcept
	{
		LARGE_INTEGER t;

		QueryPerformanceCounter(&t);

		return { static_cast<uint64_t>(t.QuadPart) };
	}
}
