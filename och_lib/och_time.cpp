#include "och_time.h"

#include "och_fmt.h"
#include <Windows.h>

namespace och
{
	//Internals

	struct time_data_t
	{
		int64_t tz_bias;
		uint64_t highres_clock_ticks_per_second;

		time_data_t()
		{
			DYNAMIC_TIME_ZONE_INFORMATION tz;

			GetDynamicTimeZoneInformation(&tz);

			tz_bias = tz.Bias * 600000000llu;
		}
	};

	time_data_t time_data;

	/*
	int64_t g_timezone_offset;

	uint64_t g_highres_clock_ticks_per_second;

	void init_time()
	{
		DYNAMIC_TIME_ZONE_INFORMATION tz;

		GetDynamicTimeZoneInformation(&tz);

		g_timezone_offset = tz.Bias * 600000000llu;
	}
	*/

	timespan timezone_offset() noexcept { return{ time_data.tz_bias }; }

	//timespan

	timespan timespan::operator+(timespan rhs) const noexcept { return { val + rhs.val }; }

	timespan timespan::operator-(timespan rhs) const noexcept { return { val - rhs.val }; }

	void timespan::operator+=(timespan rhs) noexcept { val += rhs.val; }

	void timespan::operator-=(timespan rhs) noexcept { val -= rhs.val; }

	bool timespan::operator<(timespan rhs) const noexcept { return val < rhs.val; }

	bool timespan::operator<=(timespan rhs) const noexcept { return val <= rhs.val; }

	bool timespan::operator>(timespan rhs) const noexcept { return val > rhs.val; }

	bool timespan::operator>=(timespan rhs) const noexcept { return val >= rhs.val; }

	bool timespan::operator==(timespan rhs) const noexcept { return val == rhs.val; }

	bool timespan::operator!=(timespan rhs) const noexcept { return val != rhs.val; }



	//time

