#include "och_lib.h"

#include <Windows.h>

int main()
{
	och::filehandle file("testout.txt", och::fio::access_write, och::fio::open_truncate, och::fio::open_fail);
	
	och::print(file.handle, u8"Numberwang: |{:8.7>~😂}|", u8"ab😂cd");
}
