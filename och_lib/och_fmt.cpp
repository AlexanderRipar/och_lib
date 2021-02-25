#include "och_fmt.h"

#include <cstdint>

#include "och_range.h"
#include "och_fio.h"

namespace och
{
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*////////////////////////////////////////////////formatting helpers/////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	constexpr const char* hex_lower_upper = "0123456789abcdef0123456789ABCDEF";

	void write_with_padding(och::iohandle out, och::stringview text, const och::parsed_context& context)
	{
		uint32_t written = (uint32_t)text.get_codepoints();

		if (context.flags & 4)
			while (written++ < context.width)
				och::write_to_file(out, { context.filler.cbegin(), context.filler.cend() });

		och::write_to_file(out, { text.raw_cbegin(), text.raw_cend() });

		if (!(context.flags & 4))
			while (written++ < context.width)
				och::write_to_file(out, { context.filler.cbegin(), context.filler.cend() });
	}

	char* reverse_itos(char* out, uint64_t n)
	{
		while (n >= 10)
		{
			*out-- = (n % 10) + '0';

			n /= 10;
		}

		*out-- = (char)n + '0';

		return out;
	}

	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////formatting functions////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	void fmt_uint(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		uint64_t value = arg_value.u;

		char buf[32];

		char* curr = buf + 31;

		if (context.format_specifier == '\0')
			curr = reverse_itos(curr, value);
		else if (context.format_specifier == 'x')
			while (value)
			{
				*curr-- = hex_lower_upper[value & 0xF];

				value >>= 4;
			}
		else if(context.format_specifier == 'X')
			while (value)
			{
				*curr-- = hex_lower_upper[16 + value & 0xF];

				value >>= 4;
			}
		else if (context.format_specifier == 'b')
			while (value)
			{
				*curr-- = '0' + (value & 1);

				value >>= 1;
			}
		else if (context.format_specifier == 'B')
			for (uint32_t i = 0; i != 32; ++i)
			{
				*curr-- = '0' + (value & 1);

				value >>= 1;
			}
		else
		{
			och::write_with_padding(out, "[[Invalid format-specifier]]", context);

			return;
		}
		
		++curr;

