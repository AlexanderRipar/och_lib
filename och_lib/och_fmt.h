#pragma once

#include <cstdint>

#include "och_range.h"
#include "och_fio.h"

namespace och
{
	struct arg_wrapper;

	using fmt_fn = void (*) (och::iohandle out, uint64_t arg_value, const char* context, const och::arg_wrapper* argv, const uint32_t argc);

	struct arg_wrapper
	{
		uint64_t value;

		fmt_fn format_function;

		arg_wrapper(uint64_t value, fmt_fn format_function);
	};

	arg_wrapper wrap_fmt_arg(uint64_t value);

	struct default_context
	{
		uint16_t precision;
		uint16_t width;
		uint8_t flags;
		och::utf8_codepoint filler;
		och::utf8_codepoint format_specifier;

		default_context(const char* context, const och::arg_wrapper* argv, const uint32_t argc);
	};

	void vprint(const char* format, uint32_t cunits, arg_wrapper* argv, uint32_t argc, och::iohandle out);

	//{[argindex] [:[width] [.precision] [rightadj] [~filler] [signmode] [format specifier]]}
	template<typename... Args>
	void print(const och::utf8_string& format, Args... args)
	{
		arg_wrapper argv[]{ wrap_fmt_arg(args)... };

		vprint(format.raw_cbegin(), format.get_codeunits(), argv, sizeof...(args), och::out);
	}

	void print(const och::utf8_string& format);
}
