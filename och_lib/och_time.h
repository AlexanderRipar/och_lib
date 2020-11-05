#pragma once

#include <cstdint>
#include "och_memrun.h"

namespace och
{
	void init_time();

	struct local_date;

	struct date;

	struct timediff
	{
		uint64_t val;

		uint64_t nanoseconds() const noexcept { return val * 100llu; }

		uint64_t microseconds() const noexcept { return val / 10llu; }

		uint64_t milliseconds() const noexcept { return val / 10'000llu; }

		uint64_t seconds() const noexcept { return val / 10'000'000llu; }

		uint64_t hours() const noexcept { return val / 600'000'000llu; }

		uint64_t days() const noexcept { return val / 14'400'000'000llu; }
	};

	struct timepoint
	{
		uint64_t val;

		timepoint() noexcept {}

		timepoint(const date& utc) noexcept;

		static timepoint now() noexcept;

		timediff since_epoch() const noexcept;

		timediff operator-(timepoint rhs) const noexcept;
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

		date() noexcept {}

		date(const local_date& local) noexcept;

		date(timepoint tp) noexcept;

		static date now() noexcept;

		bool operator<(const date& rhs) const noexcept
		{
			return year < rhs.year || month < rhs.month || monthday < rhs.monthday || hour < rhs.hour || minute < rhs.minute || second < rhs.second || millisecond < rhs.millisecond;
		}

		bool operator>(const date& rhs) const noexcept
		{
			return year > rhs.year || month > rhs.month || monthday > rhs.monthday || hour > rhs.hour || minute > rhs.minute || second > rhs.second || millisecond > rhs.millisecond;
		}

		bool operator<=(const date& rhs) const noexcept
		{
			return !operator>(rhs);
		}

		bool operator>=(const date& rhs) const noexcept
		{
			return !operator<(rhs);
		}

		bool operator==(const date& rhs) const noexcept
		{
			return year == rhs.year && month == rhs.month && monthday == rhs.monthday && hour == rhs.hour && minute == rhs.minute && second == rhs.second && millisecond == rhs.millisecond;
		}

		bool operator!=(const date& rhs) const noexcept
		{
			return !operator==(rhs);
		}
	};

	struct local_date
	{
		uint16_t year;
		uint16_t month;
		uint16_t weekday;
		uint16_t monthday;
		uint16_t hour;
		uint16_t minute;
		uint16_t second;
		uint16_t millisecond;

		local_date() noexcept {}

		local_date(const date& utc) noexcept;

		local_date(timepoint tp) noexcept;

		static local_date now() noexcept;

		bool operator<(const local_date& rhs) const noexcept
		{
			return year < rhs.year || month < rhs.month || monthday < rhs.monthday || hour < rhs.hour || minute < rhs.minute || second < rhs.second || millisecond < rhs.millisecond;
		}

		bool operator>(const local_date& rhs) const noexcept
		{
			return year > rhs.year || month > rhs.month || monthday > rhs.monthday || hour > rhs.hour || minute > rhs.minute || second > rhs.second || millisecond > rhs.millisecond;
		}

		bool operator<=(const local_date& rhs) const noexcept
		{
			return !operator>(rhs);
		}

		bool operator>=(const local_date& rhs) const noexcept
		{
			return !operator<(rhs);
		}

		bool operator==(const local_date& rhs) const noexcept
		{
			return year == rhs.year && month == rhs.month && monthday == rhs.monthday && hour == rhs.hour && minute == rhs.minute && second == rhs.second && millisecond == rhs.millisecond;
		}

		bool operator!=(const local_date& rhs) const noexcept
		{
			return !operator==(rhs);
		}
	};

	struct highres_timepoint
	{
		uint64_t val;

		highres_timepoint operator-(highres_timepoint rhs) const noexcept { return { val - rhs.val }; }

		void operator-=(highres_timepoint rhs) noexcept { val -= rhs.val; }

		bool operator<(highres_timepoint rhs) const noexcept { return val < rhs.val; }

		bool operator>(highres_timepoint rhs) const noexcept { return val > rhs.val; }

		bool operator<=(highres_timepoint rhs) const noexcept { return val <= rhs.val; }

		bool operator>=(highres_timepoint rhs) const noexcept { return val >= rhs.val; }

		bool operator==(highres_timepoint rhs) const noexcept { return val == rhs.val; }

		bool operator!=(highres_timepoint rhs) const noexcept { return val != rhs.val; }

		static highres_timepoint now() noexcept;
	};
}
