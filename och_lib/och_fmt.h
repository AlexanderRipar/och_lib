#pragma once

#include <cstdint>
#include <cstring>

#include "och_memrun.h"

namespace och
{
	struct arg
	{
		enum class types : uint8_t
		{
			u,	//unsigned integer
			i,	//signed integer
			f,	//float
			d,	//double
			s,	//const char*
			m,	//och::ministring
		};

		const types type;

		arg(        uint8_t i) : type{ types::u }, i{ static_cast<uint64_t>(i) } {}
		arg(       uint16_t i) : type{ types::u }, i{ static_cast<uint64_t>(i) } {}
		arg(       uint32_t i) : type{ types::u }, i{ static_cast<uint64_t>(i) } {}
		arg(       uint64_t i) : type{ types::u }, i{ static_cast<uint64_t>(i) } {}
		arg(         int8_t i) : type{ types::i }, i{ static_cast<uint64_t>(i) } {}
		arg(        int16_t i) : type{ types::i }, i{ static_cast<uint64_t>(i) } {}
		arg(        int32_t i) : type{ types::i }, i{ static_cast<uint64_t>(i) } {}
		arg(        int64_t i) : type{ types::i }, i{ static_cast<uint64_t>(i) } {}
		arg(    const char* s) : type{ types::s }, s{                       s } {}
		arg(         double d) : type{ types::d }, d{                       d } {}
		arg(          float f) : type{ types::f }, f{                       f } {}
		arg(och::ministring m) : type{ types::m }, m{                       m } {}
		arg(    och::string m) : type{ types::m }, m{                       m } {}

		union
		{
			uint64_t i;
			const char* s;
			double d;
			float f;
			och::ministring m;
		};
	};

#define FMT_C '{'//REMOVE; ONLY FOR NOW

#define FMT_RET void
#define FMT_ARG (arg in, FILE* out, uint32_t precision, uint32_t width, bool rightadj, char filler, uint8_t signmode)
#define FMT_FX(name) FMT_RET name FMT_ARG

	FMT_FX(f_uint)
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

		if (rightadj)
			for (uint32_t j = 19 - i; j < width; ++j)
				putc(filler, out);

		fwrite(buf + i + 1, 1, 19llu - i, out);

