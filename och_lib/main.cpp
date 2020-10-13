#include "och_lib.h"

#include <array>

#include "tests.h"

int main()
{
	och::filehandle file("Test", och::fio::access_read, och::fio::open_normal, och::fio::open_fail);

	char buf[256];

	file.read(buf);

	std::array<char, 256> arr;

	/*och::iohandle file = och::open_file("testfile.txt", och::fio::access_read, och::fio::open_normal, och::fio::open_fail);

	if (!file._ptr)
		return -1;

	char dat[8];

	och::memrun buf(dat);

	for(och::memrun res = och::read_from_file(file, buf); res.len(); res = och::read_from_file(file, buf))
		och::print("{}\n", res);
	
	/*
	och::timer t;

	for (int i = 0; i < 1024; ++i) printf("Hello\n");

	uint64_t t1 = t.ms();
	

	t.restart();

	for (int i = 0; i < 1024; ++i) och::print("Hello\n");

	uint64_t t0 = t.ms();
	

	och::print("\noch::print  {:7>} ms\n"
		         "printf      {:7>} ms\n", t0, t1);
	*/
}
