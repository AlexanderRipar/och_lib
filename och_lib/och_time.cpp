#include "och_time.h"

#include "och_fmt.h"
#include <Windows.h>

namespace och
{
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*//////////////////////////////////////////////////////internals////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	// Global internal needed by time-function.
	// These must be determined at runtime, so this global struct,
	// initialized on startup does the job just fine.
	struct
	{
		const int32_t tz_bias_min{ query_tz_bias_min() };
		const uint16_t tz_bias_hr_min{ format_tz_bias_hr_min() };
		const int64_t tz_bias_100ns{ tz_bias_min * 600000000llu };
		const uint64_t highres_clock_ticks_per_second{ query_highres_clock_ticks_per_second() };

		int32_t query_tz_bias_min() const noexcept
		{
			DYNAMIC_TIME_ZONE_INFORMATION tz;

			GetDynamicTimeZoneInformation(&tz);

			return tz.Bias;
		}

		uint16_t format_tz_bias_hr_min() const noexcept
		{
			uint16_t val = 0;

			int32_t bias = tz_bias_min;

			if (bias > 0)
				val |= 0x8000;
			else
				bias = -bias;
				
			val |= (bias / 60) << 10;
			val |= (bias % 60) << 4;

			return val;
		}

		uint64_t query_highres_clock_ticks_per_second() const noexcept
		{
			uint64_t freq = 1;

			QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&freq));

