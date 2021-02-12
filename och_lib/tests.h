#pragma once

#include "och_lib.h"

#define OCH_TEST(test) if(!(test)){ test_ret = false; och::print("\n" #test); }

namespace och
{
	bool test_fmt()
	{
		bool test_ret = true;

		och::tempfilehandle file;

		OCH_TEST(file.handle);

		och::string data("This should be written to a test-file");

		file.write(data);

		file.set_fileptr(0);

		char dat[255];

		och::range buf(dat);

		och::range<char> ret = file.read(buf);

		for (int i = 0; i < ret.len(); ++i)
		{
			och::print("{:c}", ret[i]);
			OCH_TEST(ret[i] == data[i]);
		}

		return test_ret;
	}

	bool test()
	{
		bool test_ret = true;

		OCH_TEST(test_fmt());

		och::print("\nFinished Test\n");

		return test_ret;
	}
}

#undef OCH_TEST
