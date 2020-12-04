#include "och_time.h"

#include "och_fmt.h"
#include <Windows.h>

namespace och
{
	//Internals

	struct time_data_t
	{
		const int64_t tz_bias;
		const uint64_t highres_clock_ticks_per_second;

		int64_t query_tz_bias() const noexcept
		{
			DYNAMIC_TIME_ZONE_INFORMATION tz;

			GetDynamicTimeZoneInformation(&tz);

			return tz.Bias * 600000000llu;
		}

		uint64_t query_highres_clock_ticks_per_second() const noexcept
		{
			uint64_t freq = 1;

			QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&freq));

			return freq;
		}

		time_data_t() : tz_bias{ query_tz_bias() }, highres_clock_ticks_per_second{ query_highres_clock_ticks_per_second() } {}
	};

	time_data_t time_data;

	timespan timezone_bias() noexcept { return{ time_data.tz_bias }; }



	//time

	time::time(const time_info& date) noexcept
	{
		SystemTimeToFileTime(reinterpret_cast<const SYSTEMTIME*>(&date), reinterpret_cast<FILETIME*>(this));
	}

	time time::now() noexcept
	{
		time t;

		GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&t));

		return t;
	}



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



	//highres_time

	highres_time::highres_time() noexcept
	{
		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&val));
	}



	//highres_timespan

	uint64_t highres_timespan::nanoseconds() const noexcept
	{
		return val * 1000'000'000 / time_data.highres_clock_ticks_per_second;
	}

	uint64_t highres_timespan::microseconds() const noexcept
	{
		return val * 1000'000 / time_data.highres_clock_ticks_per_second;
	}

	uint64_t highres_timespan::milliseconds() const noexcept
	{
		return val * 1000 / time_data.highres_clock_ticks_per_second;
	}

	uint64_t highres_timespan::seconds() const noexcept
	{
		return val / time_data.highres_clock_ticks_per_second;
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