			return freq;
		}
	}
	time_data;

	timespan timezone_bias() noexcept
	{
		return{ time_data.tz_bias_100ns };
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*//////////////////////////////////////////////////////timespan/////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	timespan timespan::operator+(timespan rhs) const noexcept
	{
		return timespan{ val + rhs.val };
	}

	timespan timespan::operator-(timespan rhs) const noexcept
	{
		return timespan{ val - rhs.val };
	}

	void timespan::operator+=(timespan rhs) noexcept
	{
		val += rhs.val;
	}

	void timespan::operator-=(timespan rhs) noexcept
	{
		val -= rhs.val;
	}

	bool timespan::operator<(timespan rhs) const noexcept
	{
		return val < rhs.val;
	}

	bool timespan::operator<=(timespan rhs) const noexcept
	{
		return val <= rhs.val;
	}

	bool timespan::operator>(timespan rhs) const noexcept
	{
		return val > rhs.val;
	}

	bool timespan::operator>=(timespan rhs) const noexcept
	{
		return val >= rhs.val;
	}

	bool timespan::operator==(timespan rhs) const noexcept
	{
		return val == rhs.val;
	}

	bool timespan::operator!=(timespan rhs) const noexcept
	{
		return val != rhs.val;
	}

	int64_t timespan::microseconds() const noexcept
	{
		return val / 10llu;
	}

	int64_t timespan::milliseconds() const noexcept
	{
		return val / 10000llu;
	}

	int64_t timespan::seconds() const noexcept
	{
		return val / 10000000llu;
	}

	int64_t timespan::minutes() const noexcept
	{
		return val / 600000000llu;
	}

	int64_t timespan::hours() const noexcept
	{
		return val / (60 * 600000000llu);
	}

	int64_t timespan::days() const noexcept
	{
		return val / (24 * 60 * 600000000llu);
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*////////////////////////////////////////////////////////time///////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	time::time(const date& d) noexcept
	{
		date pure = d;

		pure.m_weekday &= 7;

		SystemTimeToFileTime(reinterpret_cast<const SYSTEMTIME*>(&d), reinterpret_cast<FILETIME*>(this));
	}

	time time::now() noexcept
	{
		time t;

		GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&t));

		return t;
	}

	timespan time::operator-(time rhs) const noexcept {
		return timespan{ static_cast<int64_t>(val - rhs.val) };
	}

	time time::operator+(timespan rhs) const noexcept
	{
		return time{val + rhs.val };
	}

	time time::operator-(timespan rhs) const noexcept
	{
		return time{ val - rhs.val };
	}

	void time::operator+=(timespan rhs) noexcept
	{
		val += rhs.val;
	}

	void time::operator-=(timespan rhs) noexcept
	{
		val -= rhs.val;
	}

	bool time::operator<(time rhs) const noexcept
	{
		return val < rhs.val;
	}

	bool time::operator<=(time rhs) const noexcept
	{
		return val <= rhs.val;
	}

	bool time::operator>(time rhs) const noexcept
	{
		return val > rhs.val;
	}

	bool time::operator>=(time rhs) const noexcept
	{
		return val >= rhs.val;
	}

	bool time::operator==(time rhs) const noexcept
	{
		return val == rhs.val;
	}

	bool time::operator!=(time rhs) const noexcept
	{
		return val != rhs.val;
	}


	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*////////////////////////////////////////////////////////date///////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	date::date(uint16_t year, uint16_t month, uint16_t weekday, uint16_t monthday, uint16_t hour, uint16_t minute, uint16_t second, uint16_t millisecond) noexcept
		: m_year{ year }, m_month{ month }, m_weekday{ weekday }, m_monthday{ monthday }, m_hour{ hour }, m_minute{ minute }, m_second{ second }, m_millisecond{ millisecond } {}

	date::date(time t) noexcept
	{
		FileTimeToSystemTime(reinterpret_cast<FILETIME*>(&t), reinterpret_cast<SYSTEMTIME*>(this));
	}

	bool date::utc_offset_is_negative() const noexcept
	{
		return m_weekday & 0x8000;
	}

	uint16_t date::utc_offset_minutes() const noexcept
	{
		return (m_weekday & 0x03F0) >> 4;
	}

	uint16_t date::utc_offset_hours() const noexcept
	{
		return (m_weekday & 0x7C00) >> 10;
	}

	bool date::is_utc() const noexcept
	{
		return (m_weekday >> 4) == 0x0800;
	}

	uint16_t date::year() const noexcept
	{
		return m_year;
	}

	uint16_t date::month() const noexcept
	{
		return m_month;
	}

	uint16_t date::weekday() const noexcept
	{
		return m_weekday & 7;
	}

	uint16_t date::monthday() const noexcept
	{
		return m_monthday;
	}

	uint16_t date::hour() const noexcept
	{
		return m_hour;
	}

	uint16_t date::minute() const noexcept
	{
		return m_minute;
	}

	uint16_t date::second() const noexcept
	{
		return m_second;
	}

	uint16_t date::millisecond() const noexcept
	{
		return m_millisecond;
	}

	date date::utc_now() noexcept
	{
		date date;

		GetSystemTime(reinterpret_cast<SYSTEMTIME*>(&date));

		date.m_weekday |= 0x8000;

		return date;
	}

	date date::local_now() noexcept
	{
		date date;

		GetLocalTime(reinterpret_cast<SYSTEMTIME*>(&date));

		date.m_weekday |= time_data.tz_bias_hr_min;

		return date;
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*//////////////////////////////////////////////////highres_timespan/////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	int64_t highres_timespan::nanoseconds() const noexcept
	{
		return val * 1000'000'000i64 / time_data.highres_clock_ticks_per_second;
	}

	int64_t highres_timespan::microseconds() const noexcept
	{
		return val * 1000'000i64 / time_data.highres_clock_ticks_per_second;
	}

	int64_t highres_timespan::milliseconds() const noexcept
	{
		return val * 1000i64 / time_data.highres_clock_ticks_per_second;
	}

	int64_t highres_timespan::seconds() const noexcept
	{
		return val / time_data.highres_clock_ticks_per_second;
	}

	highres_timespan highres_timespan::operator+(highres_timespan rhs) const noexcept
	{
		return highres_timespan{ val + rhs.val };
	}

	highres_timespan highres_timespan::operator-(highres_timespan rhs) const noexcept
	{
		return highres_timespan{ val - rhs.val };
	}

	void highres_timespan::operator+=(highres_timespan rhs) noexcept
	{
		val += rhs.val;
	}

	void highres_timespan::operator-=(highres_timespan rhs) noexcept
	{
		val -= rhs.val;
	}

	bool highres_timespan::operator<(highres_timespan rhs) const noexcept
	{
		return val < rhs.val;
	}

	bool highres_timespan::operator<=(highres_timespan rhs) const noexcept
	{
		return val <= rhs.val;
	}

	bool highres_timespan::operator>(highres_timespan rhs) const noexcept
	{
		return val > rhs.val;
	}

	bool highres_timespan::operator>=(highres_timespan rhs) const noexcept
	{
		return val >= rhs.val;
	}

	bool highres_timespan::operator==(highres_timespan rhs) const noexcept
	{
		return val == rhs.val;
	}

	bool highres_timespan::operator!=(highres_timespan rhs) const noexcept
	{
		return val != rhs.val;
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*////////////////////////////////////////////////////highres_time///////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	highres_time highres_time::now() noexcept
	{
		highres_time t;

		QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&t));

		return t;
	}

	highres_timespan highres_time::operator-(highres_time rhs) const noexcept
	{
		return highres_timespan{ static_cast<int64_t>(val - rhs.val) };
	}

	highres_time highres_time::operator+(highres_timespan rhs) const noexcept
	{
		return highres_time{ val + rhs.val };
	}

	highres_time highres_time::operator-(highres_timespan rhs) const noexcept
	{
		return highres_time{ val - rhs.val };
	}

	void highres_time::operator+=(highres_timespan rhs) noexcept
	{
		val += rhs.val;
	}

	void highres_time::operator-=(highres_timespan rhs) noexcept
	{
		val -= rhs.val;
	}

	bool highres_time::operator<(highres_time rhs) const noexcept
	{
		return val < rhs.val;
	}

	bool highres_time::operator<=(highres_time rhs) const noexcept
	{
		return val <= rhs.val;
	}

	bool highres_time::operator>(highres_time rhs) const noexcept
	{
		return val > rhs.val;
	}

	bool highres_time::operator>=(highres_time rhs) const noexcept
	{
		return val >= rhs.val;
	}

	bool highres_time::operator==(highres_time rhs) const noexcept
	{
		return val == rhs.val;
	}

	bool highres_time::operator!=(highres_time rhs) const noexcept
	{
		return val != rhs.val;
	}
}
