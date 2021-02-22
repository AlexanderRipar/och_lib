#include "och_fmt.h"

#include <cstdint>

#include "och_range.h"
#include "och_fio.h"

namespace och
{
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////formatting functions////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	
	void write_ascii_with_padding(och::iohandle out, och::stringview text, och::stringview filler, uint32_t to_write, bool is_rightadj)
	{
		uint32_t written = (uint32_t)text.get_codeunits();

		if (is_rightadj)
			while (written++ < to_write)
				och::write_to_file(out, { filler.raw_cbegin(), filler.raw_cend() });

		och::write_to_file(out, { text.raw_cbegin(), text.raw_cend() });

		if (!is_rightadj)
			while (written++ < to_write)
				och::write_to_file(out, { filler.raw_cbegin(), filler.raw_cend() });
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

		write_ascii_with_padding(out, och::stringview(curr, (uint32_t)(buf - curr + 20), 0), och::stringview(context.filler.utf8, context.filler.get_codeunits(), 0), context.width, context.flags & 4);
	}

	void fmt_uint(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		uint64_t value = arg_value.u;

		char buf[20];

		char* curr = buf + 19;

		curr = reverse_itos(curr, value) + 1;

		write_ascii_with_padding(out, och::stringview(curr, (uint32_t)(buf - curr + 20), 0), och::stringview(context.filler.utf8, context.filler.get_codeunits(), 0), context.width, context.flags & 4);
	}

	void fmt_float(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		constexpr och::stringview nan_text("-nan");
		constexpr och::stringview inf_text("-inf");

		uint32_t value = (uint32_t)arg_value.u;

		if (context.format_specifier == 'b');

		const bool is_negative = value & 0x8000'0000;
		const int8_t exponent = (int8_t)(((value & 0x7F80'0000) >> 23) - 127);
		const uint32_t mantissa = value & 0x007F'FFFF;

		och::stringview text("[[fmt_float is not yet implemented]]");
		
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

	void fmt_cstring(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		const char* value = reinterpret_cast<const char*>(arg_value.p);

		uint32_t cunits = 0, cpoints = 0;

		_utf8_len(value, cunits, cpoints, context.precision);

		//write_ascii_with_padding(out, och::stringview(value, cunits), och::stringview(context.filler.utf8, context.filler.get_codeunits()), context.width + cunits - cpoints, context.flags & 4);
	}

	void fmt_utf8_string(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		//const och::utf8_string& value = *reinterpret_cast<const och::utf8_string*>(arg_value.p);
		//
		//if (value.get_codepoints() == value.get_codeunits())
		//{
		//	och::stringview view(value.raw_cbegin(), value.raw_cend());
		//
		//	if (context.precision < view.len())
		//		view.end = view.beg + context.precision;
		//
		//	write_ascii_with_padding(out, view, och::stringview(context.filler.utf8, context.filler.get_codeunits()), context.width, context.flags & 4);
		//}
		//else
		//{
		//	uint32_t written;
		//
		//	och::stringview filler(context.filler.utf8, context.filler.get_codeunits());
		//
		//	och::stringview text;
		//
		//	if (context.precision < value.get_codepoints())
		//	{
		//		const char* beg = value.raw_cbegin();
		//
		//		const char* end = beg;
		//
		//		uint32_t cpoints = 0;
		//
		//		while (cpoints != context.precision)
		//		{
		//			cpoints += !_is_utf8_surr(*end);
		//
		//			++end;
		//		}
		//
		//		text = och::stringview(beg, end);
		//
		//		written = context.precision;
		//	}
		//	else
		//	{
		//		written = value.get_codepoints();
		//
		//		text = och::stringview(value.raw_cbegin(), value.raw_cend());
		//	}
		//
		//	if (context.flags & 4)
		//		while (written++ < context.width)
		//			och::write_to_file(out, filler);
		//
		//	och::write_to_file(out, text);
		//
		//	if (!(context.flags & 4))
		//		while (written++ < context.width)
		//			och::write_to_file(out, filler);
		//}
	}
	
	void fmt_utf8_view(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		och::write_to_file(out, "[[fmt_utf8_view is not yet implemented]]");
	}

	void fmt_codepoint(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		och::utf8_codepoint value = arg_value.c;

		//write_ascii_with_padding(out, och::stringview(value.utf8, value.get_codeunits()), och::stringview(context.filler.utf8, context.filler.get_codeunits()), context.width, context.flags & 4);
	}

	void fmt_date(och::iohandle out, fmt_value arg_value, const parsed_context& context)
	{
		const och::date& value = *reinterpret_cast<const och::date*>(arg_value.p);

		

		och::write_to_file(out, "[[fmt_date is not yet implemented]]");
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////////fmt_value/////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	fmt_value::fmt_value(uint64_t u) : u{ u } {}

	fmt_value::fmt_value(int64_t i) : i{ i } {}

	fmt_value::fmt_value(float f) : f{ f } {}

	fmt_value::fmt_value(double d) : d{ d } {}

	fmt_value::fmt_value(const void* p) : p{ p } {}

	fmt_value::fmt_value(och::utf8_codepoint c) : c{ c } {}



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

	arg_wrapper::arg_wrapper(char32_t value) : value{ och::utf8_codepoint(value) }, formatter{ fmt_codepoint } {};

	arg_wrapper::arg_wrapper(const och::utf8_codepoint& value) : value{ value }, formatter{ fmt_codepoint } {};



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////parsed_context//////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	parsed_context::parsed_context(const char* context, const och::arg_wrapper* argv, const uint32_t argc) : raw_context{ context }, argv{ argv }, argc{ argc }
	{
		//[width] [.precision] [rightadj] [~filler] [signmode] [format specifier]}

		if (*context == '#')//Argument Reference
		{
			++context;

			uint32_t width_idx = 0;

			while (*context >= '0' && *context <= '9')
				width_idx = width_idx * 10 + *context++ - '0';

			_ASSERT(width_idx < argc);

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

				_ASSERT(precision_idx < argc);

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

			filler = utf8_codepoint(context);

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
			format_specifier = utf8_codepoint(context);

			context += format_specifier.get_codeunits();
		}

		_ASSERT(*context == '}');
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////vprint//////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	//{[argindex] [:[width] [.precision] [rightadj] [~filler] [signmode] [formatmode]]}
	void vprint(const char* beg, const char* format_end, arg_wrapper* argv, uint32_t argc, och::iohandle out)
	{
		uint32_t arg_counter = 0;

		const char* last_fmt_end = beg;

		const char* curr = beg;

		while (curr < format_end)
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

				_ASSERT(arg_idx < argc);

				_ASSERT(*curr == ':' || *curr == '}');

				if (*curr == ':')
					++curr;

				parsed_context format_context(curr, argv, argc);

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
