#include "och_fmt.h"

#include <cstdint>

#include "och_range.h"
#include "och_fio.h"

namespace och
{
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////formatting functions////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	
	void write_with_padding(och::iohandle out, och::stringview text, och::utf8_char filler, uint32_t to_write, bool is_rightadj)
	{
		uint32_t written = (uint32_t)text.get_codepoints();

		if (is_rightadj)
			while (written++ < to_write)
				och::write_to_file(out, { filler.cbegin(), filler.cend() });

		och::write_to_file(out, { text.raw_cbegin(), text.raw_cend() });

		if (!is_rightadj)
			while (written++ < to_write)
				och::write_to_file(out, { filler.cbegin(), filler.cend() });
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

	void fmt_int(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		int64_t value = arg_value.i;

		char buf[20];

		char* curr = buf + 19;

		bool is_negative = false;

		if (value < 0)
		{
			is_negative = true;

			value = -value;
		}

		curr = reverse_itos(curr, value);

		if (is_negative)
			*curr = '-';
		else if (context.flags & 1)
			*curr = '+';
		else if (context.flags & 2)
			*curr = ' ';
		else
			++curr;

		write_with_padding(out, och::stringview(curr, (uint32_t)(buf - curr + 20), (uint32_t)(buf - curr + 20)), context.filler, context.width, context.flags & 4);
	}

	void fmt_uint(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		uint64_t value = arg_value.u;

		char buf[20];

		char* curr = buf + 19;

		curr = reverse_itos(curr, value) + 1;

		write_with_padding(out, och::stringview(curr, (uint32_t)(buf - curr + 20), (uint32_t)(buf - curr + 20)), context.filler, context.width, context.flags & 4);
	}

	void fmt_utf8_view(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		och::utf8_view value = *(const och::utf8_view*)arg_value.p;

		if (value.get_codepoints() > context.precision)
			value = value.subview(0, context.precision);

		write_with_padding(out, value, context.filler, context.width, context.flags & 4);
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

		write_with_padding(out, och::stringview(value.cbegin(), value.get_codeunits(), 1), context.filler, context.width, context.flags & 4);
	}

	void fmt_float(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		//constexpr och::stringview nan_text("-nan");
		//constexpr och::stringview inf_text("-inf");
		//
		//uint32_t value = (uint32_t)arg_value.u;
		//
		//if (context.format_specifier == 'b');
		//
		//const bool is_negative = value & 0x8000'0000;
		//const int8_t exponent = (int8_t)(((value & 0x7F80'0000) >> 23) - 127);
		//const uint32_t mantissa = value & 0x007F'FFFF;
		//
		//och::stringview text("[[fmt_float is not yet implemented]]");
		//
		//if (exponent == 0x7F80'0000)//infinity or nan
		//	if (mantissa)
		//		text = och::stringview(nan_text.beg + !is_negative, nan_text.end);
		//	else
		//		text = och::stringview(inf_text.beg + !is_negative, inf_text.end);
		//else if (!exponent)//denorm or zero
		//	if (mantissa)
		//		text = "Denorm";
		//	else
		//		text = "0";
		//else//normalized number
		//{
		//	char buf[64];
		//
		//	char* curr = buf + 63;
		//
		//	
		//}
		//
		//write_ascii_with_padding(out, text, och::stringview(context.filler.utf8, context.filler.get_codeunits()), context.width, context.flags & 4);
	}

	void fmt_double(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		och::write_to_file(out, "[[fmt_double is not yet implemented]]");
	}

	void fmt_date(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		// y -> year
		// m -> month, w/o leading zero
		// n -> short monthname (Jan, Feb, ...)
		// d -> monthday, w/o leading zero
		// w -> short weekday (Mon, Tue, ...)
		// i -> hour, w/o leading zero
		// j -> minute, w/o leading zero
		// k -> second, w/o leading zero
		// l -> millisecond, three digits
		//
		// u -> utc-offset hours (+-hh)
		// U -> utc-offset minutes (mm)
		//
		// Capital letters indicate leading zeroes, or extended name
		auto date_formatter = [](char* out, const och::date& in, const char* format, uint32_t& utf8_cpoints) -> char*
		{
			constexpr const char* weekdays = "Sunday\0\0\0\0" "Monday\0\0\0\0" "Tuesday\0\0\0" "Wednesday\0" "Thursday\0\0" "Friday\0\0\0\0" "Saturday\0";
			constexpr const char* months = "January\0\0\0" "February\0\0" "March\0\0\0\0\0" "April\0\0\0\0\0" "May\0\0\0\0\0\0\0" "June\0\0\0\0\0\0" 
				                           "July\0\0\0\0\0\0" "August\0\0\0\0" "September\0" "October\0\0\0" "November\0\0" "December\0";

			bool last_char_was_surrogate = false;

			for (char c = *format; c != '}'; c = *++format)
			{
				char* before = out;

				switch (c)
				{
				case 'y':
				{
					uint16_t y = in.year();

					int32_t idx = y >= 10000 ? 5 : y >= 1000 ? 4 : y >= 100 ? 3 : y >= 10 ? 2 : 1;

					out += idx;

					for(int32_t i = 1; i != idx; ++i)
					{
						out[-i] = '0' + y % 10;
						y /= 10;
					}

					out[-idx] = '0' + (char)y;
				}
				break;
				case 'Y':
				{
					uint16_t y = in.year();

					if (y >= 10000)
					{
						*out++ = '0' + (char)(y / 10000);
						y /= 10;
					}

					out[3] = '0' + y % 10;
					y /= 10;
					out[2] = '0' + y % 10;
					y /= 10;
					out[1] = '0' + y % 10;
					y /= 10;
					out[0] = '0' + (char)y;

					out += 4;
				}
					break;
				case 'm':
				{
					if (in.month() >= 10)
					{
						*out++ = '1';
						*out++ = '0' + (char)(in.month() - 10);
					}
					else
						*out++ = '0' + (char)(in.month());
				}
					break;
				case 'M':
				{
					*out++ = '0' + (char)(in.month() / 10);
					*out++ = '0' + in.month() % 10;
				}
					break;
				case 'n':
				{
					*out++ = months[(in.month() - 1) * 10    ];
					*out++ = months[(in.month() - 1) * 10 + 1];
					*out++ = months[(in.month() - 1) * 10 + 2];
				}
					break;
				case 'N':
				{
					const char* monthname = months + (ptrdiff_t)(in.month() - 1) * 10;

					while (*monthname)
						*out++ = *monthname++;
				}
					break;
				case 'd':
				{
					if (in.monthday() >= 10)
						*out++ = '0' + (char)(in.monthday() / 10);

					*out++ = '0' + in.monthday() % 10;
				}
					break;
				case 'D':
				{
					*out++ = '0' + (char)(in.monthday() / 10);
					*out++ = '0' + in.monthday() % 10;
				}
					break;
				case 'w':
				{
					*out++ = weekdays[in.weekday() * 10    ];
					*out++ = weekdays[in.weekday() * 10 + 1];
					*out++ = weekdays[in.weekday() * 10 + 2];

				}
					break;
				case 'W':
				{
					const char* dayname = weekdays + (ptrdiff_t)in.weekday() * 10;

					while (*dayname)
						*out++ = *dayname++;
				}
					break;
				case 'i':
				{
					if (in.hour() >= 10)
						*out++ = '0' + (char)(in.hour() / 10);

					*out++ = '0' + in.hour() % 10;
				}
					break;
				case 'I':
				{
					*out++ = '0' + (char)(in.hour() / 10);
					*out++ = '0' + in.hour() % 10;
				}
					break;
				case 'j':
				{
					if (in.minute() >= 10)
						*out++ = '0' + (char)(in.minute() / 10);

					*out++ = '0' + in.minute() % 10;
				}
					break;
				case 'J':
				{
					*out++ = '0' + (char)(in.minute() / 10);
					*out++ = '0' + in.minute() % 10;
				}
					break;
				case 'k':
				{
					if (in.second() >= 10)
						*out++ = '0' + (char)(in.second() / 10);

					*out++ = '0' + in.second() % 10;
				}
					break;
				case 'K':
				{
					*out++ = '0' + (char)(in.second() / 10);
					*out++ = '0' + in.second() % 10;
				}
					break;
				case 'l':
				{
					if(in.millisecond() >= 100)
						*out++ = '0' + (char)(in.millisecond() / 100);
					if(in.millisecond() >= 10)
						*out++ = '0' + (in.millisecond() / 10) % 10;
					*out++ = '0' + in.millisecond() % 10;
				}
					break;
				case 'L':
				{
					*out++ = '0' + (char)(in.millisecond() / 100);
					*out++ = '0' + (in.millisecond() / 10) % 10;
					*out++ = '0' + in.millisecond() % 10;
				}
					break;
				case 'u':
				{
					if (in.is_utc())
					{
						*out++ = 'Z';

						break;
					}

					uint16_t h = in.utc_offset_hours();

					*out++ = in.utc_offset_is_negative() ? '-' : '+';

					*out++ = '0' + (char)(h / 10);
					*out++ = '0' + h % 10;
				}
					break;
				case 'U':
				{
					if (in.is_utc())
						break;

					uint16_t m = in.utc_offset_minutes();

					*out++ = '0' + (char)(m / 10);
					*out++ = '0' + m % 10;
				}
					break;
				case '{':
					c = *++format;//Fallthrough...
				default:
					*out++ = c;
					utf8_cpoints -= _is_utf8_surr(c);
					break;
				}

				utf8_cpoints += (uint32_t)(out - before);
			}

			return out;
		};

		char buf[64];

		char* curr = buf;

		const och::date& value = *reinterpret_cast<const och::date*>(arg_value.p);

		//   [y]yyyy-mm-dd, hh:mm:ss.mmm
		//d: [y]yyyy-mm-dd
		//t: is_utc ? hh:mm:ss.mmm : hh:mm:ss:mmm
		//u: is_utc ? [y]yyyy-mm-ddThh:mm:ss.mmmZ : [y]yyyy-mm-ddThh:mm:ss.mmm+-hh:mm

		uint32_t printed_utf_cpoints = 0;

		if (context.format_specifier == '\0')
			curr = date_formatter(curr, value, "y-M-D, I:J:K.L}", printed_utf_cpoints);
		else if (context.format_specifier == 'd')
			curr = date_formatter(curr, value, "y-M-D}", printed_utf_cpoints);
		else if (context.format_specifier == 't')
			curr = date_formatter(curr, value, "I:J:K.L}", printed_utf_cpoints);
		else if (context.format_specifier == 'u')
			curr = date_formatter(curr, value, "Y-M-DTI:J:K.Lu:U}", printed_utf_cpoints);
		else if (context.format_specifier == 'x')
			curr = date_formatter(curr, value, context.raw_context, printed_utf_cpoints);
		else
			curr = date_formatter(curr, value, "/I/nva/l/i/d /date-for/mat spec/if/ier", printed_utf_cpoints);

		write_with_padding(out, och::stringview(buf, (uint32_t)(curr - buf), printed_utf_cpoints), context.filler, context.width, context.flags & 4);
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
					arg_idx = *curr - '0';

					while (*curr >= '0' && *curr <= '9')
						arg_idx = arg_idx * 10 + *curr - '0';
				}
				else
					arg_idx = arg_counter++;

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
