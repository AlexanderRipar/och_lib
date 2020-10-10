#include "och_lib.h"

int main()
{
	och::timer t;

	for (int i = 0; i < 1024; ++i) printf("Hello\n");

	uint64_t t1 = t.ms();
	

	t.restart();

	for (int i = 0; i < 1024; ++i) och::print("Hello\n");

	uint64_t t0 = t.ms();
	

	och::print("\noch::print  {:7>} ms\n"
		         "printf      {:7>} ms\n", t0, t1);
}
