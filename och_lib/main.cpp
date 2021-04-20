#include "och_lib.h"

int main()
{
	const char* fmt_str = "|{0:{1}+}|\n";

	och::print(fmt_str, 0x34ABCD, 10, 1);

	char buf[256];

	och::sprint(buf, fmt_str, 0x34ABCD, 10, 1);

	och::print(buf);
}
