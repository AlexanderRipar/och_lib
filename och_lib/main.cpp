#include "och_lib.h"

#include<limits>

#include <cstdio>

void test_float_print(int32_t precision)
{
	constexpr int32_t total_files = 256;

	constexpr uint64_t floats_per_file = 0x1'0000'0000ull / total_files;

	och::print("Running...\n\n");

	char std_format[32];

	och::sprint(std_format, "%.{}f", precision);

	char och_format[32];

	och::sprint(och_format, "{{:.{}}", precision);

	char std_format_ext[32];

	och::sprint(std_format_ext, "%.{}f", precision + 1);

	char och_format_ext[32];

	och::sprint(och_format_ext, "{{:.{}}", precision + 1);

	och::print("Formats: \"{}\", \"{}\"\nExt. Formats: \"{}\", \"{}\"\n\n", std_format, och_format, std_format_ext, och_format_ext);

	for (uint64_t file = 0; file != total_files; ++file)
	{
		och::print("0x{:8>~0X} - 0x{:8>~0X}...\n", file * floats_per_file, (file + 1) * floats_per_file - 1);

		uint64_t errcnt = 0, std_lo = 0, std_hi = 0;

		for(uint64_t i = file * floats_per_file; i != (file + 1) * floats_per_file; ++i)
		{
			if ((i & 0x7F80'0000) == 0x7F80'0000)
				continue;

			float f;

			memcpy(&f, &i, 4);

			char och_buf[256];

			och::sprint(och_buf, och_format, f);

			char std_buf[256];

			sprintf_s(std_buf, std_format, f);

			bool is_equal = true;

			for (int j = 0; j != 256; ++j)
				if (och_buf[j] != std_buf[j])
				{
					if (och_buf[j + 1] == '\0' && std_buf[j + 1] == '\0' && och_buf[j] == std_buf[j] + 1)
						if (och_buf[j] == std_buf[j] + 1)
						{
							++std_lo;

							break;
						}
						else if (och_buf[j] == std_buf[j] - 1)
						{
							++std_hi;

							break;
						}

					is_equal = false;
				}
				else if (och_buf[j] == '\0')
					break;

			if (!is_equal)
			{
				if (errcnt < 8)
					och::print("\noch={}\nstd={}\n", och_buf, std_buf);

				++errcnt;
			}

			++i;
		}

		och::print("Err: {}; StdLo: {}; StdHi: {}\n\n", errcnt, std_lo, std_hi);
	}
}

void print_bits_as_float(uint32_t bpat)
{
	float f;

	memcpy(&f, &bpat, 4);

	och::print("OCH: {:.30}\n", f);

	printf("STD: %.30f\n\n", f);
}

void load_floats(int beg = 0)
{
	och::filehandle floats("float_tests\\float_test49.txt", och::fio::access_read, och::fio::open_normal, och::fio::open_fail);
	
	if (!floats)
	{
		och::print("Could not open file...\n");

		return;
	}

	uint32_t fs[256];
	
	floats.seek(beg * sizeof(float), och::fio::setptr_beg);

	och::range in = floats.read(och::range(fs));
	
	och::print("Loaded {} floats\n", in.len());

	for (uint32_t i : in)
		print_bits_as_float(i);
}

int main()
{
	test_float_print(70);

	//load_floats();

	//float f = 0.000000000000000006144265330000F;
	//
	//uint32_t n;
	//
	//memcpy(&n, &f, 4);
	//
	//och::print("{:8>~0X}\n", n);
	//
	//printf("%.70f\n", 0.0000000000000000000000000000000000000000003363116314379560970216951000F);
	//
	//och::print("{:.70}\n", 0.0000000000000000000000000000000000000000003363116314379560970216951000F);

	//print_bits_as_float(0x3505AF2D - 1);

	//print_bits_as_float(0x3505AF2D);

	//print_bits_as_float(0x3505AF2D + 1);
}
