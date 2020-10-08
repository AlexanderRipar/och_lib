#include "och_lib.h"

#include <chrono>

int main()
{
	uint64_t t1;

	{
		std::chrono::steady_clock::time_point beg = std::chrono::steady_clock::now();

		for (int i = 0; i < 1024; ++i)
		{
			printf("Hello %d\n", i);
		}

		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		t1 = std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count();
	}

	uint64_t t0;

	{
		std::chrono::steady_clock::time_point beg = std::chrono::steady_clock::now();

		for (int i = 0; i < 1024; ++i)
		{
			och::print("Hello {}\n", i);
		}

		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

		t0 = std::chrono::duration_cast<std::chrono::milliseconds>(end - beg).count();
	}

	och::print("\noch::print: {:7>} ms\n"
		         "printf:     {:7>} ms\n", t0, t1);
}
