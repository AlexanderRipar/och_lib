#include "och_lib.h"

#include <Windows.h>

int main()
{
	SetConsoleOutputCP(65001);

	//och::filehandle outfile("testout.txt", och::fio::access_readwrite, och::fio::open_truncate, och::fio::open_normal);
	//
	//och::time t = och::time::now();
	//
	//och::print(u8"{}\n", "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");

	int32_t i = -34;

	och::print("{:X}\n", i);
	
	//och::print(u8"{}\n", U'😂');
	//
	//och::print(u8"[{:u64}]\n", och::date::local_now());
	//
	//och::print(u8"[{}]\n", och::date::local_now());
	//
	//och::print(u8"[{:u64}]\n", och::date::utc_now());
	//
	//och::timespan s = t - och::time::now();
	//
	//och::print(u8"[{:μ}]]\n", s);
}
