#include "och_fmt.h"

#include <cstdint>

#include "och_memrun.h"
#include "och_fio.h"

namespace och
{
	constexpr const char f_hex_symbols[2][16]{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
	const char* f_curr_hex_symbols = f_hex_symbols[0];

	constexpr int vprint_buf_size = 8192;

	char _vbuf[vprint_buf_size];
	och::memrun vbuf(_vbuf);

	inline void to_vbuf(char c, iohandle out)
	{
		if (!vbuf.len())
		{
			och::write_to_file(out, { vbuf.end - vprint_buf_size, vbuf.end });
			vbuf.beg = _vbuf;
		}

		*(vbuf.beg++) = c;
	}

	inline void to_vbuf(const char* str, uint32_t len, iohandle out)
	{
		while (true)
		{
			uint32_t to_write = len < (uint32_t)vbuf.len() ? len : (uint32_t)vbuf.len();

			for (uint32_t i = 0; i < to_write; ++i)
				*(vbuf.beg++) = *(str++);

			len -= to_write;
			
			if (!len)
				return;

			och::write_to_file(out, { vbuf.end - vprint_buf_size, vbuf.beg });
			vbuf.beg = _vbuf;
		}
	}

	uint32_t f_default_precision = 4;
	float f_default_precision_factor = 10.0F * 10.0F * 10.0F * 10.0F;

	void f_uint(arg in, och::iohandle out)
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
				to_vbuf(in.get_filler(), out);
		
			to_vbuf(buf + i + 1, 19 - i, out);
		}
		else
		{
			to_vbuf(buf + i + 1, 19 - i, out);

			for (uint32_t j = 19 - i; j < in.get_width(); ++j)
				to_vbuf(in.get_filler(), out);
		}
	}

	void f_int(arg in, och::iohandle out)
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
				to_vbuf(in.get_filler(), out);
		
			to_vbuf(buf + i + 1, 19llu - i, out);
		}
		else
		{
			to_vbuf(buf + i + 1, 19llu - i, out);
		
			for (uint32_t j = 19 - i; j < in.get_width(); ++j)
				to_vbuf(in.get_filler(), out);
		}
	}

	void f_ministring(arg in, och::iohandle out)
	{
		uint16_t len = in.s.len();

		if (in.get_precision() != -1 && len > in.get_precision())
			len = in.get_precision();

		if (in.get_rightadj())
		{
			for (size_t j = len; j < in.get_width(); ++j)
				to_vbuf(in.get_filler(), out);
		
			to_vbuf(in.s.begin(), len, out);
		}
		else
		{
			to_vbuf(in.s.begin(), len, out);
		
			for (size_t j = len; j < in.get_width(); ++j)
				to_vbuf(in.get_filler(), out);
		}
	}

	void f_float(arg in, och::iohandle out)
	{
		//constexpr uint32_t bufsize = 32;
		//
		//char buf[bufsize];
		//
		//uint32_t i = bufsize;
		//
		//float f = in.f;
		//
		//if (f == INFINITY)
		//{
		//	i -= 3;
		//	memcpy(buf + 17, "inf", 3);
		//
		//	if (in.get_signmode() == 1)
		//		buf[--i] = '+';
		//	else if (in.get_signmode() == 2)
		//		buf[--i] = ' ';
		//
		//}
		//else if (f == -INFINITY)
		//{
		//	i -= 4;
		//	memcpy(buf + 16, "-inf", 4);
		//}
		//else
		//{
		//	int64_t int_val = static_cast<int64_t>(f);
		//
		//	float precision_factor;
		//
		//	if (in.get_precision() == -1)
		//	{
		//		in.set_precision(f_default_precision);
		//		precision_factor = f_default_precision_factor;
		//	}
		//	else
		//		precision_factor = powf(in.get_precision(), 10.0F);
		//
		//	uint64_t dec_val = static_cast<uint64_t>((in.f - static_cast<float>(int_val)) * f_default_precision_factor);
		//
		//	while (dec_val && bufsize - i != in.get_precision())
		//	{
		//		buf[--i] = '0' + dec_val % 10;
		//
		//		dec_val /= 10;
		//	}
		//
		//	while (bufsize - i != in.get_precision())
		//		buf[--i] = '0';
		//
		//	if (in.get_precision())
		//		buf[--i] = '.';
		//
		//	bool negative = false;
		//	if (f < 0)
		//	{
		//		negative = true;
		//		int_val = -int_val;
		//	}
		//	else if (!int_val)
		//		buf[--i] = '0';
		//	else
		//		while (int_val)
		//		{
		//			buf[--i] = '0' + int_val % 10;
		//
		//			int_val /= 10;
		//		}
		//
		//	if (negative)
		//		buf[--i] = '-';
		//	else if (in.get_signmode() == 1)
		//		buf[--i] = '+';
		//	else if (in.get_signmode() == 2)
		//		buf[--i] = ' ';
		//}
		//
		//if (in.get_rightadj())
		//{
		//	for (uint32_t j = bufsize - i; j < in.get_width(); ++j)
		//		putc(in.get_filler(), out);
		//
		//	fwrite(buf + i, 1, bufsize - i, out);
		//}
		//else
		//{
		//	fwrite(buf + i, 1, bufsize - i, out);
		//
		//	for (uint32_t j = bufsize - i; j < in.get_width(); ++j)
		//		putc(in.get_filler(), out);
		//}
	}

	void f_double(arg in, och::iohandle out)
	{

	}

	void f_hexadecimal(arg in, och::iohandle out)
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
				to_vbuf(in.get_filler(), out);
		
			to_vbuf(buf + i + 1, 15llu - i, out);
		}
		else
		{
			to_vbuf(buf + i + 1, 15llu - i, out);
		
			for (uint32_t j = 15 - i; j < in.get_width(); ++j)
				to_vbuf(in.get_filler(), out);
		}
	}

	void f_binary(arg in, och::iohandle out)
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
				to_vbuf(in.get_filler(), out);
		
			to_vbuf(buf + i + 1, 63llu - i, out);
		}
		else
		{
			to_vbuf(buf + i + 1, 63llu - i, out);
		
			for (uint32_t j = 63 - i; j < in.get_width(); ++j)
				to_vbuf(in.get_filler(), out);
		}
	}

	void f_character(arg in, och::iohandle out)
	{
		if (in.get_rightadj())
		{
			for (uint32_t j = 1; j < in.get_width(); ++j)
				to_vbuf(in.get_filler(), out);
		
			to_vbuf(static_cast<char>(in.i), out);
		}
		else
		{
			to_vbuf(static_cast<char>(in.i), out);
		
			for (uint32_t j = 1; j < in.get_width(); ++j)
				to_vbuf(in.get_filler(), out);
		}
	}

	void f_scientific(arg in, och::iohandle out)
	{

	}

	//Format-function vtable
	fmt_function format_functions[32]{
		f_uint,
		f_int,
		f_float,
		f_double,
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

	// [argindex] [:[width] [.precision] [rightadj] [~filler] [signmode] [formatmode]]
	void vprint(const char* fmt, arg argv[], uint32_t argc, och::iohandle out)
	{
		uint32_t arg_idx = 0;

		for (char c = *fmt; c; c = *(++fmt))
		{
			if (c == '{')//Escape char encountered
			{
				c = *(++fmt);

				if (c == '{')						//Exit for printing escape-character
				{
					to_vbuf(c, out);

					continue;
				}

				//Actual formatting encountered

				if (c >= '0' && c <= '9')					//Index
				{
					arg_idx = c - '0';
					c = *(++fmt);
				}

				_ASSERT(arg_idx < argc);

				arg curr_arg = argv[arg_idx];

				if (c == ':')								//Formato xxxtendo
				{
					// [width] [precision] [rightadj] [filler] [signmode] [formatmode]
					//
					//		width			[0-9]*, indicating minimum field width to be padded to
					//		precision		.[0-9]*, indicating decimal digits for float/double, or number of printable characters for strings
					//		rightadj		'>', indicating that the formatted string shall be right-adjusted in its field
					//		filler			~.
					//		signmode		'+' or '_', meaning positive signed ints are padded with a plus or space respectively
					//		formatmode		o, x, X, b, c for (u)ints, e, b for float/double

					uint16_t f_width = 0;
					while ((c = *(++fmt)) >= '0' && c <= '9')
						f_width = f_width * 10 + c - '0';
					curr_arg.set_width(f_width);

					if (c == '.')
					{
						uint16_t f_precision = 0;
						while ((c = *(++fmt)) >= '0' && c <= '9')
							f_precision = f_precision * 10 + c - '0';
						curr_arg.set_precision(f_precision);
					}

					if (c == '>')
					{
						curr_arg.set_rightadj();
						c = *(++fmt);
					}

					if (c == '~')
					{
						c = *(++fmt);
						curr_arg.set_filler(c);
						c = *(++fmt);
					}

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
						curr_arg.set_offset(c - 'a' + 5);
						c = *(++fmt);
					}
				}

				_ASSERT(c == '}');

				format_functions[static_cast<uint8_t>(curr_arg.get_offset())](curr_arg, out);

				++arg_idx;
			}
			else
			{
				to_vbuf(c, out);
			}
		}

		if (vbuf.beg != _vbuf)
		{
			och::write_to_file(out, { _vbuf, vbuf.beg });
			vbuf.beg = _vbuf;
		}
	}

	void print(const och::string fmt)
	{
		och::write_to_file(och::out, fmt);
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

		bool register_formatfunction(och::fmt_function function, char specifier)
		{
			if (specifier <= 'a' && specifier >= 'z')
				return false;

			uint8_t index = specifier - 'a' + 5;

			format_functions[index] = function;

			return true;
		}
	}
}
