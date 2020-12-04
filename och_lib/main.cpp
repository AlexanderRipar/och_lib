#include "och_lib.h"

#include "tests.h"

#include <Windows.h>

int main()
{
	och::timespan offset = och::timezone_offset();

	och::print("{}\n", offset.val);

	och::time utc_now = och::time::now();

	och::time loc_now = utc_now - offset;

	och::print("utc: {}\nloc: {}\n", utc_now.val, loc_now.val);

	och::time_info loc_date(loc_now);
	och::time_info utc_date(utc_now);

	och::print("utc: {}-{:2>~0}-{:2>~0} {:2>~0}:{:2>~0}.{:2>~0}.{:4>~0}\n", utc_date.year, utc_date.month, utc_date.monthday, utc_date.hour, utc_date.minute, utc_date.second, utc_date.millisecond);
	och::print("loc: {}-{:2>~0}-{:2>~0} {:2>~0}:{:2>~0}.{:2>~0}.{:4>~0}\n", loc_date.year, loc_date.month, loc_date.monthday, loc_date.hour, loc_date.minute, loc_date.second, loc_date.millisecond);
}
