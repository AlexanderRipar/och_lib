#include "och_fmt.h"

#include <cstdint>

#include "och_range.h"
#include "och_fio.h"

namespace och
{
	arg_wrapper::arg_wrapper(uint64_t value, fmt_fn format_function) : value{ value }, format_function{ format_function } {}

	arg_wrapper wrap_fmt_arg(uint64_t value)
	{
		return arg_wrapper{ value, nullptr };
	}

	//[width] [.precision] [rightadj] [~filler] [signmode] [format specifier]}
	default_context::default_context(const char* context, const och::arg_wrapper* argv, const uint32_t argc)
	{

		if (*context == '#')//Argument Reference
		{
			++context;

			uint32_t width_idx = 0;

			while (*context >= '0' && *context <= '9')
				width_idx = width_idx * 10 + *context++ - '0';

			_ASSERT(width_idx < argc);

			width = static_cast<uint16_t>(argv[width_idx].value);
		}
		else
		{
			width = 0;
			if (*context >= '0' && *context <= '9')
				while (*context >= '0' && *context <= '9')
					width = width * 10 + *context++ - '0';
			else
				++context;
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

				precision = static_cast<uint16_t>(argv[precision_idx].value);
			}
			else
			{
				precision = 0;

				while (*context >= '0' && *context <= '9')
					precision = precision * 10 + *context++ - '0';
			}
		}
		else
			precision = ~0;

		flags = 0;

		if (*context == '>')
		{
			flags |= 4;
			++context;
		}

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

	//{[argindex] [:[width] [.precision] [rightadj] [~filler] [signmode] [formatmode]]}
	void vprint(const char* format, uint32_t cunits, arg_wrapper* argv, uint32_t argc, och::iohandle out)
	{
		uint32_t arg_counter = 0;

		for (uint32_t last_fmt_end = 0, idx = 0; idx != cunits;)
		{
			if (format[idx++] == '{')
			{
				++arg_counter;

				uint32_t arg_idx;

				if (format[idx] >= '0' && format[idx] <= '9')
				{
					arg_idx = format[idx++] - '0';

					while (format[idx] >= '0' && format[idx] <= '9')
						arg_idx = arg_idx * 10 + format[idx++] - '0';
				}
				else
					arg_idx = arg_counter;

				_ASSERT(arg_idx < argc);

				_ASSERT(format[idx] == ':' || format[idx] == '}');

				if (format[idx] == ':')
					++idx;

				argv[arg_idx].format_function(out, argv[arg_idx].value, format + idx, argv, argc);
			}
		}
	}

	void print(const och::utf8_string& format)
	{
		och::write_to_file(och::out, { format.raw_cbegin(), format.raw_cend() });
	}

	void print(const och::utf8_view& format)
	{
		och::write_to_file(och::out, { format.m_ptr, format.m_ptr + format.m_codeunits });
	}
}
