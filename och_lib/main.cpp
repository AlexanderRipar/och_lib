#include "och_lib.h"

#include <cstdio>

int main()
{
	och::filehandle outfile("testout.txt", och::fio::access_readwrite, och::fio::open_truncate, och::fio::open_normal);

	och::time t = och::time::now();

	och::print(outfile, u8"{}\n", U'😂');

	och::print(outfile, u8"{:MIN}\n", och::time::now() - t);
}
