#pragma once

#include <cstdint>

#include "och_memrun.h"
#include "och_fio.h"

namespace och
{
	struct arg
	{
	private:
		uint16_t width = 0;
		uint16_t precision = static_cast<uint16_t>(-1);
		uint16_t flags = 0;
		uint8_t offset;
		char filler = ' ';
	public:
		union
		{
			uint64_t i;
			double d;
			float f;
			och::ministring s;
		};

		enum class types : uint8_t
		{
			_uint,
			_int,
			_float,
			_double,
			_string,
		};

		void set_width(uint16_t w) { width = w; }
		void set_precision(uint16_t p) { precision = p; }
		void set_rightadj() { flags |= 4; }
		void set_signmode(uint16_t s) { flags |= s; }
		void set_offset(uint8_t o) { offset = o; }
		void set_filler(char f) { filler = f; }

		uint16_t get_width() { return width; }
		uint16_t get_precision() { return precision; }
		bool get_rightadj() { return flags & 4; }
		uint16_t get_signmode() { return  flags & 3; }
		uint8_t get_offset() { return offset; }
		char get_filler() { return filler; }

		arg(            uint8_t i) : offset{ static_cast<uint8_t>(types::_uint  ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(           uint16_t i) : offset{ static_cast<uint8_t>(types::_uint  ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(           uint32_t i) : offset{ static_cast<uint8_t>(types::_uint  ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(           uint64_t i) : offset{ static_cast<uint8_t>(types::_uint  ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(             int8_t i) : offset{ static_cast<uint8_t>(types::_int   ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(            int16_t i) : offset{ static_cast<uint8_t>(types::_int   ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(            int32_t i) : offset{ static_cast<uint8_t>(types::_int   ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(            int64_t i) : offset{ static_cast<uint8_t>(types::_int   ) }, i{                                             static_cast<uint64_t>(i) } {}
		arg(              float f) : offset{ static_cast<uint8_t>(types::_float ) }, f{                                                                   f  } {}
		arg(             double d) : offset{ static_cast<uint8_t>(types::_double) }, d{                                                                   d  } {}
		arg(        const char* s) : offset{ static_cast<uint8_t>(types::_string) }, s{                                              s, _const_strlen_u16(s) } {}
		arg(        och::string s) : offset{ static_cast<uint8_t>(types::_string) }, s{                                                                   s  } {}
		arg(    och::ministring s) : offset{ static_cast<uint8_t>(types::_string) }, s{                                                                   s  } {}
		arg(  och::memrun<char> s) : offset{ static_cast<uint8_t>(types::_string) }, s{ reinterpret_cast<const char*>(s.beg), static_cast<uint16_t>(s.len()) } {}
	};

	using fmt_function = void (*) (arg in, och::iohandle out);

	// [argindex] [:[width] [.precision] [rightadj] [~filler] [signmode] [formatmode]]
	void vprint(const char* fmt, arg argv[], uint32_t argc, och::iohandle);

	void print(const och::string fmt);

	template<typename... Args>
	void print(const och::string fmt, Args... args)
	{
		arg argv[]{ args... };

		vprint(fmt.beg, argv, sizeof...(args), och::out);
	}

	template<typename... Args>
	void print(och::iohandle out, const och::string fmt, Args... args)
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
