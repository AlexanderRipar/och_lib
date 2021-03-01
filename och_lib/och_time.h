#pragma once

#include <cstdint>

namespace och
{
	struct date;

	struct timespan
	{
		int64_t val;

		timespan operator+(timespan rhs) const noexcept;

		timespan operator-(timespan rhs) const noexcept;

		void operator+=(timespan rhs) noexcept;

		void operator-=(timespan rhs) noexcept;

		bool operator<(timespan rhs) const noexcept;

		bool operator<=(timespan rhs) const noexcept;

		bool operator>(timespan rhs) const noexcept;

		bool operator>=(timespan rhs) const noexcept;

		bool operator==(timespan rhs) const noexcept;

		bool operator!=(timespan rhs) const noexcept;

		int64_t microseconds() const noexcept;

		int64_t milliseconds() const noexcept;

		int64_t seconds() const noexcept;

		int64_t minutes() const noexcept;

		int64_t hours() const noexcept;

		int64_t days() const noexcept;
	};

	struct time
	{
		uint64_t val;

		time() = default;

		time(uint64_t val) : val{ val } {}

		explicit time(const date& date) noexcept;

		static time now() noexcept;

		timespan operator-(time rhs) const noexcept;

		time operator+(timespan rhs) const noexcept;

		time operator-(timespan rhs) const noexcept;

		void operator+=(timespan rhs) noexcept;

		void operator-=(timespan rhs) noexcept;

		bool operator<(time rhs) const noexcept;

		bool operator<=(time rhs) const noexcept;

		bool operator>(time rhs) const noexcept;

		bool operator>=(time rhs) const noexcept;

		bool operator==(time rhs) const noexcept;

		bool operator!=(time rhs) const noexcept;
	};

	struct date
	{
		uint16_t m_year;
		uint16_t m_month;
		uint16_t m_weekday;//Has local utc-offset encoded in high bits(4-15); If only highest of these bits is set, date is in utc
		uint16_t m_monthday;
		uint16_t m_hour;
		uint16_t m_minute;
		uint16_t m_second;
		uint16_t m_millisecond;

		date() = default;

		date(uint16_t year, uint16_t month, uint16_t weekday, uint16_t monthday, uint16_t hour, uint16_t minute, uint16_t second, uint16_t millisecond) noexcept;

		date(time t) noexcept;

		bool utc_offset_is_negative() const noexcept;
		
		uint16_t utc_offset_minutes() const noexcept;
		
		uint16_t utc_offset_hours() const noexcept;

		bool is_utc() const noexcept;

		uint16_t year() const noexcept;
		
		uint16_t month() const noexcept;
		
		uint16_t weekday() const noexcept;

		uint16_t monthday() const noexcept;

		uint16_t hour() const noexcept;
		
		uint16_t minute() const noexcept;
		
		uint16_t second() const noexcept;
		
		uint16_t millisecond() const noexcept;
		
		static date utc_now() noexcept;

		static date local_now() noexcept;
	};

	struct highres_timespan
	{
		int64_t val;

		highres_timespan operator+(highres_timespan rhs) const noexcept;

		highres_timespan operator-(highres_timespan rhs) const noexcept;

		void operator+=(highres_timespan rhs) noexcept;

		void operator-=(highres_timespan rhs) noexcept;

		bool operator<(highres_timespan rhs) const noexcept;

		bool operator<=(highres_timespan rhs) const noexcept;

		bool operator>(highres_timespan rhs) const noexcept;

		bool operator>=(highres_timespan rhs) const noexcept;

		bool operator==(highres_timespan rhs) const noexcept;

		bool operator!=(highres_timespan rhs) const noexcept;

		int64_t nanoseconds() const noexcept;

		int64_t microseconds() const noexcept;

		int64_t milliseconds() const noexcept;

		int64_t seconds() const noexcept;
	};

	struct highres_time
	{
		uint64_t val;

		static highres_time now() noexcept;

		highres_timespan operator-(highres_time rhs) const noexcept { return { static_cast<int64_t>(val - rhs.val) }; }

		highres_time operator+(highres_timespan rhs) const noexcept;

		highres_time operator-(highres_timespan rhs) const noexcept;

		void operator+=(highres_timespan rhs) noexcept;

		void operator-=(highres_timespan rhs) noexcept;

		bool operator<(highres_time rhs) const noexcept;

		bool operator<=(highres_time rhs) const noexcept;

		bool operator>(highres_time rhs) const noexcept;

		bool operator>=(highres_time rhs) const noexcept;

		bool operator==(highres_time rhs) const noexcept;

		bool operator!=(highres_time rhs) const noexcept;
	};

	timespan timezone_bias() noexcept;
}
