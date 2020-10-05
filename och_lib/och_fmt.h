#pragma once

#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cmath>

#include "och_memrun.h"

namespace och
{
	constexpr const char f_hex_symbols[2][16]{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	const char* f_curr_hex_symbols = f_hex_symbols[0];

	uint32_t f_default_precision = 4;
	float f_default_precision_factor = 10.0F * 10.0F * 10.0F * 10.0F;

	struct arg
	{
	private:
		uint16_t width = 0;
		uint16_t precision = -1;
		uint16_t flags = 0;
		uint8_t offset;
		char filler = ' ';
	public:
		union
		{
			uint64_t i;
			const char* s;
			double d;
			float f;
			och::ministring m;
		};

		enum class types : uint8_t
		{
			u,	//unsigned integer
			i,	//signed integer
			f,	//float
			d,	//double
			s,	//const char*
			m,	//och::ministring
		};

		void set_width(uint16_t w) { width = w; }
		void set_precision(uint16_t p) { precision = p; }
		void set_rightadj() { flags |= 4; }
		void set_signmode(uint16_t s) { flags |= s; }
		void set_offset(uint8_t o) { offset = 0; }
		void set_filler(char f) { filler = f; }

		uint16_t get_width() { return width; }
		uint16_t get_precision() { return precision; }
		bool get_rightadj() { return flags & 4; }
		uint16_t get_signmode() { return  flags & 3; }
		uint8_t get_offset() { return offset; }
		char get_filler() { return filler; }

		arg(        uint8_t i) : offset{ static_cast<uint8_t>(types::u) }, i{ static_cast<uint64_t>(i) } {}
		arg(       uint16_t i) : offset{ static_cast<uint8_t>(types::u) }, i{ static_cast<uint64_t>(i) } {}
		arg(       uint32_t i) : offset{ static_cast<uint8_t>(types::u) }, i{ static_cast<uint64_t>(i) } {}
		arg(       uint64_t i) : offset{ static_cast<uint8_t>(types::u) }, i{ static_cast<uint64_t>(i) } {}
		arg(         int8_t i) : offset{ static_cast<uint8_t>(types::i) }, i{ static_cast<uint64_t>(i) } {}
		arg(        int16_t i) : offset{ static_cast<uint8_t>(types::i) }, i{ static_cast<uint64_t>(i) } {}
		arg(        int32_t i) : offset{ static_cast<uint8_t>(types::i) }, i{ static_cast<uint64_t>(i) } {}
		arg(        int64_t i) : offset{ static_cast<uint8_t>(types::i) }, i{ static_cast<uint64_t>(i) } {}
		arg(    const char* s) : offset{ static_cast<uint8_t>(types::s) }, s{                       s  } {}
		arg(         double d) : offset{ static_cast<uint8_t>(types::d) }, d{                       d  } {}
		arg(          float f) : offset{ static_cast<uint8_t>(types::f) }, f{                       f  } {}
		arg(och::ministring m) : offset{ static_cast<uint8_t>(types::m) }, m{                       m  } {}
		arg(    och::string m) : offset{ static_cast<uint8_t>(types::m) }, m{                       m  } {}

	};

#define OCH_FMT_RET void
#define OCH_FMT_ARG (arg in, FILE* out)
#define OCH_FMT_FX(name) OCH_FMT_RET name OCH_FMT_ARG

	OCH_FMT_FX(f_uint)
	{
		uint64_t val = in.i;

		char buf[20];
		int i = 19;

		if (!val)
			buf[i--] = '0';

		while (val)
		{
			buf[i] = '0' + val % 10;

			val /= 10;

			--i;
		}

		if (in.get_rightadj())
		{
			for (uint32_t j = 19 - i; j < in.get_width(); ++j)
				putc(in.get_filler(), out);

			fwrite(buf + i + 1, 1, 19llu - i, out);
		}
		else
		{
			fwrite(buf + i + 1, 1, 19llu - i, out);

			for (uint32_t j = 19 - i; j < in.get_width(); ++j)
				putc(in.get_filler(), out);
		}
	}

	OCH_FMT_FX(f_int)
	{
		int64_t val = static_cast<int64_t>(in.i);

		char buf[20];
		int i = 19;

		if (!val)
			buf[i--] = '0';

		bool negative = false;

		if (val < 0)
		{
			negative = true;
			val = -val;
		}

		while (val)
		{
			buf[i--] = '0' + val % 10;

			val /= 10;
		}

		if (negative)
			buf[i--] = '-';
		else if (in.get_signmode() == 1)
			buf[i--] = '+';
		else if (in.get_signmode() == 2)
			buf[i--] = ' ';

		if (in.get_rightadj())
		{
			for (uint32_t j = 19 - i; j < in.get_width(); ++j)
				putc(in.get_filler(), out);

			fwrite(buf + i + 1, 1, 19llu - i, out);
		}
		else
		{
			fwrite(buf + i + 1, 1, 19llu - i, out);

			for (uint32_t j = 19 - i; j < in.get_width(); ++j)
				putc(in.get_filler(), out);
		}
	}

	OCH_FMT_FX(f_cstring)
	{
		size_t len = strlen(in.s);

		if (in.get_precision() != -1 && len > in.get_precision())
			len = in.get_precision();

		if (in.get_rightadj())
		{
			for (size_t j = len; j < in.get_width(); ++j)
				putc(in.get_filler(), out);

			fwrite(in.s, 1, len, out);
		}
		else
		{
			fwrite(in.s, 1, len, out);

			for (size_t j = len; j < in.get_width(); ++j)
				putc(in.get_filler(), out);
		}
	}

	OCH_FMT_FX(f_ministring)
	{
		size_t len = in.m.len();

		if (in.get_precision() != -1 && len > in.get_precision())
			len = in.get_precision();

		if (in.get_rightadj())
		{
			for (size_t j = len; j < in.get_width(); ++j)
				putc(in.get_filler(), out);

			fwrite(in.s, 1, len, out);
		}
		else
		{
			fwrite(in.s, 1, len, out);

			for (size_t j = len; j < in.get_width(); ++j)
				putc(in.get_filler(), out);
		}
	}

	//TODO
	OCH_FMT_FX(f_float)
	{
		constexpr uint32_t bufsize = 32;

		char buf[bufsize];

		uint32_t i = bufsize;

		float f = in.f;

		if (f == INFINITY)
		{
			i -= 3;
			memcpy(buf + 17, "inf", 3);

			if (in.get_signmode() == 1)
				buf[--i] = '+';
			else if (in.get_signmode() == 2)
				buf[--i] = ' ';

		}
		else if (f == -INFINITY)
		{
			i -= 4;
			memcpy(buf + 16, "-inf", 4);
		}
		else
		{
			int64_t int_val = static_cast<int64_t>(f);

			float precision_factor;

			if (in.get_precision() == -1)
			{
				in.set_precision(f_default_precision);
				precision_factor = f_default_precision_factor;
			}
			else
				precision_factor = powf(in.get_precision(), 10.0F);

			uint64_t dec_val = static_cast<uint64_t>((in.f - static_cast<float>(int_val)) * f_default_precision_factor);

			while (dec_val && bufsize - i != in.get_precision())
			{
				buf[--i] = '0' + dec_val % 10;

				dec_val /= 10;
			}

			while (bufsize - i != in.get_precision())
				buf[--i] = '0';

			if (in.get_precision())
				buf[--i] = '.';

			bool negative = false;
			if (f < 0)
			{
				negative = true;
				int_val = -int_val;
			}
			else if (!int_val)
				buf[--i] = '0';
			else
				while (int_val)
				{
					buf[--i] = '0' + int_val % 10;

					int_val /= 10;
				}

			if (negative)
				buf[--i] = '-';
			else if (in.get_signmode() == 1)
				buf[--i] = '+';
			else if (in.get_signmode() == 2)
				buf[--i] = ' ';
		}

		if (in.get_rightadj())
		{
			for (uint32_t j = bufsize - i; j < in.get_width(); ++j)
				putc(in.get_filler(), out);

			fwrite(buf + i, 1, bufsize - i, out);
		}
		else
		{
			fwrite(buf + i, 1, bufsize - i, out);

			for (uint32_t j = bufsize - i; j < in.get_width(); ++j)
				putc(in.get_filler(), out);
		}
	}

	//TODO
	OCH_FMT_FX(f_double)
	{

	}

	//Extended formatmode functions

	OCH_FMT_FX(f_hexadecimal)
	{
		uint64_t val = in.i;

		char buf[16];
		int i = 15;

		if (!val)
			buf[i--] = '0';

		while (val)
		{
			buf[i--] = f_curr_hex_symbols[val & 0xF];

			val >>= 4;
		}

		if (in.get_rightadj())
		{
			for (uint32_t j = 15 - i; j < in.get_width(); ++j)
				putc(in.get_filler(), out);

			fwrite(buf + i + 1, 1, 15llu - i, out);
		}
		else
		{
			fwrite(buf + i + 1, 1, 15llu - i, out);

			for (uint32_t j = 15 - i; j < in.get_width(); ++j)
				putc(in.get_filler(), out);
		}
	}

	OCH_FMT_FX(f_binary)
	{
		uint64_t val = in.i;

		char buf[64];
		int i = 63;

		if (!val)
			buf[i--] = '0';

		while (val)
		{
			buf[i--] = '0' + (val & 1);

			val >>= 1;
		}

		if (in.get_rightadj())
		{
			for (uint32_t j = 63 - i; j < in.get_width(); ++j)
				putc(in.get_filler(), out);

			fwrite(buf + i + 1, 1, 63llu - i, out);
		}
		else
		{
			fwrite(buf + i + 1, 1, 63llu - i, out);

			for (uint32_t j = 63 - i; j < in.get_width(); ++j)
				putc(in.get_filler(), out);
		}
	}

	OCH_FMT_FX(f_character)
	{
		if (in.get_rightadj())
		{
			for (uint32_t j = 1; j < in.get_width(); ++j)
				putc(in.get_filler(), out);

			putc(static_cast<char>(in.i), out);
		}
		else
		{
			putc(static_cast<char>(in.i), out);

			for (uint32_t j = 1; j < in.get_width(); ++j)
				putc(in.get_filler(), out);
		}
	}

	//TODO
	OCH_FMT_FX(f_scientific)
	{

	}

	//Format-function vtable

	OCH_FMT_RET (*const format_functions[32])OCH_FMT_ARG
	{
		f_uint,
		f_int,
		f_float,
		f_double,
		f_cstring,
		f_ministring,
		nullptr,			//a
		f_binary,			//b
		f_character,		//c
		nullptr,			//d
		f_scientific,		//e
		nullptr,			//f
		nullptr,			//g
		nullptr,			//h
		nullptr,			//i
		nullptr,			//j
		nullptr,			//k
		nullptr,			//l
		nullptr,			//m
		nullptr,			//n
		nullptr,			//o
		nullptr,			//p
		nullptr,			//q
		nullptr,			//r
		nullptr,			//s
		nullptr,			//t
		nullptr,			//u
		nullptr,			//v
		nullptr,			//w
		f_hexadecimal,		//x
		nullptr,			//y
		nullptr,			//z
	};

	// [argindex} [:[width] [.precision] [rightadj] [~filler] [signmode] [formatmode]]
	void vprint(const char* fmt, arg argv[], uint32_t argc, FILE* out)
	{
		uint32_t arg_idx = 0;

		for (char c = *fmt; c; c = *(++fmt))
		{
			if (c == '{')//Escape char encountered
			{
				c = *(++fmt);

				if (c == '{')						//Exit for printing escape-character
				{
					putc('{', out);
					continue;
				}

				//Actual formatting encountered

				if(c >= '0' && c <= '9')			//Index
				{
					arg_idx = c - '0';
					c = *(++fmt);
				}

				_ASSERT(arg_idx < argc);

				arg curr_arg = argv[arg_idx];

				if (c == ':')						//Formato xxxtendo
				{
					// [width] [precision] [rightadj] [filler] [signmode] [formatmode]
					//
					//		width			[0-9]*, indicating minimum field width to be padded to
					//		precision		.[0-9]*, indicating decimal digits for float/double, or number of printable characters for strings
					//		rightadj		'>', indicating that the formatted string shall be right-adjusted in its field
					//		filler			~.
					//		signmode		'+' or '_', meaning positive signed ints are padded with a plus or space respectively
					//		formatmode		o, x, X, b, c for (u)ints, e, b for float/double

					//f_width
					uint16_t f_width = 0;
					while ((c = *(++fmt)) >= '0' && c <= '9')
						f_width = f_width * 10 + c - '0';
					curr_arg.set_width(f_width);

					//f_precision
					if (c == '.')
					{
						uint16_t f_precision = 0;
						while ((c = *(++fmt)) >= '0' && c <= '9')
							f_precision = f_precision * 10 + c - '0';
						curr_arg.set_precision(f_precision);
					}

					//f_rightadj
					if (c == '>')
					{
						curr_arg.set_rightadj();
						c = *(++fmt);
					}

					//f_filler
					if (c == '~')
					{
						c = *(++fmt);
						curr_arg.set_filler(c);
						c = *(++fmt);
					}

					//f_signmode
					if (c == '+')
					{
						curr_arg.set_signmode(1);
						c = *(++fmt);
					}
					else if (c == '_')
					{
						curr_arg.set_signmode(2);
						c = *(++fmt);
					}

					if (c >= 'a' && c <= 'z')
					{
						curr_arg.set_offset(c - 'a' + 6);
						c = *(++fmt);
					}
				}

				_ASSERT(c == '}');

				format_functions[static_cast<uint8_t>(curr_arg.get_offset())](curr_arg, out);

				++arg_idx;
			}
			else
			{
				putc(c, out);
			}
		}
	}

#undef OCH_FMT_FX
#undef OCH_FMT_ARG
#undef OCH_FMT_RET

	template<typename... Args>
	void print(const char* const fmt, Args... args)
	{
		arg argv[]{ args... };

		vprint(fmt, argv, sizeof...(args), stdout);
	}

	void print(const char* const fmt)
	{
		fputs(fmt, stdout);
	}

	namespace fmt
	{
		void hex_lowercase()
		{
			f_curr_hex_symbols = f_hex_symbols[1];
		}

		void hex_uppercase()
		{
			f_curr_hex_symbols = f_hex_symbols[0];
		}

		void set_default_precision(uint32_t precision)
		{
			if (precision > 9)
				precision = 9;

			f_default_precision = precision;

			float precision_factor = 1.0F;

			for (uint32_t i = 0; i < precision; ++i)
				precision_factor *= 10.0F;

			f_default_precision_factor = precision_factor;
		}
	}
}
