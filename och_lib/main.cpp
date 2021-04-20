#include "och_lib.h"

int main()
{
	const char* fmt_str = "|{0:40u}|\n|{1:40u}|\n";

	och::date loc_now = och::date::local_now();
	och::date utc_now = och::date::utc_now();

	och::print(fmt_str, loc_now, utc_now, 20);

	char buf[256];

	och::sprint(buf, fmt_str, loc_now, utc_now, 20);

	och::print(buf);
}
