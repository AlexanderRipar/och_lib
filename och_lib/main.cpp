#include "och_lib.h"

#include <cstdio>

int main()
{
	och::print("{1} ||||| {0}\n", 3, och::date::local_now(), och::utf8_char(U'😂'));
}
