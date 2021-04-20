#pragma once

#include <cstdint>

#include "och_range.h"
#include "och_fio.h"
#include "och_type_union.h"

namespace och
{
	struct arg_wrapper;

	struct parsed_context
	{
		const char* raw_context;
		range<const arg_wrapper> argv;
		utf8_char filler;
		utf8_char format_specifier;
		uint16_t precision;
		uint16_t width;
		uint8_t flags;

		parsed_context(const char*& context, const range<const arg_wrapper> argv);
	};

	using fmt_fn = void (*) (type_union arg_value, const parsed_context& context);

	struct arg_wrapper
	{
		type_union value;

		fmt_fn formatter;

		arg_wrapper( uint8_t value);
		
		arg_wrapper(uint16_t value);
		
		arg_wrapper(uint32_t value);
		
		arg_wrapper(uint64_t value);
		
		arg_wrapper(  int8_t value);
					 
		arg_wrapper( int16_t value);
					 
		arg_wrapper( int32_t value);
					 
		arg_wrapper( int64_t value);
					 
		arg_wrapper(   float value);
					 
		arg_wrapper(  double value);

		arg_wrapper(const char* value);

		arg_wrapper(const utf8_string& value);

		arg_wrapper(const utf8_view& value);

		arg_wrapper(const date& value);

		arg_wrapper(char32_t value);

		arg_wrapper(char value);

		arg_wrapper(const utf8_char& value);

		arg_wrapper(timespan value);

		arg_wrapper(highres_timespan value);
	};

	uint32_t vprint(iohandle out, const stringview& format, const range<const arg_wrapper>& argv, uint32_t buffer_bytes = 0xFFFF'FFFF);

	//{[argindex] [:[width] [.precision] [rightadj] [~filler] [signmode] [format specifier]]}
	template<typename... Args>
	void print(iohandle out, const stringview& format, Args... args)
	{
		const arg_wrapper argv[]{ arg_wrapper(args)... };

		vprint(out, format, och::range<const arg_wrapper>(argv));
	}

	template<typename... Args>
	void print(iohandle out, const char* format, Args... args)
	{
		print(out, och::stringview(format), args...);
	}

	template<typename... Args>
	void print(iohandle out, const utf8_string& format, Args... args)
	{
		print(out, och::stringview(format), args...);
	}



	template<typename... Args>
	void print(const filehandle& out, const stringview& format, Args... args)
	{
		print(out.handle, format, args...);
	}

	template<typename... Args>
	void print(const filehandle& out, const char* format, Args... args)
	{
		print(out.handle, och::stringview(format), args...);
	}

	template<typename... Args>
	void print(const filehandle& out, const utf8_string& format, Args... args)
	{
		print(out.handle, och::stringview(format), args...);
	}



	template<typename... Args>
	void print(const stringview& format, Args... args)
	{
		print(och::standard_out, format, args...);
	}

	template<typename... Args>
	void print(const char* format, Args... args)
	{
		print(och::standard_out, och::stringview(format), args...);
	}

	template<typename... Args>
	void print(const utf8_string& format, Args... args)
	{
		print(och::standard_out, och::stringview(format), args...);
	}



	void print(iohandle out, const stringview& format);

	void print(iohandle out, const char* format);

	void print(iohandle out, const utf8_string& format);



	void print(const filehandle& out, const stringview& format);

	void print(const filehandle& out, const char* format);

	void print(const filehandle& out, const utf8_string& format);



	void print(const stringview& format);

	void print(const char* format);

	void print(const utf8_string& format);




	template<typename... Args>
	uint32_t sprint(range<char> buf, const stringview& format, Args... args)
	{
		const arg_wrapper argv[]{ arg_wrapper(args)... };

		return vprint(och::iohandle(static_cast<void*>(buf.beg)), format, och::range<const arg_wrapper>(argv), static_cast<uint32_t>(buf.len()));
	}

	template<typename... Args>
	uint32_t sprint(range<char> buf, const char* format, Args... args)
	{
		return sprint(buf, och::stringview(format), args...);
	}

	template<typename... Args>
	uint32_t sprint(range<char> buf, const utf8_string& format, Args... args)
	{
		return sprint(buf, och::stringview(format), args...);
	}

	
	
	uint32_t sprint(range<char> buf, const stringview& format);

	uint32_t sprint(range<char> buf, const char* format);

	uint32_t sprint(range<char> buf, const char* format);
}