		if (!rightadj)
			for (uint32_t j = 19 - i; j < width; ++j)
				putc(filler, out);
	}

	FMT_FX(f_int)
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
		else if (signmode == 1)
			buf[i--] = '+';
		else if (signmode == 2)
			buf[i--] = ' ';

		if (rightadj)
			for (uint32_t j = 19 - i; j < width; ++j)
				putc(filler, out);

		fwrite(buf + i + 1, 1, 19llu - i, out);

		if (!rightadj)
			for (uint32_t j = 19 - i; j < width; ++j)
				putc(filler, out);
	}

	FMT_FX(f_cstring)
	{
		size_t len = strlen(in.s);

		if (precision != -1 && len > precision)
			len = precision;

		if (rightadj)
			for (size_t j = len; j < width; ++j)
				putc(filler, out);

		fwrite(in.s, 1, len, out);

		if (!rightadj)
			for (size_t j = len; j < width; ++j)
				putc(filler, out);
	}

	FMT_FX(f_ministring)
	{
		size_t len = in.m.len();

		if (precision != -1 && len > precision)
			len = precision;

		if (rightadj)
			for (size_t j = len; j < width; ++j)
				putc(filler, out);

		fwrite(in.m.begin(), 1, len, out);

		if (!rightadj)
			for (size_t j = len; j < width; ++j)
				putc(filler, out);
	}

	//TODO
	FMT_FX(f_float)
	{

	}

	//TODO
	FMT_FX(f_double)
	{

	}

	//Extended formatmode functions

	constexpr const char f_hex_symbols[2][16]{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	const char* f_curr_hex_symbols = f_hex_symbols[0];

	FMT_FX(f_hexadecimal)
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

		if (rightadj)
			for (uint32_t j = 15 - i; j < width; ++j)
				putc(filler, out);

		fwrite(buf + i + 1, 1, 15llu - i, out);

		if (!rightadj)
			for (uint32_t j = 15 - i; j < width; ++j)
				putc(filler, out);
	}

	FMT_FX(f_binary)
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

		if (rightadj)
			for (uint32_t j = 63 - i; j < width; ++j)
				putc(filler, out);

		fwrite(buf + i + 1, 1, 63llu - i, out);

		if (!rightadj)
			for (uint32_t j = 63 - i; j < width; ++j)
				putc(filler, out);

		/*uint64_t val = in.i;

		char buf[64];

		uint32_t bits_written = 0;

		while (val)
		{
			buf[64 - ++bits_written] = '0' + (val & 1);

			val >>= 1;
		}

		uint32_t max_bits = bits_written < precision * 8 ? bits_written : precision * 8;

		if (rightadj)
			for (uint32_t j = max_bits; j < width; ++j)
				putc(filler, out);

		fwrite(buf + 64 - max_bits, 1, max_bits, out);

		if (!rightadj)
			for (uint32_t j = max_bits; j < width; ++j)
				putc(filler, out);*/
	}

	FMT_FX(f_character)
	{
		if (rightadj)
			for (uint32_t j = 1; j < width; ++j)
				putc(filler, out);

		putc(static_cast<char>(in.i), out);

		if (!rightadj)
			for (uint32_t j = 1; j < width; ++j)
				putc(filler, out);
	}

	//TODO
	FMT_FX(f_scientific)
	{

	}

	//Format-function vtable

	FMT_RET (*const format_functions[])FMT_ARG
	{
		f_uint,
		f_int,
		f_float,
		f_double,
		f_cstring,
		f_ministring
	};

	enum class fmt_m : uint8_t
	{
		def,
		hex,
		bin,
		chr,
		sci,
	};

	// [width] [precision] [rightadj] [filler] [signmode] [formatmode]
	void vprint(const char* fmt, arg argv[], uint32_t argc, FILE* out)
	{
		uint32_t arg_idx = 0;

		for (char c = *fmt; c; c = *(++fmt))
		{
			if (c == FMT_C)//Escape char encountered
			{
				c = *(++fmt);

				if (c == FMT_C)						//Exit for printing escape-character
				{
					putc(FMT_C, out);
					continue;
				}

				//Actual formatting encountered

				if(c >= '0' && c <= '9')			//Index
				{
					arg_idx = c - '0';
					c = *(++fmt);
				}

				_ASSERT(arg_idx < argc);

				arg& curr_arg = argv[arg_idx];

				uint32_t f_width = 0;
				uint32_t f_precision = -1;
				bool f_rightadj = false;
				char f_filler = ' ';
				uint8_t f_signmode = 0;

				bool is_special_formatmode = false;

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
					while ((c = *(++fmt)) >= '0' && c <= '9')
						f_width = f_width * 10 + c - '0';

					//f_precision
					if (c == '.')
						while ((c = *(++fmt)) >= '0' && c <= '9')
							f_precision = f_precision * 10 + c - '0';
					else
						f_precision = -1;

					//f_rightadj
					if (c == '>')
					{
						f_rightadj = true;
						c = *(++fmt);
					}

					//f_filler
					if (c == '~')
					{
						c = *(++fmt);
						f_filler = c;
						c = *(++fmt);
					}

					//f_signmode
					if (c == '+')
					{
						f_signmode = 1;
						c = *(++fmt);
					}
					else if (c == '_')
					{
						f_signmode = 2;
						c = *(++fmt);
					}

					//f_formatmode
					switch (c)
					{
					case 'x':
						_ASSERT(curr_arg.type == arg::types::i || curr_arg.type == arg::types::u);
						f_hexadecimal(curr_arg, out, f_precision, f_width, f_rightadj, f_filler, f_signmode);
						is_special_formatmode = true;
						break;

					case 'c':
						_ASSERT(curr_arg.type == arg::types::i || curr_arg.type == arg::types::u);
						f_character(curr_arg, out, f_precision, f_width, f_rightadj, f_filler, f_signmode);
						is_special_formatmode = true;
						break;

					case 'b':
						_ASSERT(curr_arg.type == arg::types::i || curr_arg.type == arg::types::u || curr_arg.type == arg::types::f || curr_arg.type == arg::types::d);
						f_binary(curr_arg, out, f_precision, f_width, f_rightadj, f_filler, f_signmode);
						is_special_formatmode = true;
						break;

					case 'e':
						_ASSERT(curr_arg.type == arg::types::f || curr_arg.type == arg::types::d);
						f_scientific(curr_arg, out, f_precision, f_width, f_rightadj, f_filler, f_signmode);
						is_special_formatmode = true;
						break;

					default:
						--fmt;//Cludge to counteract increment below
						break;
					}

					c = *(++fmt);
				}

				_ASSERT(c == '}');

				if(!is_special_formatmode)
					format_functions[static_cast<uint8_t>(curr_arg.type)](curr_arg, out, f_precision, f_width, f_rightadj, f_filler, f_signmode);

				++arg_idx;
			}
			else
			{
				putc(c, out);
			}
		}
	}

#undef FMT_C
#undef FMT_RET
#undef FMT_ARG
#undef FMT_FX

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
	}
}
