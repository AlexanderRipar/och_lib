#include "och_lib.h"

int main()
{
	och::filehandle file("testout.txt", och::fio::access_write, och::fio::open_truncate, och::fio::open_normal);
	
	och::date now = och::date::local_now();

	och::print(file, u8"{:xYMDTIJK.LuU}", now);

	//och::print(file.handle, u8"Numberwang: |{:8.2>~😂}|, {}", u8"ab😂cd", -300);
}
