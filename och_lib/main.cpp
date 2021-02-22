#include "och_lib.h"

#include <Windows.h>

#include <string_view>

int main()
{
	och::filehandle file("testout.txt", och::fio::access_write, och::fio::open_truncate, och::fio::open_fail);
	
	//, INFINITY);//

	constexpr och::utf8_view view(u8"a");

	constexpr och::utf8_view v = view;

	//och::print(file.handle, u8"Numberwang: |{:8.7>~😂}|", u8"ab😂cd");
}
