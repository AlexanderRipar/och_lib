#include "och_matmath.h"

#include "och_fmt.h"

/*
void time_int_print()
{
	och::print("Running...\n\n");

	for (uint64_t exponent = 0; exponent != 256; ++exponent)
	{
		uint64_t lo_i = exponent << 23, hi_i = (exponent << 23) | 0x7FFFFF;

		float lo_f, hi_f;

		memcpy(&lo_f, &lo_i, 4);

		memcpy(&hi_f, &hi_i, 4);

		och::print("\n\n0x{:2>~0X} ({} - {})", exponent, lo_f, hi_f);

		char buf[256];

		och::timer timer;

		for (uint64_t sign = 0; sign != 2; ++sign)
			for (uint64_t i = lo_i | (sign << 31); i != ((hi_i + 1) | (sign << 31)); ++i)
			{
				if ((i & 0x7F80'0000) == 0x7F80'0000)
					continue;

				och::sprint(buf, "{}", (int32_t)i);
			}

		och::timespan och_time = timer.reset();

		for (uint64_t sign = 0; sign != 2; ++sign)
			for (uint64_t i = lo_i | (sign << 31); i != ((hi_i + 1) | (sign << 31)); ++i)
			{
				if ((i & 0x7F80'0000) == 0x7F80'0000)
					continue;

				sprintf_s(buf, "%d", (int32_t)(i));
			}

		och::timespan std_time = timer.read();

		och::print(", {}, {}", och_time, std_time);
	}
}

void time_float_vs_int_print(int32_t precision)
{
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

	for (uint64_t exponent = 0; exponent != 256; ++exponent)
	{
		uint64_t lo_i = exponent << 23, hi_i = (exponent << 23) | 0x7FFFFF;

		float lo_f, hi_f;

		memcpy(&lo_f, &lo_i, 4);

		memcpy(&hi_f, &hi_i, 4);

		och::print("\n\n0x{:2>~0X} ({} - {})", exponent, lo_f, hi_f);

		char buf[256];

		och::timer timer;

		for (uint64_t sign = 0; sign != 2; ++sign)
			for (uint64_t i = lo_i | (sign << 31); i != ((hi_i + 1) | (sign << 31)); ++i)
			{
				if ((i & 0x7F80'0000) == 0x7F80'0000)
					continue;

				float f;

				memcpy(&f, &i, 4);

				och::sprint(buf, och_format, f);
			}

		och::timespan flt_time = timer.reset();

		for (uint64_t sign = 0; sign != 2; ++sign)
			for (uint64_t i = lo_i | (sign << 31); i != ((hi_i + 1) | (sign << 31)); ++i)
			{
				if ((i & 0x7F80'0000) == 0x7F80'0000)
					continue;

				och::sprint(buf, "{}", i);
			}

		och::timespan int_time = timer.read();

		och::print(", {}, {}", flt_time, int_time);
	}
}

void time_float_print(int32_t precision)
{
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

	for (uint64_t exponent = 0; exponent != 256; ++exponent)
	{
		uint64_t lo_i = exponent << 23, hi_i = (exponent << 23) | 0x7FFFFF;

		float lo_f, hi_f;

		memcpy(&lo_f, &lo_i, 4);

		memcpy(&hi_f, &hi_i, 4);

		och::print("\n\n0x{:2>~0X} ({} - {})", exponent, lo_f, hi_f);

		char buf[256];

		och::timer timer;

		for (uint64_t sign = 0; sign != 2; ++sign)
			for (uint64_t i = lo_i | (sign << 31); i != ((hi_i + 1) | (sign << 31)); ++i)
			{
				if ((i & 0x7F80'0000) == 0x7F80'0000)
					continue;

				float f;

				memcpy(&f, &i, 4);

				och::sprint(buf, och_format, f);
			}

		och::timespan och_time = timer.reset();

		for (uint64_t sign = 0; sign != 2; ++sign)
			for (uint64_t i = lo_i | (sign << 31); i != ((hi_i + 1) | (sign << 31)); ++i)
			{
				if ((i & 0x7F80'0000) == 0x7F80'0000)
					continue;

				float f;

				memcpy(&f, &i, 4);

				sprintf_s(buf, std_format, f);
			}

		och::timespan std_time = timer.read();

		och::print(", {}, {}", och_time, std_time);
	}
}

void test_float_print(int32_t precision)
{
	och::timer function_timer;

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

	for (uint64_t exponent = 0; exponent != 256; ++exponent)
	{
		uint64_t lo_i = exponent << 23, hi_i = (exponent << 23) | 0x7FFFFF;

		float lo_f, hi_f;

		memcpy(&lo_f, &lo_i, 4);

		memcpy(&hi_f, &hi_i, 4);

		och::print("\n\n0x{:2>~0X} ({} - {})...\n", exponent, lo_f, hi_f);

		uint64_t errcnt = 0, std_lo = 0, std_hi = 0;

		for(uint64_t sign = 0; sign != 2; ++sign)
			for (uint64_t i = lo_i | (sign << 31); i != ((hi_i + 1) | (sign << 31)); ++i)
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
			}

		if (errcnt)
			och::print("!!! ERR: {} !!!  ", errcnt);

		if (std_lo)
			och::print("std_lo: {}  ", std_lo);

		if (std_hi)
			och::print("std_hi: {}  ", std_hi);

		if (!(errcnt | std_lo | std_hi))
			och::print("All good");
	}

	och::print("\n\nTotal time taken: {}.\n", function_timer.read());
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
		och::print("Could not open section...\n");

		return;
	}

	uint32_t fs[256];
	
	floats.seek(beg * sizeof(float), och::fio::setptr_beg);

	och::range in = floats.read(och::range(fs));
	
	och::print("Loaded {} floats\n", in.len());

	for (uint32_t i : in)
		print_bits_as_float(i);
}
*/

int main()
{
	//test_float_print(70);

	//time_float_print(70);

	//och::utf8_string str;

	//och::sprint(str, "Hello there {} no. {}.\n", "Michael", 755.3F);

	//och::print(str);

	//time_float_vs_int_print(70);

	//time_int_print();
}
