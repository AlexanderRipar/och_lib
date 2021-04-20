﻿#include "och_fmt.h"

#include <cstdint>
#include <intrin.h>

#include "och_fio.h"
#include "och_utf8.h"
#include "och_utf8buf.h"

namespace och
{
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////formatting internals and data///////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	constexpr const char* hex_lower_upper = "0123456789abcdef0123456789ABCDEF";

	constexpr och::stringview invalid_specifier_msg("[[Invalid format-specifier]]");

	constexpr const char* weekdays = "Sunday\0\0\0\0" "Monday\0\0\0\0" "Tuesday\0\0\0" "Wednesday\0" "Thursday\0\0" "Friday\0\0\0\0" "Saturday\0";

	constexpr const char* months = "January\0\0\0"    "February\0\0"   "March\0\0\0\0\0" "April\0\0\0\0\0" "May\0\0\0\0\0\0\0" "June\0\0\0\0\0\0"
		"July\0\0\0\0\0\0" "August\0\0\0\0" "September\0"     "October\0\0\0"   "November\0\0"      "December\0";

	union
	{
		och::iohandle file;
		och::range<char> buffer;
	} curr_output;

	bool is_file_output;
	uint32_t overrun_count;

	och::forward_utf8_buffer<1024> _vprint_buf;

	bool is_rightadj(const parsed_context& context)
	{
		return context.flags & 4;
	}

	uint32_t get_writable(uint32_t wish)
	{
		uint32_t available = static_cast<uint32_t>(curr_output.buffer.len());

		if (available < wish)
		{
			overrun_count += wish - available;
			return available;
		}
		else
			return wish;
	}

	void to_vbuf(const och::stringview& text)
	{
		if (is_file_output)
		{
			if (!_vprint_buf.push(text))
				_vprint_buf.flush(curr_output.file);
		}
		else
		{
			uint32_t bytes = get_writable(text.get_codeunits());

			for (uint32_t i = 0; i != bytes; ++i)
				*curr_output.buffer.beg++ = text.raw_cbegin()[i];
		}
	}

	void to_vbuf(utf8_char c)
	{
		if (is_file_output)
		{
			if (!_vprint_buf.push(c))
				_vprint_buf.flush(curr_output.file);
		}
		else
		{
			uint32_t bytes = get_writable(c.get_codeunits());

			for (uint32_t i = 0; i != bytes; ++i)
				*curr_output.buffer.beg++ = c.cbegin()[i];
		}
	}

	void pad_vbuf(uint32_t text_codepoints, const och::parsed_context& context)
	{
		int32_t filler_cpoints = context.width - text_codepoints;

		if (filler_cpoints < 0)
			return;

		if (is_file_output)
		{
			while (filler_cpoints > 0)
			{
				if (!_vprint_buf.fill(context.filler, filler_cpoints & (_vprint_buf.size - 1)))
					_vprint_buf.flush(curr_output.file);
				filler_cpoints -= _vprint_buf.size - 1;
			}
		}
		else
		{
			int32_t filler_cunits = filler_cpoints * context.filler.get_codeunits();

			uint32_t writable = get_writable(filler_cunits);

			for (uint32_t i = 0; i < writable; i += context.filler.get_codeunits())
				for (uint32_t j = 0; j != context.filler.get_codeunits(); ++j)
					*curr_output.buffer.beg++ = context.filler.cbegin()[j];
		}
	}

	void to_vbuf_with_padding(utf8_char c, const parsed_context& context)
	{
		if (is_rightadj(context))
			pad_vbuf(1, context);

		to_vbuf(c);

		if (!is_rightadj(context))
			pad_vbuf(1, context);
	}

	void to_vbuf_with_padding(const och::stringview& v, const parsed_context& context)
	{
		if (is_rightadj(context))
			pad_vbuf(v.get_codepoints(), context);

		to_vbuf(v);

		if (!is_rightadj(context))
			pad_vbuf(v.get_codepoints(), context);
	}