	time time::now() noexcept
	{
		time t;

		GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&t));

		return t;
	}

	time time::operator+(timespan rhs) const noexcept { return { val + rhs.val }; }

	time time::operator-(timespan rhs) const noexcept { return { val - rhs.val }; }

	timespan time::operator+(time rhs) const noexcept { return { static_cast<int64_t>(val + rhs.val) }; }

	timespan time::operator-(time rhs) const noexcept { return { static_cast<int64_t>(val - rhs.val) }; }

	void time::operator+=(timespan rhs) noexcept { val += rhs.val; }

	void time::operator-=(timespan rhs) noexcept { val -= rhs.val; }

	bool time::operator<(time rhs) const noexcept { return val < rhs.val; }

	bool time::operator<=(time rhs) const noexcept { return val <= rhs.val; }

	bool time::operator>(time rhs) const noexcept { return val > rhs.val; }

	bool time::operator>=(time rhs) const noexcept { return val >= rhs.val; }

	bool time::operator==(time rhs) const noexcept { return val == rhs.val; }

	bool time::operator!=(time rhs) const noexcept { return val != rhs.val; }



	//time_info

	time_info::time_info(uint16_t year, uint16_t month, uint16_t weekday, uint16_t monthday, uint16_t hour, uint16_t minute, uint16_t second, uint16_t millisecond) noexcept
		: year{ year }, month{ month }, weekday{ weekday }, monthday{ monthday }, hour{ hour }, minute{ minute }, second{ second }, millisecond{ millisecond } {}

	time_info::time_info(time t) noexcept
	{
		FileTimeToSystemTime(reinterpret_cast<FILETIME*>(&t), reinterpret_cast<SYSTEMTIME*>(this));
	}

	time_info time_info::utc_now() noexcept
	{
		time_info date;

		GetSystemTime(reinterpret_cast<SYSTEMTIME*>(&date));

		return date;
	}

	time_info time_info::local_now() noexcept
	{
		time_info date;

		GetLocalTime(reinterpret_cast<SYSTEMTIME*>(&date));

		return date;
	}

	/*
	datespan datespan::operator+(datespan rhs) const noexcept
	{
		return
		{
			(uint16_t) (year + rhs.year),
			(uint16_t) (month + rhs.month),
			(uint16_t) (monthday + rhs.monthday),
			(uint16_t) (hour + rhs.hour),
			(uint16_t) (minute + rhs.minute),
			(uint16_t) (second + rhs.second),
			(uint16_t) (millisecond + rhs.millisecond)
		};
	}

	datespan datespan::operator-(datespan rhs) const noexcept
	{
		return
		{
			(uint16_t)(year - rhs.year),
			(uint16_t)(month - rhs.month),
			(uint16_t)(monthday - rhs.monthday),
			(uint16_t)(hour - rhs.hour),
			(uint16_t)(minute - rhs.minute),
			(uint16_t)(second - rhs.second),
			(uint16_t)(millisecond - rhs.millisecond)
		};
	}

	void datespan::operator+=(datespan rhs) noexcept
	{
		year += rhs.year;
		month += rhs.month;
		monthday += rhs.monthday;
		hour += rhs.hour;
		minute += rhs.minute;
		second += rhs.second;
		millisecond += rhs.millisecond;
	}

	void datespan::operator-=(datespan rhs) noexcept
	{
		year -= rhs.year;
		month -= rhs.month;
		monthday -= rhs.monthday;
		hour -= rhs.hour;
		minute -= rhs.minute;
		second -= rhs.second;
		millisecond -= rhs.millisecond;
	}

	bool datespan::operator<(datespan rhs) const noexcept
	{
		if (year > rhs.year)
			return false;
		if (month > rhs.month)
			return false;
		if (monthday > rhs.monthday)
			return false;
		if (hour > rhs.hour)
			return false;
		if (minute > rhs.minute)
			return false;
		if (second > rhs.second)
			return false;
		if (millisecond >= rhs.millisecond)
			return false;
	}

	bool datespan::operator<=(datespan rhs) const noexcept
	{
		if (year > rhs.year)
			return false;
		if (month > rhs.month)
			return false;
		if (monthday > rhs.monthday)
			return false;
		if (hour > rhs.hour)
			return false;
		if (minute > rhs.minute)
			return false;
		if (second > rhs.second)
			return false;
		if (millisecond > rhs.millisecond)
			return false;
	}

	bool datespan::operator>(datespan rhs) const noexcept
	{
		return !operator<=(rhs);
	}

	bool datespan::operator>=(datespan rhs) const noexcept
	{
		return !operator<(rhs);
	}

	bool datespan::operator==(datespan rhs) const noexcept
	{
		return year == rhs.year && month == rhs.month && monthday == rhs.monthday && hour == rhs.hour && minute == rhs.minute && second == rhs.second && millisecond == rhs.millisecond;
	}

	bool datespan::operator!=(datespan rhs) const noexcept
	{
		return !operator==(rhs);
	}



	//date
	//TODO
	date date::now() noexcept
	{

	}
	//TODO
	datespan date::curr_timezone_offset() noexcept
	{

	}
	//TODO
	date::date(time t) noexcept
	{

	}

	date::date(uint16_t year, uint16_t month, uint16_t monthday, uint16_t hour = 0, uint16_t minute = 0, uint16_t second = 0, uint16_t millisecond = 0)
		: year{ year }, month{ month }, monthday{ monthday }, hour{ hour }, minute{ minute }, second{ second }, millisecond{ millisecond } {}
	//TODO check monthday
	bool date::is_valid() const noexcept
	{
		if (month > 12)
			return false;

		//if(monthday > whatever)

		if (hour > 23)
			return false;

		if (minute > 59)
			return false;

		if (second > 59)
			return false;

		if (millisecond > 999)
			return false;
	}

	date date::operator+(datespan rhs) const noexcept
	{
		return
		{
			(uint16_t)(year + rhs.year),
			(uint16_t)(month + rhs.month),
			(uint16_t)(monthday + rhs.monthday),
			(uint16_t)(hour + rhs.hour),
			(uint16_t)(minute + rhs.minute),
			(uint16_t)(second + rhs.second),
			(uint16_t)(millisecond + rhs.millisecond)
		};
	}

	date date::operator-(datespan rhs) const noexcept
	{
		return
		{
			(uint16_t)(year - rhs.year),
			(uint16_t)(month - rhs.month),
			(uint16_t)(monthday - rhs.monthday),
			(uint16_t)(hour - rhs.hour),
			(uint16_t)(minute - rhs.minute),
			(uint16_t)(second - rhs.second),
			(uint16_t)(millisecond - rhs.millisecond)
		};
	}

	void date::operator-=(datespan rhs) noexcept
	{

	}

	void date::operator+=(datespan rhs) noexcept
	{

	}

	bool date::operator<(date rhs) const noexcept
	{

	}

	bool date::operator<=(date rhs) const noexcept
	{

	}

	bool date::operator>(date rhs) const noexcept
	{

	}

	bool date::operator>=(date rhs) const noexcept
	{

	}

	bool date::operator==(date rhs) const noexcept
	{

	}

	bool date::operator!=(date rhs) const noexcept
	{

	}
	*/

	/*
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
		TzSpecificLocalTimeToSystemTimeEx(&time_info.timezone, reinterpret_cast<const SYSTEMTIME*>(&local), reinterpret_cast<SYSTEMTIME*>(this));
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
		SystemTimeToTzSpecificLocalTimeEx(&time_info.timezone, reinterpret_cast<const SYSTEMTIME*>(&utc), reinterpret_cast<SYSTEMTIME*>(this));
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
	*/
}
