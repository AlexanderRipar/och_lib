#pragma once

#include <cstdint>
#include "och_memrun.h"

namespace och
{
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
	};

	struct time
	{
		uint64_t val;

		static time now() noexcept;

		time operator+(timespan rhs) const noexcept;

		time operator-(timespan rhs) const noexcept;

		timespan operator+(time rhs) const noexcept;

		timespan operator-(time rhs) const noexcept;

		void operator+=(timespan rhs) noexcept;

		void operator-=(timespan rhs) noexcept;

		bool operator<(time rhs) const noexcept;

		bool operator<=(time rhs) const noexcept;

		bool operator>(time rhs) const noexcept;

		bool operator>=(time rhs) const noexcept;

		bool operator==(time rhs) const noexcept;

		bool operator!=(time rhs) const noexcept;
	};

	struct time_info
	{
		uint16_t year;
		uint16_t month;
		uint16_t weekday;
		uint16_t monthday;
		uint16_t hour;
		uint16_t minute;
		uint16_t second;
		uint16_t millisecond;

		time_info() = default;

		time_info(uint16_t year, uint16_t month, uint16_t weekday, uint16_t monthday, uint16_t hour, uint16_t minute, uint16_t second, uint16_t millisecond) noexcept;

		time_info(time t) noexcept;

		static time_info utc_now() noexcept;

		static time_info local_now() noexcept;
	};

	timespan timezone_offset() noexcept;
}
