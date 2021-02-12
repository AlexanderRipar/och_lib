#pragma once

#include <cstdint>

namespace och
{
	struct date;

	struct timespan
	{
		int64_t val;

		timespan operator+(timespan rhs) const noexcept { return { val + rhs.val }; }

		timespan operator-(timespan rhs) const noexcept { return { val - rhs.val }; }

		void operator+=(timespan rhs) noexcept { val += rhs.val; }

		void operator-=(timespan rhs) noexcept { val -= rhs.val; }

		bool operator<(timespan rhs) const noexcept { return val < rhs.val; }

		bool operator<=(timespan rhs) const noexcept { return val <= rhs.val; }

		bool operator>(timespan rhs) const noexcept { return val > rhs.val; }

		bool operator>=(timespan rhs) const noexcept { return val >= rhs.val; }

		bool operator==(timespan rhs) const noexcept { return val == rhs.val; }

		bool operator!=(timespan rhs) const noexcept { return val != rhs.val; }

		uint64_t microseconds() const noexcept { return val / 10llu; }

		uint64_t milliseconds() const noexcept { return val / 10000llu; }

		uint64_t seconds() const noexcept { return val / 10000000llu; }

		uint64_t minutes() const noexcept { return val / 600000000llu; }

		uint64_t hour() const noexcept { return val / (60 * 600000000llu); }

		uint64_t days() const noexcept { return val / (24 * 60 * 600000000llu); }
	};

	struct time
	{
		uint64_t val;

		time() = default;

		time(uint64_t val) : val{ val } {}

		explicit time(const date& date) noexcept;

		static time now() noexcept;

		timespan operator-(time rhs) const noexcept { return { static_cast<int64_t>(val - rhs.val) }; }

		timespan operator+(timespan rhs) const noexcept { return { static_cast<int64_t>(val + rhs.val) }; }

		timespan operator-(timespan rhs) const noexcept { return { static_cast<int64_t>(val - rhs.val) }; }

		void operator+=(timespan rhs) noexcept { val += rhs.val; }

		void operator-=(timespan rhs) noexcept { val -= rhs.val; }

		bool operator<(time rhs) const noexcept { return val < rhs.val; }

		bool operator<=(time rhs) const noexcept { return val <= rhs.val; }

		bool operator>(time rhs) const noexcept { return val > rhs.val; }

		bool operator>=(time rhs) const noexcept { return val >= rhs.val; }

		bool operator==(time rhs) const noexcept { return val == rhs.val; }

		bool operator!=(time rhs) const noexcept { return val != rhs.val; }
	};

	struct date
	{
		uint16_t year;
		uint16_t month;
		uint16_t weekday;
		uint16_t monthday;
		uint16_t hour;
		uint16_t minute;
		uint16_t second;
		uint16_t millisecond;

		date() = default;

		date(uint16_t year, uint16_t month, uint16_t weekday, uint16_t monthday, uint16_t hour, uint16_t minute, uint16_t second, uint16_t millisecond) noexcept;

		date(time t) noexcept;

		static date utc_now() noexcept;

		static date local_now() noexcept;
	};

	struct highres_timespan
	{
		int64_t val;

		highres_timespan operator+(highres_timespan rhs) const noexcept { return { val + rhs.val }; }

		highres_timespan operator-(highres_timespan rhs) const noexcept { return { val - rhs.val }; }

		void operator+=(highres_timespan rhs) noexcept { val += rhs.val; }

		void operator-=(highres_timespan rhs) noexcept { val -= rhs.val; }

		bool operator<(highres_timespan rhs) const noexcept { return val < rhs.val; }

		bool operator<=(highres_timespan rhs) const noexcept { return val <= rhs.val; }

		bool operator>(highres_timespan rhs) const noexcept { return val > rhs.val; }

		bool operator>=(highres_timespan rhs) const noexcept { return val >= rhs.val; }

		bool operator==(highres_timespan rhs) const noexcept { return val == rhs.val; }

		bool operator!=(highres_timespan rhs) const noexcept { return val != rhs.val; }

		uint64_t nanoseconds() const noexcept;

		uint64_t microseconds() const noexcept;

		uint64_t milliseconds() const noexcept;

		uint64_t seconds() const noexcept;
	};

	struct highres_time
	{
		uint64_t val;

		static highres_time now() noexcept;

		highres_timespan operator-(highres_time rhs) const noexcept { return { static_cast<int64_t>(val - rhs.val) }; }

		highres_timespan operator+(highres_timespan rhs) const noexcept { return { static_cast<int64_t>(val + rhs.val) }; }

		highres_timespan operator-(highres_timespan rhs) const noexcept { return { static_cast<int64_t>(val - rhs.val) }; }

		void operator+=(highres_timespan rhs) noexcept { val += rhs.val; }

		void operator-=(highres_timespan rhs) noexcept { val -= rhs.val; }

		bool operator<(highres_time rhs) const noexcept { return val < rhs.val; }

		bool operator<=(highres_time rhs) const noexcept { return val <= rhs.val; }

		bool operator>(highres_time rhs) const noexcept { return val > rhs.val; }

		bool operator>=(highres_time rhs) const noexcept { return val >= rhs.val; }

		bool operator==(highres_time rhs) const noexcept { return val == rhs.val; }

		bool operator!=(highres_time rhs) const noexcept { return val != rhs.val; }
	};

	timespan timezone_bias() noexcept;
}
