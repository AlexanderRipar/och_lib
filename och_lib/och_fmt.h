#pragma once

#include <cstdint>

#include "och_range.h"
#include "och_fio.h"

namespace och
{
	struct arg
	{
		uint16_t precision = static_cast<uint16_t>(-1);
		uint16_t flags = 0;
		uint8_t width = 0;
		char filler = ' ';
		uint8_t fmt_specifier = '\0';
		uint8_t offset;

		union
		{
			uint64_t i;
			double d;
			float f;
			och::mini_stringview s;
			char32_t c;
			och::time t;
		};

		enum class types : uint8_t
		{
			_uint,
			_int,
			_float,
			_double,
			_string,
			_char,
			_time,
		};

		void set_width(uint8_t w) { width = w; }
		void set_precision(uint16_t p) { precision = p; }
		void set_rightadj() { flags |= 4; }
		void set_signmode(uint16_t s) { flags |= s; }
		void set_offset(uint8_t o) { offset = o; }
		void set_filler(char f) { filler = f; }
		void set_fmt_specifier(char s) { fmt_specifier = s; }

		uint16_t get_width() { return width; }
		uint16_t get_precision() { return precision; }
		bool get_rightadj() { return flags & 4; }
		uint16_t get_signmode() { return  flags & 3; }
		uint8_t get_offset() { return offset; }
		char get_filler() { return filler; }
		char get_fmt_specifier() { return fmt_specifier; }

		arg(                uint8_t i) : offset{ static_cast<uint8_t>(types::_uint  ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(               uint16_t i) : offset{ static_cast<uint8_t>(types::_uint  ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(               uint32_t i) : offset{ static_cast<uint8_t>(types::_uint  ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(               uint64_t i) : offset{ static_cast<uint8_t>(types::_uint  ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(                 int8_t i) : offset{ static_cast<uint8_t>(types::_int   ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(                int16_t i) : offset{ static_cast<uint8_t>(types::_int   ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(                int32_t i) : offset{ static_cast<uint8_t>(types::_int   ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(                int64_t i) : offset{ static_cast<uint8_t>(types::_int   ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(                  float f) : offset{ static_cast<uint8_t>(types::_float ) }, f{                                                                   f  } {}
		arg(                 double d) : offset{ static_cast<uint8_t>(types::_double) }, d{                                                                   d  } {}
		arg(            const char* s) : offset{ static_cast<uint8_t>(types::_string) }, s{                                              s, _const_strlen_u16(s) } {}
		arg(        och::stringview s) : offset{ static_cast<uint8_t>(types::_string) }, s{                                                                   s  } {}
		arg(   och::mini_stringview s) : offset{ static_cast<uint8_t>(types::_string) }, s{                                                                   s  } {}
		arg(       och::range<char> s) : offset{ static_cast<uint8_t>(types::_string) }, s{ reinterpret_cast<const char*>(s.beg), static_cast<uint16_t>(s.len()) } {}
		arg(const och::utf8_string& s) : offset{ static_cast<uint8_t>(types::_string) }, s{                                   s.raw_cbegin(), s.get_codeunits()  } {}
		arg(                   char c) : offset{ static_cast<uint8_t>(types::_string) }, c{                                            static_cast<char32_t>(c)  } {}
		arg(               char32_t c) : offset{ static_cast<uint8_t>(types::_string) }, c{                                                                   c  } {}
		arg(              och::time t) : offset{ static_cast<uint8_t>(types::_time  ) }, t{                                                                   t  } {}
	};

	using fmt_function = void (*) (arg in, och::iohandle out);

	// [argindex] [:[width] [.precision] [rightadj] [~filler] [signmode] [formatmode]]
	void vprint(const char* fmt, arg argv[], uint32_t argc, och::iohandle);

	void print(const och::stringview fmt);

	template<typename... Args>
	void print(const och::stringview fmt, Args... args)
	{
		arg argv[]{ args... };

		vprint(fmt.beg, argv, sizeof...(args), och::out);
	}

	void print(och::iohandle out, const och::stringview fmt);

	template<typename... Args>
	void print(och::iohandle out, const och::stringview fmt, Args... args)
	{
		arg argv[]{ args... };

		vprint(fmt.beg, argv, sizeof...(args), out);
	}

	namespace fmt
	{
		void hex_lowercase();

		void hex_uppercase();

		void set_default_precision(uint32_t precision);

		bool register_formatfunction(fmt_function function, char specifier);
	}
}
