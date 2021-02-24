#include "och_lib.h"

#include <cstdio>

int main()
{
	och::print("{1} ||||| {}", 3, och::date::local_now());
}