		write_with_padding(out, och::stringview(curr, (uint32_t)(buf - curr + 32), (uint32_t)(buf - curr + 32)), context);
	}

	void fmt_int(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		int64_t value = arg_value.i;

		char buf[32];

		char* curr = buf + 31;

		bool is_negative = false;

		if (value < 0)
		{
			is_negative = true;

			value = -value;
		}

		if (context.format_specifier == '\0')
		{
			curr = reverse_itos(curr, value);
		}
		else if (context.format_specifier == 'X')
			while (value)
			{
				*curr-- = hex_lower_upper[16 + value & 0xF];

				value >>= 4;
			}
		else if (context.format_specifier == 'b')
			while (value)
			{
				*curr-- = '0' + (value & 1);

				value >>= 1;
			}
		else if (context.format_specifier == 'B')
			for (uint32_t i = 0; i != 31; ++i)
			{
				*curr-- = '0' + (value & 1);

				value >>= 1;
			}
		else
		{
			och::write_with_padding(out, "[[Invalid format-specifier", context);

			return;
		}

		if (is_negative)
			*curr-- = '-';
		else if (context.flags & 1)
			*curr-- = '+';
		else if (context.flags & 2)
			*curr-- = ' ';

		++curr;

		write_with_padding(out, och::stringview(curr, (uint32_t)(buf - curr + 32), (uint32_t)(buf - curr + 32)), context);
	}

	void fmt_utf8_view(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		och::utf8_view value = *(const och::utf8_view*)arg_value.p;

		if (value.get_codepoints() > context.precision)
			value = value.subview(0, context.precision);

		write_with_padding(out, value, context);
	}

	void fmt_utf8_string(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		och::utf8_view value(*(const och::utf8_string*)arg_value.p);

		fmt_utf8_view(out, (const void*)&value, context);
	}

	void fmt_cstring(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		och::utf8_view value(reinterpret_cast<const char*>(arg_value.p));

		fmt_utf8_view(out, (const void*)&value, context);
	}

	void fmt_codepoint(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		och::utf8_char value = arg_value.c;

		write_with_padding(out, och::stringview(value.cbegin(), value.get_codeunits(), 1), context);
	}

	void fmt_float(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		//     ->   base 10 decimal
		// e   ->   base 10 scientific ([+ -]N.NNNNe+-NNNN)
		// x,X ->   base 16 decimal. x uses lowercase letters, H uppercase
		// h,H ->   base 16 scientific ([+ -]N.NNNNp+-NNNN). h uses lowercase letters, H uppercase
		// b   ->   bitpattern
		// B   ->   bitpattern with ' separating sign, exponent and mantissa
		
		char buf[64];

		char* curr = buf;

		const uint32_t value = (uint32_t)arg_value.u;
		
		const bool is_negative = value & 0x8000'0000;
		const int8_t exponent = (int8_t)(((value & 0x7F80'0000) >> 23) - 127);
		const uint32_t mantissa = value & 0x007F'FFFF;

		if /*TODO Implement*/ (context.format_specifier == '\0')
		{

		}
		else if (context.format_specifier == 'b')
		{
			uint32_t mask = 0x8000'0000;

			while (mask)
			{
				*curr++ = '0' + ((value & mask) == mask);
				mask >>= 1;
			}
		}
		else if (context.format_specifier == 'B')
		{
			uint32_t mask = 0x8000'0000;

			*curr++ = '0' + ((value & mask) == mask);
			mask >>= 1;

			*curr++ = '\'';

			for (uint32_t i = 0; i != 8; ++i)
			{
				*curr++ = '0' + ((value & mask) == mask);
				mask >>= 1;
			}

			*curr++ = '\'';

			while (mask)
			{
				*curr++ = '0' + ((value & mask) == mask);
				mask >>= 1;
			}
		}
		else if /*TODO Implement*/ (context.format_specifier == 'x' || context.format_specifier == 'X')
		{

		}
		else if/*TODO Implement*/ (context.format_specifier == 'e')
		{

		}
		else if /*TODO Rounding*/ (context.format_specifier == 'h' || context.format_specifier == 'H')
		{
			const char* hex = context.format_specifier == 'h' ? hex_lower_upper : hex_lower_upper + 16;

			int8_t hex_exponent = exponent >> 2;

			const uint32_t hex_mantissa = ((1 << 23) | mantissa) << (exponent & 3);

			const uint32_t predec = hex_mantissa >> 23;

			const uint32_t postdec = hex_mantissa & ((1 << 23) - 1);

			if (is_negative)
				*curr++ = '-';
			else if (context.flags & 1)
				*curr++ = '+';
			else if (context.flags & 2)
				*curr++ = ' ';

			*curr++ = hex[predec];

			const char* const decimal_point = curr;

			*curr++ = '.';

			uint32_t shift = 19;

			while (shift >= 4)
			{
				*curr++ = hex[(postdec >> shift) & 0xF];
				shift -= 4;
			}

			*curr++ = hex[postdec & 3];

			while (curr[-1] == '0' && curr[-2] != '.')
				--curr;

			if (context.precision != 0xFFFF)
			{
				//Cut off until precision is equal to curr - decimal_point
				while (curr - decimal_point - 1 > context.precision)
					--curr;

				//Add '0' until precision is equal to curr - decimal_point
				while (curr - decimal_point - 1 < context.precision)
					*curr++ = '0';
			}

			*curr++ = 'p';

			if (hex_exponent < 0)
			{
				*curr++ = '-';
				hex_exponent = -hex_exponent;
			}

			if (hex_exponent > 0xF)
				*curr++ = hex[hex_exponent >> 4];

			*curr++ = hex[hex_exponent & 0xF];
		}
		else
		{
			och::write_with_padding(out, "[[Invalid format-specifier", context);
			return;
		}

		och::write_with_padding(out, och::stringview(buf, (uint32_t)(curr - buf), (uint32_t)(curr - buf)), context);
	}

	void fmt_double(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		arg_value;

		context;

		och::write_to_file(out, "[[fmt_double is not yet implemented]]");
	}

	void fmt_date(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		//          [y]yyyy-mm-dd, hh:mm:ss.mmm
		// d   ->   [y]yyyy-mm-dd
		// t   ->   is_utc ? hh:mm:ss.mmm : hh:mm:ss:mmm
		// u   ->   is_utc ? [y]yyyy-mm-ddThh:mm:ss.mmmZ : [y]yyyy-mm-ddThh:mm:ss.mmm+-hh:mm
		// x   ->   custom format:
		//          +-------------------------------------------------------------------------------------------+
		//          | y   ->   year														                        |
		//          | m   ->   month, w/o leading zero										                    |
		//          | n   ->   short monthname (Jan, Feb, ...)								                    |
		//          | d   ->   monthday, w/o leading zero									                    |
		//          | w   ->   short weekday (Mon, Tue, ...)								                    |
		//          | i   ->   hour, w/o leading zero										                    |
		//          | j   ->   minute, w/o leading zero									                        |
		//          | k   ->   second, w/o leading zero									                        |
		//          | l   ->   millisecond, three digits									                    |
		//          | 	   	   																                    |
		//          | u   ->   utc-offset hours (+-hh)										                    |
		//          | U   ->   utc-offset minutes (mm)										                    |
		//          | s?  ->   utf8_char after s is only printed if date is local			                    |
		//          | S?  ->   utf8_char after s is only printed if date is utc			                        |
		//          | x   ->   x is ignored and the next character is printed, even if it is a format-specifier |
		//          | 																	                        |
		//          | Capital letters (except U) indicate leading zeroes, or extended name                      |
		//          +-------------------------------------------------------------------------------------------+

		char buf[64];

		char* curr = buf;

		const och::date& in = *reinterpret_cast<const och::date*>(arg_value.p);

		uint32_t utf8_cpoints = 0;

		const char* format;

		if (context.format_specifier == '\0')
			format = "y-M-D, I:J:K.L}";
		else if (context.format_specifier == 'd')
			format = "y-M-D}";
		else if (context.format_specifier == 't')
			format = "I:J:K.L}";
		else if (context.format_specifier == 'u')
			format = "Y-M-DTI:J:K.Lus:U}";
		else if (context.format_specifier == 'x')
			format = context.raw_context;
		else
		{
			och::write_with_padding(out, "[[Invalid format-specifier", context);
			return;
		}

		constexpr const char* weekdays = "Sunday\0\0\0\0" "Monday\0\0\0\0" "Tuesday\0\0\0" "Wednesday\0" "Thursday\0\0" "Friday\0\0\0\0" "Saturday\0";
		constexpr const char* months = "January\0\0\0"    "February\0\0"   "March\0\0\0\0\0" "April\0\0\0\0\0" "May\0\0\0\0\0\0\0" "June\0\0\0\0\0\0"
			                           "July\0\0\0\0\0\0" "August\0\0\0\0" "September\0"     "October\0\0\0"   "November\0\0"      "December\0";

#define OCH_FMT_2DIGIT(x) if(x >= 10 || c >= 'A') *curr++ = '0' + (char)x / 10; *curr++ = '0' + (char)x % 10;

		for (char c = *format; c != '}'; c = *++format)
		{
			char* prev = curr;

			switch (c)
			{
			case 'y':
			{
				uint16_t y = in.year();

				int32_t idx = y >= 10000 ? 5 : y >= 1000 ? 4 : y >= 100 ? 3 : y >= 10 ? 2 : 1;

				curr += idx;

				for (int32_t i = 1; i != idx; ++i)
				{
					curr[-i] = '0' + y % 10;
					y /= 10;
				}

				curr[-idx] = '0' + (char)y;
			}
				break;
			case 'Y':
			{
				uint16_t y = in.year();

				if (y >= 10000)
				{
					*curr++ = '0' + (char)(y / 10000);
					y /= 10;
				}

				curr[3] = '0' + y % 10;
				y /= 10;
				curr[2] = '0' + y % 10;
				y /= 10;
				curr[1] = '0' + y % 10;
				y /= 10;
				curr[0] = '0' + (char)y;

				curr += 4;
			}
				break;
			case 'm':
			case 'M':
				OCH_FMT_2DIGIT(in.month());
				break;
			break;
			case 'n':
			{
				*curr++ = months[(in.month() - 1) * 10];
				*curr++ = months[(in.month() - 1) * 10 + 1];
				*curr++ = months[(in.month() - 1) * 10 + 2];
			}
				break;
			case 'N':
			{
				const char* monthname = months + (ptrdiff_t)(in.month() - 1) * 10;

				while (*monthname)
					*curr++ = *monthname++;
			}
				break;
			case 'd':
			case 'D':
				OCH_FMT_2DIGIT(in.monthday());
				break;
			case 'w':
			{
				*curr++ = weekdays[in.weekday() * 10];
				*curr++ = weekdays[in.weekday() * 10 + 1];
				*curr++ = weekdays[in.weekday() * 10 + 2];

			}
			break;
			case 'W':
			{
				const char* dayname = weekdays + (ptrdiff_t)in.weekday() * 10;

				while (*dayname)
					*curr++ = *dayname++;
			}
			break;
			case 'i':
			case 'I':
				OCH_FMT_2DIGIT(in.hour());
				break;
			case 'j':
			case 'J':
				OCH_FMT_2DIGIT(in.minute());
			break;
			case 'k':
			case 'K':
				OCH_FMT_2DIGIT(in.second());
			break;
			case 'l':
			{
				if (in.millisecond() >= 100)
					*curr++ = '0' + (char)(in.millisecond() / 100);
				if (in.millisecond() >= 10)
					*curr++ = '0' + (in.millisecond() / 10) % 10;
				*curr++ = '0' + in.millisecond() % 10;
			}
			break;
			case 'L':
			{
				*curr++ = '0' + (char)(in.millisecond() / 100);
				*curr++ = '0' + (in.millisecond() / 10) % 10;
				*curr++ = '0' + in.millisecond() % 10;
			}
			break;
			case 'u':
			{
				if (in.is_utc())
				{
					*curr++ = 'Z';

					break;
				}

				*curr++ = in.utc_offset_is_negative() ? '-' : '+';

				uint16_t h = in.utc_offset_hours();

				*curr++ = '0' + (char)(h / 10);
				*curr++ = '0' + h % 10;
			}
			break;
			case 'U':
			{
				if (in.is_utc())
					break;

				uint16_t m = in.utc_offset_minutes();

				*curr++ = '0' + (char)(m / 10);
				*curr++ = '0' + m % 10;
			}
			break;
			case 's':
			case 'S':
			{
				if ((c != 's') ^ (in.is_utc())) //Next char is inactive
				{
					if (*++format == 'x')
						++format;

					while (_is_utf8_surr(format[1]))
						++format;
				}
			}
			break;
			case 'x':
				c = *++format;//Fallthrough...
			default:
				*curr++ = c;
				utf8_cpoints -= _is_utf8_surr(c);
				break;
			}

			utf8_cpoints += (uint32_t)(curr - prev);
		}

#undef OCH_FMT_2DIGIT

		write_with_padding(out, och::stringview(buf, (uint32_t)(curr - buf), utf8_cpoints), context);
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////////fmt_value/////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	fmt_value::fmt_value(uint64_t u) : u{ u } {}

	fmt_value::fmt_value(int64_t i) : i{ i } {}

	fmt_value::fmt_value(float f) : f{ f } {}

	fmt_value::fmt_value(double d) : d{ d } {}

	fmt_value::fmt_value(const void* p) : p{ p } {}

	fmt_value::fmt_value(och::utf8_char c) : c{ c } {}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*////////////////////////////////////////////////////arg_wrapper////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	arg_wrapper::arg_wrapper(uint8_t value) : value{ (uint64_t)value }, formatter{ fmt_uint } {}

	arg_wrapper::arg_wrapper(uint16_t value) : value{ (uint64_t)value }, formatter{ fmt_uint } {}
															   
	arg_wrapper::arg_wrapper(uint32_t value) : value{ (uint64_t)value }, formatter{ fmt_uint } {}
															   
	arg_wrapper::arg_wrapper(uint64_t value) : value{ value }, formatter{ fmt_uint } {}

	arg_wrapper::arg_wrapper(int8_t value) : value{ (int64_t)value }, formatter{ fmt_int } {}

	arg_wrapper::arg_wrapper(int16_t value) : value{ (int64_t)value }, formatter{ fmt_int } {}

	arg_wrapper::arg_wrapper(int32_t value) : value{ (int64_t)value }, formatter{ fmt_int } {}

	arg_wrapper::arg_wrapper(int64_t value) : value{ value }, formatter{ fmt_int } {}

	arg_wrapper::arg_wrapper(float value) : value{ value }, formatter{ fmt_float } {}

	arg_wrapper::arg_wrapper(double value) : value{ value }, formatter{ fmt_double } {}

	arg_wrapper::arg_wrapper(const char* value) : value{ (const void*)value }, formatter{ fmt_cstring } {}

	arg_wrapper::arg_wrapper(const och::utf8_string& value) : value{ (const void*)&value }, formatter{ fmt_utf8_string } {};

	arg_wrapper::arg_wrapper(const och::utf8_view& value) : value{ (const void*)&value }, formatter{ fmt_utf8_view } {};

	arg_wrapper::arg_wrapper(const och::date& value) : value{ (const void*)&value }, formatter{ fmt_date } {};

	arg_wrapper::arg_wrapper(char32_t value) : value{ och::utf8_char(value) }, formatter{ fmt_codepoint } {};

	arg_wrapper::arg_wrapper(const och::utf8_char& value) : value{ value }, formatter{ fmt_codepoint } {};



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////parsed_context//////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	parsed_context::parsed_context(const char* context, const och::range<const och::arg_wrapper> argv) : argv{ argv }
	{
		//[width] [.precision] [rightadj] [~filler] [signmode] [format specifier]}

		if (*context == '#')//Argument Reference
		{
			++context;

			uint32_t width_idx = 0;

			while (*context >= '0' && *context <= '9')
				width_idx = width_idx * 10 + *context++ - '0';

			_ASSERT(width_idx < argv.len());

			width = static_cast<uint16_t>(argv[width_idx].value.u);
		}
		else
		{
			width = 0;
			if (*context >= '0' && *context <= '9')
				while (*context >= '0' && *context <= '9')
					width = width * 10 + *context++ - '0';
		}


		if (*context == '.')
		{
			++context;

			if (*context == '#')
			{
				++context;

				uint32_t precision_idx = 0;

				while (*context >= '0' && *context <= '9')
					precision_idx = precision_idx * 10 + *context++ - '0';

				_ASSERT(precision_idx < argv.len());

				precision = static_cast<uint16_t>(argv[precision_idx].value.u);
			}
			else
			{
				precision = 0;

				while (*context >= '0' && *context <= '9')
					precision = precision * 10 + *context++ - '0';
			}
		}
		else
			precision = (uint16_t)~0;

		flags = 0;

		if (*context == '>')
		{
			flags |= 4;
			++context;
		}
		else if (*context == '<')
			++context;

		if (*context == '~')
		{
			++context;

			filler = utf8_char(context);

			context += filler.get_codeunits();
		}

		if (*context == '+')
		{
			flags |= 1;
			++context;
		}
		else if (*context == '_')
		{
			flags |= 2;
			++context;
		}

		if (*context != '}')
		{
			format_specifier = utf8_char(context);

			context += format_specifier.get_codeunits();
		}

		if (*context != '}')
			raw_context = context;
		else
			raw_context = nullptr;
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////vprint//////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	//{[argindex] [:[width] [.precision] [rightadj] [~filler] [signmode] [formatmode]]}
	void vprint(och::iohandle out, const och::stringview& format, const och::range<const arg_wrapper>& argv)
	{
		uint32_t arg_counter = 0;

		const char* last_fmt_end = format.raw_cbegin();

		const char* curr = format.raw_cbegin();

		while (curr < format.raw_cend())
			if (*curr++ == '{')
			{
				if (*curr == '{')
				{
					++curr;

					continue;
				}
				
				och::write_to_file(out, { last_fmt_end, curr - 1 });

				uint32_t arg_idx;

				if (*curr >= '0' && *curr <= '9')
				{
					arg_idx = *curr++ - '0';

					while (*curr >= '0' && *curr <= '9')
						arg_idx = arg_idx * 10 + *curr++ - '0';
				}
				else
					arg_idx = arg_counter;

				++arg_counter;

				_ASSERT(arg_idx < argv.len());

				_ASSERT(*curr == ':' || *curr == '}');

				if (*curr == ':')
					++curr;

				parsed_context format_context(curr, argv);

				argv[arg_idx].formatter(out, argv[arg_idx].value, format_context);

				while (*curr++ != '}');

				last_fmt_end = curr;
			}

		och::write_to_file(out, { last_fmt_end, curr });
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////print///////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	void print(och::iohandle out, const och::stringview& format)
	{
		och::write_to_file(out, format);
	}

	void print(och::iohandle out, const char* format)
	{
		print(out, och::stringview(format));
	}

	void print(och::iohandle out, const och::utf8_string& format)
	{
		print(out, och::stringview(format));
	}



	void print(const och::filehandle& out, const och::stringview& format)
	{
		print(out.handle, format);
	}

	void print(const och::filehandle& out, const char* format)
	{
		print(out.handle, format);
	}

	void print(const och::filehandle& out, const och::utf8_string& format)
	{
		print(out.handle, format);
	}



	void print(const och::stringview& format)
	{
		print(och::standard_out, format);
	}

	void print(const char* format)
	{
		print(och::standard_out, format);
	}

	void print(const och::utf8_string& format)
	{
		print(och::standard_out, format);
	}
}
