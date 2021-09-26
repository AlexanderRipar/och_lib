#define OCH_FMT_PRESENT

#ifndef OCH_FMT_INCLUDE_GUARD

#include <cstdint>

#include "och_range.h"
#include "och_fio.h"
#include "och_type_union.h"

namespace och
{
	struct arg_wrapper;

	struct output_buffer;

	struct parsed_context
	{
		const char* raw_context = nullptr;
		range<const arg_wrapper> argv;
		utf8_char filler = ' ';
		utf8_char format_specifier = '\0';
		uint16_t precision = 0xFFFF;
		uint16_t width;
		uint8_t flags = 0;
		output_buffer& output;

		parsed_context(const char*& context, const range<const arg_wrapper> argv, output_buffer& output);
	};

	using fmt_fn = void (*) (type_union arg_value, const parsed_context& context) noexcept;

	struct arg_wrapper
	{
		type_union value;

		fmt_fn formatter;
	};

	arg_wrapper create_fmt_arg_wrapper(uint8_t value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(uint16_t value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(uint32_t value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(uint64_t value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(int8_t value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(int16_t value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(int32_t value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(int64_t value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(float value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(double value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(const char* value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(const utf8_string& value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(const utf8_view& value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(char32_t value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(char value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(const utf8_char& value) noexcept;
}

#endif // !OCH_FMT_INCLUDE_GUARD

namespace och
{
#ifdef OCH_TIME_PRESENT
	arg_wrapper create_fmt_arg_wrapper(const date& value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(timespan value) noexcept;

	arg_wrapper create_fmt_arg_wrapper(highres_timespan value) noexcept;
#endif // OCH_TIME_PRESENT

#ifdef OCH_MATMATH_PRESENT
	arg_wrapper create_fmt_arg_wrapper(const och::mat4& value);

	arg_wrapper create_fmt_arg_wrapper(const och::mat3& value);

	arg_wrapper create_fmt_arg_wrapper(const och::mat2& value);

	arg_wrapper create_fmt_arg_wrapper(const och::vec4& value);

	arg_wrapper create_fmt_arg_wrapper(const och::vec3& value);

	arg_wrapper create_fmt_arg_wrapper(const och::vec2& value);
#endif // OCH_MATMATH_PRESENT
}


#ifndef OCH_FMT_INCLUDE_GUARD
#define OCH_FMT_INCLUDE_GUARD

namespace och
{
	uint32_t vprint(iohandle out, const stringview& format, const range<const arg_wrapper>& argv, uint32_t buffer_bytes = 0xFFFF'FFFF);

	//{[argindex] [:[width] [.precision] [rightadj] [~filler] [signmode] [format specifier]]}
	template<typename... Args>
	void print(iohandle out, const stringview& format, Args... args)
	{
		const arg_wrapper argv[]{ create_fmt_arg_wrapper(args)... };

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
		const arg_wrapper argv[]{ create_fmt_arg_wrapper(args)... };

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



	template<typename... Args>
	uint32_t sprint(och::utf8_string& buf, const stringview& format, Args... args)
	{
		const arg_wrapper argv[]{ create_fmt_arg_wrapper(args)... };

		return vprint(och::iohandle(static_cast<void*>(&buf)), format, och::range<const arg_wrapper>(argv), 0xFFFF'FFFE);
	}

	template<typename... Args>
	uint32_t sprint(och::utf8_string& buf, const char* format, Args... args)
	{
		return sprint(buf, och::stringview(format), args...);
	}

	template<typename... Args>
	uint32_t sprint(och::utf8_string& buf, const utf8_string& format, Args... args)
	{
		return sprint(buf, och::stringview(format), args...);
	}



	uint32_t sprint(och::utf8_string& buf, const stringview& format);

	uint32_t sprint(och::utf8_string& buf, const char* format);

	uint32_t sprint(och::utf8_string& buf, const char* format);
}

#endif // !OCH_FMT_INCLUDE_GUARD
