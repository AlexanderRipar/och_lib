#pragma once

#include <cstdint>

#include "och_range.h"
#include "och_fio.h"

namespace och
{
	union fmt_value
	{
		uint64_t u;
		int64_t i;
		float f;
		double d;
		och::mini_stringview s;
		och::utf8_codepoint c;
		const void* p;

		fmt_value(uint64_t u);
		
		fmt_value(int64_t i);
		
		fmt_value(float f);
		
		fmt_value(double d);
		
		fmt_value(och::mini_stringview s);
		
		fmt_value(och::utf8_codepoint c);

		fmt_value(const void* p);
	};

	struct arg_wrapper;

	struct parsed_context
	{
		uint16_t precision;
		uint16_t width;
		uint8_t flags;
		och::utf8_codepoint filler;
		och::utf8_codepoint format_specifier;
		const char* raw_context;
		const och::arg_wrapper* argv;
		const uint32_t argc;

		parsed_context(const char* context, const och::arg_wrapper* argv, const uint32_t argc);
	};

	using fmt_fn = void (*) (och::iohandle out, fmt_value arg_value, const parsed_context& context);

	struct arg_wrapper
	{
		fmt_value value;

		fmt_fn formatter;

		arg_wrapper( uint8_t valuer);
		
		arg_wrapper(uint16_t valuer);
		
		arg_wrapper(uint32_t valuer);
		
		arg_wrapper(uint64_t valuer);
		
		arg_wrapper(  int8_t value);
					 
		arg_wrapper( int16_t value);
					 
		arg_wrapper( int32_t value);
					 
		arg_wrapper( int64_t value);
					 
		arg_wrapper(   float value);
					 
		arg_wrapper(  double value);

		arg_wrapper(const char* value);

		arg_wrapper(const och::utf8_string& value);

		arg_wrapper(const och::utf8_view& value);

		arg_wrapper(const och::date& value);

		arg_wrapper(char32_t value);

		arg_wrapper(const och::utf8_codepoint& value);
	};

	void vprint(const char* format, const char* format_end, arg_wrapper* argv, uint32_t argc, och::iohandle out);

	//{[argindex] [:[width] [.precision] [rightadj] [~filler] [signmode] [format specifier]]}
	template<typename... Args>
	void print(och::iohandle out, const och::stringview& format, Args... args)
	{
		arg_wrapper argv[]{ args... };

		vprint(format.beg, format.end, argv, sizeof...(args), out);
	}

	template<typename... Args>
	void print(och::iohandle out, const char* format, Args... args)
	{
		print(out, och::stringview(format, strlen(format)), args...);
	}

	template<typename... Args>
	void print(och::iohandle out, const och::utf8_string& format, Args... args)
	{
		print(out, och::stringview(format.raw_cbegin(), format.raw_cend()), args...);
	}

	template<typename... Args>
	void print(och::iohandle out, const och::utf8_view& format, Args... args)
	{
		print(out, och::stringview(format.m_ptr, format.m_codeunits), args...);
	}
	


	template<typename... Args>
	void print(const och::stringview& format, Args... args)
	{
		print(och::standard_out, format, args...);
	}

	template<typename... Args>
	void print(const char* format, Args... args)
	{
		print(och::standard_out, och::stringview(format, strlen(format)), args...);
	}

	template<typename... Args>
	void print(const och::utf8_string& format, Args... args)
	{
		print(och::standard_out, och::stringview(format.raw_cbegin(), format.raw_cend()), args...);
	}

	template<typename... Args>
	void print(const och::utf8_view& format, Args... args)
	{
		print(och::standard_out, och::stringview(format.m_ptr, format.m_codeunits), args...);
	}



	void print(och::iohandle out, const och::stringview& format);

	void print(och::iohandle out, const char* format);

	void print(och::iohandle out, const och::utf8_string& format);

	void print(och::iohandle out, const och::utf8_view& format);



	void print(const och::filehandle& out, const och::stringview& format);

	void print(const och::filehandle& out, const char* format);

	void print(const och::filehandle& out, const och::utf8_string& format);

	void print(const och::filehandle& out, const och::utf8_view& format);



	void print(const och::stringview& format);

	void print(const char* format);

	void print(const och::utf8_string& format);

	void print(const och::utf8_view& format);
}