	char* reserve_vbuf(uint32_t codeunits)
	{
		if (is_file_output)
		{
			return _vprint_buf.reserve((uint16_t)codeunits, curr_output.file);
		}
		else
		{
			uint32_t writable = get_writable(codeunits);

			char* ret = curr_output.buffer.beg;

			curr_output.buffer.beg += writable;

			if (writable != codeunits)
			{
				//Act as if we filled the buffer to make counting of written chars possible
				curr_output.buffer.beg = curr_output.buffer.end;

				return nullptr;
			}

			return ret;
		}
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*////////////////////////////////////////////////formatting helpers/////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	uint32_t log2(uint64_t n)
	{
		uint32_t idx;

		_BitScanReverse64((unsigned long*)&idx, n);

		return idx + 1;
	}

	uint32_t log10(uint64_t n)
	{
		uint32_t log{ 0 };

		while (n >= 1000)
		{
			log += 3;

			n /= 1000;
		}

		if (n >= 100)
			return log + 3;
		else if (n >= 10)
			return log + 2;
		else
			return log + 1;
	}

	uint32_t log16(uint64_t n)
	{
		return (log2(n) + 3) >> 2;
	}

	void h_fmt_two_digit(uint64_t n)
	{
		to_vbuf((char)('0' + n / 10));

		to_vbuf((char)('0' + n % 10));
	}

	void h_fmt_three_digit(uint64_t n)
	{
		to_vbuf((char)('0' + n / 100));
		to_vbuf((char)('0' + (n / 10) % 10));
		to_vbuf((char)('0' + n % 10));
	}

	void h_fmt_decimal(uint64_t n, int32_t digits, char sign = '\0')
	{
		char* curr = reserve_vbuf(digits + (sign != 0)) + digits - (sign == 0);

		if (curr)
		{
			while (n >= 10)
			{
				*curr-- = (char)('0' + n % 10);

				n /= 10;
			}

			*curr-- = (char)('0' + n);

			if (sign)
				*curr = sign;
		}
	}

	void h_fmt_hex(uint64_t value, int32_t digits, const char* hex_charset)
	{
		char* curr = reserve_vbuf(digits) + digits - 1;

		if (curr)
		{
			while (--digits > 0)
			{
				*curr-- = hex_charset[value & 0xF];

				value >>= 4;
			}

			*curr-- = hex_charset[value & 0xF];
		}
	}

	void h_fmt_binary(uint64_t value, int32_t digits)
	{
		char* curr = reserve_vbuf(digits) + digits - 1;

		if (curr)
		{
			while (--digits > 0)
			{
				*curr-- = '0' + (value & 1);

				value >>= 1;
			}

			*curr-- = '0' + (value & 1);
		}
	}

	void h_fmt_integer_base(uint64_t n, const parsed_context& context, uint32_t bit_width, char sign = '\0')
	{
		const char* hex_fmt = hex_lower_upper;
		
		uint32_t digits;
		uint32_t min_binary_width = 0;

		switch (*context.format_specifier.cbegin())
		{
		case '\0':
			{
				uint64_t abs_n = (sign == '-' ? -(int64_t)n : n);

				digits = log10(abs_n) + (sign != 0);

				if (is_rightadj(context))
					pad_vbuf(digits, context);

				h_fmt_decimal(abs_n, digits - (sign != 0), sign);
			}
			break;

		case 'X':
			{
				hex_fmt += 16;
		case 'x':
				digits = log16(n);

				if (digits > (bit_width >> 2))
					digits = bit_width >> 2;

				if (is_rightadj(context))
					pad_vbuf(digits, context);

				h_fmt_hex(n, digits, hex_fmt);
			}
			break;

		case 'B':
			{
				min_binary_width = bit_width;
		case 'b':
				digits = log2(n);

				if (digits < min_binary_width)
					digits = min_binary_width;
				else if (digits > bit_width)
					digits = bit_width;

				if (is_rightadj(context))
					pad_vbuf(digits, context);

				h_fmt_binary(n, digits);
			}
			break;

		default:
			{
				to_vbuf_with_padding(invalid_specifier_msg, context);
			}
			return; //Necessary to avoid final right padding
		}

		if (!is_rightadj(context))
			pad_vbuf(digits, context);
	}

	uint32_t h_get_date_cpoints(const char* fmt, const och::date& d) noexcept
	{
		uint32_t cpoints = 0;

		for (char c = *fmt; c != '}'; c = *++fmt)
			switch (c)
			{
			case 'y':
				cpoints += log10(d.year());
				break;
			case 'Y':
				cpoints += d.year() >= 10000 ? 5 : 4;
				break;
			case 'm':
				cpoints += d.month() >= 10 ? 2 : 1;
				break;
			case 'N':
			{
				const char* monthname = months + (ptrdiff_t)(d.month() - 1) * 10;

				while (*monthname++)
					++cpoints;
			}
			break;
			case 'd':
				cpoints += d.monthday() >= 10 ? 2 : 1;
				break;
			case 'W':
			{
				const char* dayname = weekdays + (ptrdiff_t)d.weekday() * 10;

				while (*dayname++)
					++cpoints;
			}
			break;
			case 'i':
				cpoints += d.hour() >= 10 ? 2 : 1;
				break;
			case 'j':
				cpoints += d.minute() >= 10 ? 2 : 1;
				break;
			case 'k':
				cpoints += d.second() >= 10 ? 2 : 1;
				break;
			case 'l':
				cpoints += d.millisecond() >= 100 ? 3 : d.millisecond() >= 10 ? 2 : 1;
				break;
			case 'D':
			case 'I':
			case 'J':
			case 'K':
			case 'M':
				cpoints += 2;
				break;
			case 'L':
			case 'w':
			case 'n':
				cpoints += 3;
				break;
			case 'u':
				cpoints += d.is_utc() ? 1 : 3;
				break;
			case 'U':
				cpoints += d.is_utc() ? 0 : 2;
				break;
			case 's':
			case 'S':
				if ((c != 's') ^ (d.is_utc())) //Next char is inactive
				{
					if (*++fmt == 'x')
						++fmt;

					while (_is_utf8_surr(fmt[1]))
						++fmt;
				}
				break;
			case 'x':
				c = *++fmt; //Fallthrough...
			default:
				cpoints += !_is_utf8_surr(c);
				break;
			}

		return cpoints;
	}

	char h_get_sign(int64_t n, const parsed_context& context)
	{
		if (n < 0)
			return '-';
		else if (context.flags & 1)
			return '+';
		else if (context.flags & 2)
			return ' ';
		else
			return '\0';
	}

	uint16_t h_parse_fmt_index(const char*& context)
	{
		uint16_t val = 0;

		for (char d = *context; d >= '0' && d <= '9'; d = *++context)
			val = val * 10 + d - '0';

		return val;
	}

	uint16_t h_parse_fmt_index_relative(const char*& context, const och::range<const och::arg_wrapper> argv)
	{
		bool by_arg = *context == '{';

		if (by_arg)
			++context;

		uint16_t val = h_parse_fmt_index(context);

		if (by_arg)
		{
			_ASSERT(*context == '}');

			++context;

			_ASSERT(val < argv.len());

			val = argv[val].value.u16;
		}

		return val;
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////formatting functions////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	void fmt_uint64(type_union arg_value, const parsed_context& context)
	{
		uint64_t value = arg_value.u64;

		h_fmt_integer_base(value, context, 64);
	}

	void fmt_int64(type_union arg_value, const parsed_context& context)
	{
		int64_t value = arg_value.i64;

		h_fmt_integer_base(value, context, 64, h_get_sign(value, context));
	}

	void fmt_uint32(type_union arg_value, const parsed_context& context)
	{
		uint32_t value = arg_value.u32;

		h_fmt_integer_base(value, context, 32);
	}

	void fmt_int32(type_union arg_value, const parsed_context& context)
	{
		int32_t value = arg_value.i32;

		h_fmt_integer_base(value, context, 32, h_get_sign(value, context));
	}

	void fmt_uint16(type_union arg_value, const parsed_context& context)
	{
		uint16_t value = arg_value.u16;

		h_fmt_integer_base(value, context, 16);
	}

	void fmt_int16(type_union arg_value, const parsed_context& context)
	{
		int16_t value = arg_value.i16;

		h_fmt_integer_base(value, context, 16, h_get_sign(value, context));
	}

	void fmt_uint8(type_union arg_value, const parsed_context& context)
	{
		uint8_t value = arg_value.u8;

		h_fmt_integer_base(value, context, 8);
	}

	void fmt_int8(type_union arg_value, const parsed_context& context)
	{
		int8_t value = arg_value.i8;

		h_fmt_integer_base(value, context, 8, h_get_sign(value, context));
	}

	void fmt_utf8_view(type_union arg_value, const parsed_context& context)
	{
		och::utf8_view value = *(const och::utf8_view*)arg_value.ptr;

		if (value.get_codepoints() > context.precision)
			value = value.subview(0, context.precision);

		to_vbuf_with_padding(value, context);
	}

	void fmt_utf8_string(type_union arg_value, const parsed_context& context)
	{
		och::utf8_view value(*(const och::utf8_string*)arg_value.ptr);

		fmt_utf8_view((const void*)&value, context);
	}

	void fmt_cstring(type_union arg_value, const parsed_context& context)
	{
		och::utf8_view value(reinterpret_cast<const char*>(arg_value.ptr));

		fmt_utf8_view((const void*)&value, context);
	}

	void fmt_codepoint(type_union arg_value, const parsed_context& context)
	{
		och::utf8_char value = arg_value.utf_c;

		to_vbuf_with_padding(value, context);
	}

	//TODO implement
	void fmt_float(type_union arg_value, const parsed_context& context)
	{
		arg_value;

		to_vbuf_with_padding(och::stringview("[[fmt_float is not yet implemented]]"), context);

		//     ->   base 10 decimal
		// e   ->   base 10 scientific ([+ -]N.NNNNe+-NNNN)
		// x,X ->   base 16 decimal. x uses lowercase letters, H uppercase
		// h,H ->   base 16 scientific ([+ -]N.NNNNp+-NNNN). h uses lowercase letters, H uppercase
		// b   ->   bitpattern
		// B   ->   bitpattern with ' separating sign, exponent and mantissa

		//char buf[64];
		//
		//char* curr = buf;
		//
		//const uint32_t value = (uint32_t)arg_value.u;
		//
		//const bool is_negative = value & 0x8000'0000;
		//const int8_t exponent = (int8_t)(((value & 0x7F80'0000) >> 23) - 127);
		//const uint32_t mantissa = value & 0x007F'FFFF;
		//
		//if /*TODO Implement*/ (context.format_specifier == '\0')
		//{
		//
		//}
		//else if (context.format_specifier == 'b')
		//{
		//	uint32_t mask = 0x8000'0000;
		//
		//	while (mask)
		//	{
		//		*curr++ = '0' + ((value & mask) == mask);
		//		mask >>= 1;
		//	}
		//}
		//else if (context.format_specifier == 'B')
		//{
		//	uint32_t mask = 0x8000'0000;
		//
		//	*curr++ = '0' + ((value & mask) == mask);
		//	mask >>= 1;
		//
		//	*curr++ = '\'';
		//
		//	for (uint32_t i = 0; i != 8; ++i)
		//	{
		//		*curr++ = '0' + ((value & mask) == mask);
		//		mask >>= 1;
		//	}
		//
		//	*curr++ = '\'';
		//
		//	while (mask)
		//	{
		//		*curr++ = '0' + ((value & mask) == mask);
		//		mask >>= 1;
		//	}
		//}
		//else if /*TODO Implement*/ (context.format_specifier == 'x' || context.format_specifier == 'X')
		//{
		//
		//}
		//else if/*TODO Implement*/ (context.format_specifier == 'e')
		//{
		//
		//}
		//else if /*TODO Rounding*/ (context.format_specifier == 'h' || context.format_specifier == 'H')
		//{
		//	const char* hex = context.format_specifier == 'h' ? hex_lower_upper : hex_lower_upper + 16;
		//
		//	int8_t hex_exponent = exponent >> 2;
		//
		//	const uint32_t hex_mantissa = ((1 << 23) | mantissa) << (exponent & 3);
		//
		//	const uint32_t predec = hex_mantissa >> 23;
		//
		//	const uint32_t postdec = hex_mantissa & ((1 << 23) - 1);
		//
		//	if (is_negative)
		//		*curr++ = '-';
		//	else if (context.flags & 1)
		//		*curr++ = '+';
		//	else if (context.flags & 2)
		//		*curr++ = ' ';
		//
		//	*curr++ = hex[predec];
		//
		//	const char* const decimal_point = curr;
		//
		//	*curr++ = '.';
		//
		//	uint32_t shift = 19;
		//
		//	while (shift >= 4)
		//	{
		//		*curr++ = hex[(postdec >> shift) & 0xF];
		//		shift -= 4;
		//	}
		//
		//	*curr++ = hex[postdec & 3];
		//
		//	while (curr[-1] == '0' && curr[-2] != '.')
		//		--curr;
		//
		//	if (context.precision != 0xFFFF)
		//	{
		//		//Cut off until precision is equal to curr - decimal_point
		//		while (curr - decimal_point - 1 > context.precision)
		//			--curr;
		//
		//		//Add '0' until precision is equal to curr - decimal_point
		//		while (curr - decimal_point - 1 < context.precision)
		//			*curr++ = '0';
		//	}
		//
		//	*curr++ = 'p';
		//
		//	if (hex_exponent < 0)
		//	{
		//		*curr++ = '-';
		//		hex_exponent = -hex_exponent;
		//	}
		//
		//	if (hex_exponent > 0xF)
		//		*curr++ = hex[hex_exponent >> 4];
		//
		//	*curr++ = hex[hex_exponent & 0xF];
		//}
		//else
		//{
		//	och::write_with_padding(out, "[[Invalid format-specifier", context);
		//	return;
		//}
		//
		//och::write_with_padding(out, och::stringview(buf, (uint32_t)(curr - buf), (uint32_t)(curr - buf)), context);
	}

	//TODO implement
	void fmt_double(type_union arg_value, const parsed_context& context)
	{
		arg_value;

		to_vbuf_with_padding(och::stringview("[[fmt_double is not yet implemented]]"), context);
	}

	//TODO improve (Not happy)
	void fmt_date(type_union arg_value, const parsed_context& context)
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
		//          | Capital letters (except U) indicate leading zeroes, or full names                         |
		//          +-------------------------------------------------------------------------------------------+

		const och::date& value = *reinterpret_cast<const och::date*>(arg_value.ptr);

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
			to_vbuf_with_padding(invalid_specifier_msg, context);
			return;
		}

		uint32_t cpoints = h_get_date_cpoints(format, value);

		if (is_rightadj(context))
			pad_vbuf(cpoints, context);

#define OCH_FMT_2DIGIT(x) if(c & 0x20) { if(x >= 10) to_vbuf((char)('0' + x / 10)); to_vbuf((char)('0' + x % 10)); } else { to_vbuf((char)('0' + x / 10)); to_vbuf((char)('0' + x % 10)); }

		for (char c = *format; c != '}'; c = *++format)
			switch (c)
			{
			case 'y':
			{
				uint16_t y = value.year();

				char* curr = reserve_vbuf(log10(y)) + log10(y) - 1;

				if (curr)
				{
					while (y >= 10)
					{
						*curr-- = (char)('0' + y % 10);

						y /= 10;
					}

					*curr = (char)('0' + y);
				}
			}
			break;

			case 'Y':
			{
				uint16_t y = value.year();

				char* curr = reserve_vbuf(4 + (y >= 10000));

				if (curr)
				{
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
				}
			}
			break;

			case 'm':
			case 'M':
				OCH_FMT_2DIGIT(value.month());
				break;

			case 'n':
			{
				to_vbuf(months[(value.month() - 1) * 10]);
				to_vbuf(months[(value.month() - 1) * 10 + 1]);
				to_vbuf(months[(value.month() - 1) * 10 + 2]);
			}
			break;

			case 'N':
			{
				const char* monthname = months + (ptrdiff_t)(value.month() - 1) * 10;

				while (*monthname)
					to_vbuf(*monthname++);
			}
			break;

			case 'd':
			case 'D':
				OCH_FMT_2DIGIT(value.monthday());
				break;

			case 'w':
			{
				to_vbuf(weekdays[value.weekday() * 10]);
				to_vbuf(weekdays[value.weekday() * 10 + 1]);
				to_vbuf(weekdays[value.weekday() * 10 + 2]);
			}
			break;

			case 'W':
			{
				const char* dayname = weekdays + (ptrdiff_t)value.weekday() * 10;

				while (*dayname)
					to_vbuf(*dayname++);
			}
			break;

			case 'i':
			case 'I':
				OCH_FMT_2DIGIT(value.hour());
				break;

			case 'j':
			case 'J':
				OCH_FMT_2DIGIT(value.minute());
				break;

			case 'k':
			case 'K':
				OCH_FMT_2DIGIT(value.second());
				break;

			case 'l':
			{
				if (value.millisecond() >= 100)
					to_vbuf((char)('0' + value.millisecond() / 100));
				if (value.millisecond() >= 10)
					to_vbuf((char)('0' + (value.millisecond() / 10) % 10));
				to_vbuf((char)('0' + value.millisecond() % 10));
			}
			break;

			case 'L':
			{
				to_vbuf((char)('0' + (char)(value.millisecond() / 100)));
				to_vbuf((char)('0' + (value.millisecond() / 10) % 10));
				to_vbuf((char)('0' + value.millisecond() % 10));
			}
			break;

			case 'u':
			{
				if (value.is_utc())
				{
					to_vbuf('Z');

					break;
				}

				to_vbuf(value.utc_offset_is_negative() ? '-' : '+');

				uint16_t h = value.utc_offset_hours();

				to_vbuf((char)('0' + h / 10));
				to_vbuf((char)('0' + h % 10));
			}
			break;

			case 'U':
			{
				if (value.is_utc())
					break;

				uint16_t m = value.utc_offset_minutes();

				to_vbuf((char)('0' + m / 10));
				to_vbuf((char)('0' + m % 10));
			}
			break;

			case 's':
			case 'S':
			{
				if ((c != 's') ^ (value.is_utc())) //Next char is inactive
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
				to_vbuf(c);
				utf8_cpoints -= _is_utf8_surr(c);
				break;
			}

#undef OCH_FMT_2DIGIT

		if (!is_rightadj(context))
			pad_vbuf(cpoints, context);
	}

	//TODO improve (Not happy)
	void fmt_timespan(type_union arg_value, const parsed_context& context)
	{
		//     ->   seconds (with format specifier)
		// d   ->   days
		// h   ->   hours
		// min ->   minutes
		// s   ->   seconds
		// ms  ->   milliseconds
		// us,
		// sμ  ->  microseconds
		// l   ->  combination of all units which are greater than zero for the given timespan, separated by commas and with
		//         their SI-symbols. Uppercase also prints units that are zero.
		// x   ->  custom format, composed of the aforementioned letters. Other formatting rules are equivalent to fmt_date
		//
		// Uppercase letters also write the fitting SI-symbol.
		//
		// For microseconds, U writes [n...]u, while μ writes [n...]μ

		och::timespan value;

		value.val = arg_value.i64;

		char sign = h_get_sign(value.val, context);

		if (value.val < 0)
			value.val = -value.val;

		char32_t c = context.format_specifier.codepoint();

		uint32_t utf_surr_count = 0;

		const char* specifier = nullptr;

		int64_t time_value;

		switch (c)
		{
		case '\0':
			if (value.val < 10000ll && value.val > -10000ll)
			{
				if (is_rightadj(context))
					pad_vbuf(5 + (sign != 0), context);

				if (sign)
					to_vbuf(sign);

				h_fmt_three_digit(value.microseconds());
				to_vbuf('u');
				to_vbuf('s');

				if (!is_rightadj(context))
					pad_vbuf(5 + (sign != 0), context);
			}
			else if (value.val < 10000000ll && value.val > -10000000ll)
			{
				if (is_rightadj(context))
					pad_vbuf(9 + (sign != 0), context);

				if (sign)
					to_vbuf(sign);

				h_fmt_three_digit(value.milliseconds());

				to_vbuf('.');

				h_fmt_three_digit(value.microseconds() % 1000);

				to_vbuf('m');
				to_vbuf('s');

				if (!is_rightadj(context))
					pad_vbuf(9 + (sign != 0), context);
			}
			else if (value.val < 600000000ll && value.val > -600000000ll)
			{
				if (is_rightadj(context))
					pad_vbuf(7 + (sign != 0), context);

				h_fmt_two_digit(value.seconds());

				to_vbuf('.');

				h_fmt_three_digit(value.milliseconds() % 1000);

				to_vbuf('s');

				if (!is_rightadj(context))
					pad_vbuf(7 + (sign != 0), context);
			}
			else if (value.val < 60 * 600000000ll && value.val > -60 * 600000000ll)
			{
				if (is_rightadj(context))
					pad_vbuf(12 + (sign != 0), context);

				h_fmt_two_digit(value.minutes());

				to_vbuf(':');

				h_fmt_two_digit(value.seconds() % 60);

				to_vbuf('.');

				h_fmt_three_digit(value.milliseconds() % 1000);

				to_vbuf('m');
				to_vbuf('i');
				to_vbuf('n');

				if (!is_rightadj(context))
					pad_vbuf(12 + (sign != 0), context);
			}
			else
			{
				int64_t days = value.days();

				uint32_t log10_d = log10(days);

				uint32_t chars = 6 + (days ? log10_d + 4 : 0) + (sign != 0);

				if (is_rightadj(context))
					pad_vbuf(chars, context);

				if (sign)
					to_vbuf(sign);

				if (days)
				{
					h_fmt_decimal(value.days(), log10_d);

					to_vbuf('d');
					to_vbuf(' ');
					to_vbuf('+');
					to_vbuf(' ');
				}

				h_fmt_two_digit(value.hours() % 24);

				to_vbuf(':');

				h_fmt_two_digit(value.minutes() % 60);

				if (!is_rightadj(context))
					pad_vbuf(chars, context);
			}
			return;

		case 'D':
			specifier = "d";
		case 'd':
			time_value = value.days();
			break;

		case 'H':
			specifier = "h";
		case 'h':
			time_value = value.hours();
			break;

		case 'M':
			if (!context.raw_context)
			{
				to_vbuf_with_padding(invalid_specifier_msg, context);

				return;
			}
			else if (context.raw_context[0] == 'S')
			{
				specifier = "ms";

				time_value = value.milliseconds();
			}
			else if (context.raw_context[0] == 'I' && context.raw_context[1] == 'N')
			{
				specifier = "min";

				time_value = value.minutes();
			}
			else
			{
				to_vbuf_with_padding(invalid_specifier_msg, context);

				return;
			}
			break;
		case 'm':
			if (!context.raw_context)
			{
				to_vbuf_with_padding(invalid_specifier_msg, context);

				return;
			}
			else if (context.raw_context[0] == 's')
				time_value = value.milliseconds();
			else if (context.raw_context[0] == 'i' && context.raw_context[1] == 'n')
				time_value = value.minutes();
			else
			{
				to_vbuf_with_padding(invalid_specifier_msg, context);

				return;
			}
			break;

		case 'S':
			specifier = "s";
		case 's':
			time_value = value.seconds();
			break;

		case U'μ':
			specifier = u8"μs";
			time_value = value.microseconds();
			++utf_surr_count;
			break;

		case 'U':
			specifier = "us";
		case 'u':
			time_value = value.microseconds();
			break;

		case 'L':
		{
			//[+ -][D+d, ]HH:MM:SS.MMM.UUU

			int64_t days = value.days();

			uint32_t log10_d = log10(days);

			uint32_t day_chars = days ? 4 + log10_d : 0;

			uint32_t chars = 17 + day_chars + (sign != 0);

			if (is_rightadj(context))
				pad_vbuf(chars, context);

			if (sign)
				to_vbuf(sign);

			if (days)
			{
				h_fmt_decimal(value.days(), log10_d);

				to_vbuf('d');
				to_vbuf(' ');
				to_vbuf('+');
				to_vbuf(' ');
			}

			h_fmt_two_digit(value.hours() % 24);

			to_vbuf(':');

			h_fmt_two_digit(value.minutes() % 60);

			to_vbuf(':');

			h_fmt_two_digit(value.seconds() % 60);

			to_vbuf('.');

			h_fmt_three_digit(value.milliseconds() % 1000);

			to_vbuf('.');

			h_fmt_three_digit(value.microseconds() % 1000);

			to_vbuf('h');

			if (!is_rightadj(context))
				pad_vbuf(chars, context);
		}
		return;

		case 'l':
			to_vbuf_with_padding(och::stringview("[[fmt_timespan - specifier 'l' is not yet implemented]]"), context);

			{
				////[+ -][D+d, ][HHh, ][MMmin, ][SSs, ][MMMms, ]UUUus
				//int64_t days = value.days();
				//
				//uint32_t log10_d = log10(days);
				//
				//uint32_t chars = 5; //chars for us;
				//
				//if (value.val < 10000000ll)
				//{
				//	chars += 4;
				//}
				//
				//
				//*curr-- = 's';
				//*curr-- = 'u';
				//
				//uint64_t us = value.microseconds() % 1000;
				//if(us)
				//	curr = _fmt_reverse_itos(curr, us);
				//
				//if (value.val < 10000llu) break;
				//
				//*curr-- = ' ';
				//*curr-- = ',';
				//
				//*curr-- = 's';
				//*curr-- = 'm';
				//
				//uint64_t ms = value.milliseconds() % 1000;
				//if(ms)
				//	curr = _fmt_reverse_itos(curr, ms);
				//
				//if (value.val < 10000000llu) break;
				//
				//*curr-- = ' ';
				//*curr-- = ',';
				//
				//*curr-- = 's';
				//
				//uint64_t s = value.seconds() % 60;
				//if (ms)
				//	curr = _fmt_reverse_itos(curr, s);
				//
				//if (value.val < 600000000llu) break;
				//
				//*curr-- = ' ';
				//*curr-- = ',';
				//
				//*curr-- = 'n';
				//*curr-- = 'i';
				//*curr-- = 'm';
				//
				//uint64_t min = value.minutes() % 60;
				//if (min)
				//	curr = _fmt_reverse_itos(curr, min);
				//
				//if (value.val < 60 * 600000000llu) break;
				//
				//*curr-- = ' ';
				//*curr-- = ',';
				//
				//*curr-- = 'h';
				//uint64_t h = value.hours() % 24;
				//if (h)
				//	curr = _fmt_reverse_itos(curr, h);
				//
				//if (value.val < 24 * 60 * 600000000llu) break;
				//
				//*curr-- = ' ';
				//*curr-- = ',';
				//
				//*curr-- = 'd';
				//curr = _fmt_reverse_itos(curr, value.days());
			}
			return;

		case 'x':

			to_vbuf_with_padding(och::stringview("[[fmt_timespan - specifier 'x' is not yet implemented]]"), context);
			return;

		default:
			to_vbuf_with_padding(invalid_specifier_msg, context);
			return;
		}

		uint32_t log10_v = log10(time_value);

		uint32_t chars = log10_v + (sign != 0) - utf_surr_count;

		uint32_t specifier_len = 0;
		if (specifier)
			for (uint32_t i = 0; specifier[i]; ++i)
				++specifier_len;

		chars += specifier_len;

		if (is_rightadj(context))
			pad_vbuf(chars, context);

		h_fmt_decimal(time_value, log10_v, sign);

		if (specifier)
			to_vbuf(och::stringview(specifier, specifier_len, 1));

		if (!is_rightadj(context))
			pad_vbuf(chars, context);
	}

	//TODO implement
	void fmt_highres_timespan(type_union arg_value, const parsed_context& context)
	{
		arg_value;

		context;

		to_vbuf_with_padding(och::stringview("[[fmt_highres_timespan is not yet implemented]]"), context);

		////     ->   microseconds (with format specifier)
		//// ns  ->   nanoseconds
		//// us,
		//// μs  ->   microseconds
		//// ms  ->   milliseconds
		//// s   ->   seconds
		////
		//// Uppercase letters also write the fitting SI-symbol.
		////
		//// For microseconds, U writes [n...]u, while μ writes [n...]μ
		//
		//char32_t c = context.format_specifier.codepoint();
		//
		//uint32_t utf_surr_count = 0;
		//
		//och::highres_timespan value;
		//
		//value.val = arg_value.i;
		//
		//char buf[64];
		//char* curr = buf + 63;
		//
		//switch (c)
		//{
		//case '\0':
		//{
		//	*curr-- = 's';
		//
		//	if (value.microseconds() >= 1'000'000 || value.microseconds() <= -1'000'000)
		//		curr = _fmt_reverse_itos(curr, value.seconds());
		//	else if (value.microseconds() >= 1000)
		//	{
		//		*curr-- = 'm';
		//
		//		curr = _fmt_reverse_itos(curr, value.milliseconds());
		//	}
		//	else
		//	{
		//		*curr-- = 'u';
		//
		//		curr = _fmt_reverse_itos(curr, value.microseconds());
		//	}
		//}
		//break;
		//case 'S':
		//	*curr-- = 's';
		//case 's':
		//	curr = _fmt_reverse_itos(curr, value.seconds());
		//	break;
		//case U'μ':
		//{
		//	*curr-- = '\xBC';//μ in utf-8
		//
		//	*curr-- = '\xCE';
		//
		//	curr = _fmt_reverse_itos(curr, value.microseconds());
		//
		//	++utf_surr_count;
		//}
		//break;
		//case 'U':
		//{
		//	*curr-- = 's';
		//	*curr-- = 'u';
		//}
		//case 'u':
		//	curr = _fmt_reverse_itos(curr, value.microseconds());
		//	break;
		//case 'l':
		//{
		//	curr = _fmt_reverse_itos(curr, value.microseconds() % 1000);
		//
		//	*curr-- = '.';
		//	
		//	curr = _fmt_reverse_itos(curr, value.milliseconds() % 1000);
		//
		//	*curr-- = '.';
		//
		//	curr = _fmt_reverse_itos(curr, value.seconds());
		//}
		//break;
		//case 'x':
		//{
		//	och::write_with_padding(out, "[[Format-specifier 'x' not yet implemented]]", context);
		//
		//	return;
		//}
		//break;
		//default:
		//{
		//	och::write_with_padding(out, "[[Invalid format-specifier]]", context);
		//
		//	return;
		//}
		//break;
		//}
		//
		//++curr;
		//
		//write_with_padding(out, och::stringview(curr, (uint32_t)(curr - buf), (uint32_t)(curr - buf - utf_surr_count)), context);
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*////////////////////////////////////////////////////arg_wrapper////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	arg_wrapper::arg_wrapper(uint64_t value) : value{ value }, formatter{ fmt_uint64 } {}

	arg_wrapper::arg_wrapper(int64_t value) : value{ value }, formatter{ fmt_int64 } {}

	arg_wrapper::arg_wrapper(uint32_t value) : value{ value }, formatter{ fmt_uint32 } {}

	arg_wrapper::arg_wrapper(int32_t value) : value{ value }, formatter{ fmt_int32 } {}

	arg_wrapper::arg_wrapper(uint16_t value) : value{ value }, formatter{ fmt_uint16 } {}

	arg_wrapper::arg_wrapper(int16_t value) : value{ value }, formatter{ fmt_int16 } {}

	arg_wrapper::arg_wrapper(uint8_t value) : value{ value }, formatter{ fmt_uint8 } {}

	arg_wrapper::arg_wrapper(int8_t value) : value{ value }, formatter{ fmt_int8 } {}

	arg_wrapper::arg_wrapper(float value) : value{ value }, formatter{ fmt_float } {}

	arg_wrapper::arg_wrapper(double value) : value{ value }, formatter{ fmt_double } {}

	arg_wrapper::arg_wrapper(const char* value) : value{ (const void*)value }, formatter{ fmt_cstring } {}

	arg_wrapper::arg_wrapper(const och::utf8_string& value) : value{ (const void*)&value }, formatter{ fmt_utf8_string } {}

	arg_wrapper::arg_wrapper(const och::utf8_view& value) : value{ (const void*)&value }, formatter{ fmt_utf8_view } {}

	arg_wrapper::arg_wrapper(const och::date& value) : value{ (const void*)&value }, formatter{ fmt_date } {}

	arg_wrapper::arg_wrapper(char32_t value) : value{ och::utf8_char(value) }, formatter{ fmt_codepoint } {}

	arg_wrapper::arg_wrapper(char value) : value{ och::utf8_char(value) }, formatter{ fmt_codepoint } {}

	arg_wrapper::arg_wrapper(const och::utf8_char& value) : value{ value }, formatter{ fmt_codepoint } {}

	arg_wrapper::arg_wrapper(och::timespan value) : value{ value.val }, formatter{ fmt_timespan } {}

	arg_wrapper::arg_wrapper(och::highres_timespan value) : value{ value.val }, formatter{ fmt_highres_timespan } {}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////parsed_context//////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	parsed_context::parsed_context(const char*& context, const och::range<const och::arg_wrapper> argv) : argv{ argv }
	{
		//[width] [.precision] [rightadj] [~filler] [signmode] [format specifier]}

		width = h_parse_fmt_index_relative(context, argv);

		if (*context == '.')
		{
			++context;

			precision = h_parse_fmt_index_relative(context, argv);
		}
		else
			precision = 0xFFFF;

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
			filler = utf8_char(++context);

			context += filler.get_codeunits();
		}
		else
			filler = ' ';

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
		else
			format_specifier = '\0';

		if (*context != '}')
			raw_context = context;
		else
			raw_context = nullptr;

		for (int32_t opening_bracket_cnt = 1; opening_bracket_cnt; ++context)
			if (*context == '{')
				++opening_bracket_cnt;
			else if (*context == '}')
				--opening_bracket_cnt;
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////vprint//////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	//{[argindex] [:[width] [.precision] [rightadj] [~filler] [signmode] [formatmode]]}
	uint32_t vprint(och::iohandle out, const och::stringview& format, const och::range<const arg_wrapper>& argv, uint32_t buffer_sz)
	{
		char* const initial_buffer_start = buffer_sz == 0xFFFF'FFFF ? nullptr : static_cast<char*>(out.ptr);

		is_file_output = buffer_sz == 0xFFFF'FFFF;

		curr_output.buffer = { static_cast<char*>(out.ptr), buffer_sz - 1 };

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

				to_vbuf(och::stringview(last_fmt_end, static_cast<uint32_t>(curr - 1 - last_fmt_end), 1));

				uint32_t arg_idx;

				arg_idx = *curr >= '0' && *curr <= '9' ? h_parse_fmt_index(curr) : arg_counter;

				++arg_counter;

				_ASSERT(arg_idx < argv.len());

				_ASSERT(*curr == ':' || *curr == '}');

				if (*curr == ':')
					++curr;

				parsed_context format_context(curr, argv);

				argv[arg_idx].formatter(argv[arg_idx].value, format_context);

				last_fmt_end = curr;
			}

		to_vbuf(och::stringview(last_fmt_end, static_cast<uint32_t>(curr - last_fmt_end), 1));

		if (is_file_output)
		{
			_vprint_buf.flush(out);

			return 0;
		}
		else
		{
			to_vbuf(utf8_char('\0'));

			return static_cast<uint32_t>(curr_output.buffer.beg - initial_buffer_start + overrun_count);
		}
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////print///////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	void print(och::iohandle out, const och::stringview& format)
	{
		och::write_to_file(out, och::range(format.raw_cbegin(), format.raw_cend()));
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



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////sprint//////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	uint32_t sprint(range<char> buf, const stringview& format)
	{
		uint32_t wish = format.get_codeunits();

		if (wish <= buf.len())
		{
			for (uint32_t i = 0; i != wish; ++i)
				buf[i] = format.raw_cbegin()[i];
		}

		return wish;
	}

	uint32_t sprint(range<char> buf, const char* format)
	{
		return sprint(buf, och::stringview(format));
	}

	uint32_t sprint(range<char> buf, const utf8_string& format)
	{
		return sprint(buf, och::stringview(format));
	}
}
