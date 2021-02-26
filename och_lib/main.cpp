#include "och_lib.h"

#include <cstdio>

#include <intrin.h>

int main()
{
	och::filehandle outfile("testout.txt", och::fio::access_readwrite, och::fio::open_truncate, och::fio::open_normal);

	och::time t = och::time::now();

	och::print(outfile, u8"{}\n", "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

	och::print(outfile, "[{}]\n", -34);
	
	och::print(outfile, u8"{}\n", U'😂');
	
	och::print(outfile, u8"[{:u}]\n", och::date::local_now());
	
	och::print(outfile, u8"[{}]\n", och::date::local_now());

	och::print(outfile, u8"[{:u}]\n", och::date::utc_now());

	och::timespan s = och::time::now() - t;

	och::print(outfile, u8"[{}]]\n", s);
}
