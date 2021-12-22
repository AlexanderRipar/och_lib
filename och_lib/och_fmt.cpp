#include "och_fmt.h"

#include <cstdint>
#include <cstring>
#include <cassert>
#include <intrin.h>

#include "och_fio.h"
#include "och_utf8.h"
#include "och_matmath.h"
#include "och_virtual_keys.h"

namespace och
{
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////formatting internals and data///////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	bool is_rightadj(const parsed_context& context)
	{
		return context.flags & 4;
	}

	struct output_buffer
	{
		static constexpr uint64_t file_buffer_capacity = 1024;

		och::range<char> buffer;

		union
		{
			och::iohandle backing_file;
			och::utf8_string* backing_string;
		};

		uint64_t overrun_count = 0;

		output_buffer(och::range<char> buffer, void* backing_file) : buffer{ buffer }, backing_string{ reinterpret_cast<och::utf8_string*>(backing_file) } {}
		
		bool has_file() const noexcept
		{
			return buffer.beg;
		}

		char* reserve(uint32_t codeunits)
		{
			if (buffer.beg)
			{
				if (buffer.len() >= codeunits)
				{
					char* retval = buffer.beg;

					buffer.beg += codeunits;

					return retval;
				}
				else if (backing_file)
				{
					flush_to_file();

					if (codeunits <= file_buffer_capacity)
						return buffer.beg;
				}
				else
				{
					overrun_count += codeunits - buffer.len();

					buffer.beg = buffer.end;
				}
			}
			else if (backing_string->reserve(backing_string->get_codeunits() + codeunits))
			{
					char* ret = backing_string->raw_end();

					backing_string->fmt_prepare_for_raw_write(codeunits);

					return ret;
			}

			return nullptr;
		}

		void flush_to_file()
		{
			uint32_t bytes_written;

			ignore_status(och::write_to_file(bytes_written, backing_file, och::range<const uint8_t>(reinterpret_cast<const uint8_t*>(buffer.end - file_buffer_capacity), reinterpret_cast<const uint8_t*>(buffer.beg))));
			
			buffer.beg = buffer.end - file_buffer_capacity;
		}

		void put(utf8_char c)
		{
			if (buffer.beg)
			{
				if (buffer.len() < c.get_codeunits())
				{
					if (backing_file)
						flush_to_file();
					else
					{
						overrun_count += c.get_codeunits() - buffer.len();

						buffer.beg = buffer.end;

						return;
					}
				}

				for (uint32_t i = 0; i != c.get_codeunits(); ++i)
					*buffer.beg++ = c.cbegin()[i];
			}
			else
			{
				backing_string->operator+=(c);
			}
		}

		void put(const och::stringview& v)
		{
			if (buffer.beg)
			{
				if (buffer.len() < v.get_codeunits())
				{
					if (backing_file)
					{
						flush_to_file();

						uint32_t bytes_written;
						ignore_status(och::write_to_file(bytes_written, backing_file, och::range<const uint8_t>(reinterpret_cast<const uint8_t*>(v.raw_cbegin()), reinterpret_cast<const uint8_t*>(v.raw_cend()))));
					}
					else
					{
						overrun_count += v.get_codeunits() - buffer.len();

						buffer.beg = buffer.end;
					}
				}
				else
				{
					for (uint32_t i = 0; i != v.get_codeunits(); ++i)
						*buffer.beg++ = v.raw_cbegin()[i];
				}
			}
			else
			{
				backing_string->operator+=(v);
			}
		}

		void pad(uint32_t text_codepoints, const och::parsed_context& context)
		{
			int32_t filler_cpoints = context.width - text_codepoints;

			if (filler_cpoints < 0)
				return;

			int32_t filler_cunits = filler_cpoints * context.filler.get_codeunits();

			utf8_char c = context.filler;

			if (buffer.beg)
			{
				if (filler_cunits > buffer.len())
					if (backing_file)
					{
						uint32_t bytes_written;
						ignore_status(och::write_to_file(bytes_written, backing_file, och::range<const uint8_t>(reinterpret_cast<const uint8_t*>(buffer.end - file_buffer_capacity), reinterpret_cast<const uint8_t*>(buffer.beg))));

						buffer.beg = buffer.end - file_buffer_capacity;

						buffer.beg = buffer.end - file_buffer_capacity;

						while (filler_cunits >= file_buffer_capacity)
						{
							uint32_t i = 0;

							for (i = 0; i <= file_buffer_capacity - c.get_codeunits(); i += c.get_codeunits())
								for (uint32_t j = 0; j != c.get_codeunits(); ++j)
									buffer[static_cast<ptrdiff_t>(i) + j] = c.cbegin()[i];

							filler_cunits -= i;

							ignore_status(och::write_to_file(bytes_written, backing_file, och::range<const uint8_t>(reinterpret_cast<const uint8_t*>(buffer.beg), reinterpret_cast<const uint8_t*>(buffer.beg + i))));

							buffer.beg = buffer.end - file_buffer_capacity;
						}
					}
					else
					{
						overrun_count += filler_cunits - buffer.len();

						buffer.beg = buffer.end;

						return;
					}

				for (int i = 0; i != filler_cunits; i += c.get_codeunits())
					for (uint32_t j = 0; j != c.get_codeunits(); ++j)
						*buffer.beg++ = c.cbegin()[j];
			}
			else
			{
				backing_string->reserve(backing_string->get_codeunits() + filler_cunits);

				for (int i = 0; i != filler_cpoints; ++i)
					backing_string->operator+=(c);
			}
		}

		void put_padded(utf8_char c, const parsed_context& context)
		{
			if (is_rightadj(context))
				pad(1, context);

			put(c);

			if (!is_rightadj(context))
				pad(1, context);
		}

		void put_padded(const och::stringview& v, const parsed_context& context)
		{
			if (is_rightadj(context))
				pad(v.get_codepoints(), context);

			put(v);

			if (!is_rightadj(context))
				pad(v.get_codepoints(), context);
		}

		void finalize()
		{
			if (buffer.beg && backing_file && buffer.len() != file_buffer_capacity)
				flush_to_file();
			else if (buffer.beg)
				put(utf8_char('\0'));
			else
				backing_string += '\0';
		}
	};

	constexpr const char* hex_lower_upper = "0123456789abcdef0123456789ABCDEF";

	constexpr och::stringview invalid_specifier_msg("[[Invalid format-specifier]]");

	constexpr const char* weekdays = "Sunday\0\0\0\0" "Monday\0\0\0\0" "Tuesday\0\0\0" "Wednesday\0" "Thursday\0\0" "Friday\0\0\0\0" "Saturday\0";

	constexpr const char* months = "January\0\0\0"    "February\0\0"   "March\0\0\0\0\0" "April\0\0\0\0\0" "May\0\0\0\0\0\0\0" "June\0\0\0\0\0\0"
		                           "July\0\0\0\0\0\0" "August\0\0\0\0" "September\0"     "October\0\0\0"   "November\0\0"      "December\0";



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*////////////////////////////////////////////////formatting helpers/////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	uint32_t log2(uint64_t n) noexcept
	{
		uint32_t ret = 0;

		do
		{
			n >>= 1;
			++ret;
		}
		while (n);

		return ret;
	}

	uint32_t log10(uint64_t n) noexcept
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

	uint32_t log16(uint64_t n) noexcept
	{
		uint32_t ret = 1;

		do
		{
			n >>= 4;
			++ret;
		} 
		while (n > 15);

		return ret;
	}

	void h_fmt_two_digit(output_buffer& out, uint64_t n)
	{
		out.put(utf8_char((char)('0' + n / 10)));

		out.put(utf8_char((char)('0' + n % 10)));
	}

	void h_fmt_three_digit(output_buffer& out, uint64_t n)
	{
		out.put((char)('0' + n / 100));
		out.put((char)('0' + (n / 10) % 10));
		out.put((char)('0' + n % 10));
	}

	void h_fmt_decimal(output_buffer& out, uint64_t n, int32_t digits, char sign = '\0')
	{
		char* curr = out.reserve(digits + (sign != 0)) + digits - (sign == 0);

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

	void h_fmt_hex(output_buffer& out, uint64_t value, int32_t digits, const char* hex_charset)
	{
		char* curr = out.reserve(digits) + digits - 1;

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

	void h_fmt_binary(output_buffer& out, uint64_t value, int32_t digits)
	{
		char* curr = out.reserve(digits) + digits - 1;

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

	void h_fmt_integer_base(output_buffer& out, uint64_t n, const parsed_context& context, uint32_t bit_width, char sign = '\0')
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
					out.pad(digits, context);

				h_fmt_decimal(out, abs_n, digits - (sign != 0), sign);
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
					out.pad(digits, context);

				h_fmt_hex(out, n, digits, hex_fmt);
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
					out.pad(digits, context);

				h_fmt_binary(out, n, digits);
			}
			break;

		default:
			{
				out.put_padded(invalid_specifier_msg, context);
			}
			return; //Necessary to avoid final right padding
		}

		if (!is_rightadj(context))
			out.pad(digits, context);
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
			assert(*context == '}');

			++context;

			assert(val < argv.len());

			val = argv[val].value.u16;
		}

		return val;
	}

	template<size_t SZ>
	void h_fmt_mat(type_union arg_value, const parsed_context& context) noexcept
	{
		constexpr uint32_t ELEMS = SZ * SZ;

		uint32_t prec = context.precision;

		if (prec == 0x7FFF || prec == 0xFFFF)
			prec = 4;
		else if (prec > 8)
			prec = 8;

		uint8_t cps[ELEMS];

		char flt_strs[ELEMS][32];

		for (uint32_t i = 0; i != ELEMS; ++i)
			cps[i] = static_cast<uint8_t>(och::sprint(och::range(flt_strs[i]), "{0:.{1}_}", static_cast<const float*>(arg_value.ptr)[i], prec));

		if (context.flags & 1)
			for (uint32_t i = 0; i != ELEMS; ++i)
				if (flt_strs[i][0] == ' ')
					flt_strs[i][0] = '+';

		uint8_t max_cps = static_cast<uint8_t>(context.width);

		for (uint8_t i = 0; i != ELEMS; ++i)
			if (cps[i] > max_cps)
				max_cps = cps[i];

		for (uint32_t y = 0; y != SZ; ++y)
		{
			context.output.put('|');

			for (uint32_t x = 0; x != SZ; ++x)
			{
				const uint32_t i = x * SZ + y;

				for (uint8_t c = cps[i]; c != max_cps; ++c)
					context.output.put(' ');

				context.output.put(och::stringview(flt_strs[i], cps[i], cps[i]));

				if (x != SZ - 1)
				{
					context.output.put(',');
					context.output.put(' ');
				}
			}

			context.output.put('|');

			if (y != SZ - 1)
				context.output.put('\n');
		}
	}

	template<size_t SZ>
	void h_fmt_vec(type_union arg_value, const parsed_context& context) noexcept
	{
		uint32_t prec = context.precision;

		if (prec == 0x7FFF || prec == 0xFFFF)
			prec = 4;
		else if (prec > 8)
			prec = 8;

		uint8_t cps[SZ];

		char flt_strs[SZ][32];

		for (uint32_t i = 0; i != SZ; ++i)
			cps[i] = static_cast<uint8_t>(och::sprint(och::range(flt_strs[i]), "{0:.{1}_}", static_cast<const float*>(arg_value.ptr)[i], prec));

		if (context.flags & 1)
			for (uint32_t i = 0; i != SZ; ++i)
				if (flt_strs[i][0] == ' ')
					flt_strs[i][0] = '+';

		uint8_t max_cps = static_cast<uint8_t>(context.width);

		for (uint8_t i = 0; i != SZ; ++i)
			if (cps[i] > max_cps)
				max_cps = cps[i];

		context.output.put('(');

		for (uint32_t i = 0; i != SZ; ++i)
		{
			for (uint8_t c = cps[i]; c != max_cps; ++c)
				context.output.put(' ');

			context.output.put(och::stringview(flt_strs[i], cps[i], cps[i]));

			if (i != SZ - 1)
			{
				context.output.put(',');
				context.output.put(' ');
			}
		}

		context.output.put(')');
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////formatting functions////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	void fmt_uint64(type_union arg_value, const parsed_context& context) noexcept
	{
		uint64_t value = arg_value.u64;

		h_fmt_integer_base(context.output, value, context, 64);
	}

	void fmt_int64(type_union arg_value, const parsed_context& context) noexcept
	{
		int64_t value = arg_value.i64;

		h_fmt_integer_base(context.output, value, context, 64, h_get_sign(value, context));
	}

	void fmt_uint32(type_union arg_value, const parsed_context& context) noexcept
	{
		uint32_t value = arg_value.u32;

		h_fmt_integer_base(context.output, value, context, 32);
	}

	void fmt_int32(type_union arg_value, const parsed_context& context) noexcept
	{
		int32_t value = arg_value.i32;

		h_fmt_integer_base(context.output, value, context, 32, h_get_sign(value, context));
	}

	void fmt_uint16(type_union arg_value, const parsed_context& context) noexcept
	{
		uint16_t value = arg_value.u16;

		h_fmt_integer_base(context.output, value, context, 16);
	}

	void fmt_int16(type_union arg_value, const parsed_context& context) noexcept
	{
		int16_t value = arg_value.i16;

		h_fmt_integer_base(context.output, value, context, 16, h_get_sign(value, context));
	}

	void fmt_uint8(type_union arg_value, const parsed_context& context) noexcept
	{
		uint8_t value = arg_value.u8;

		h_fmt_integer_base(context.output, value, context, 8);
	}

	void fmt_int8(type_union arg_value, const parsed_context& context) noexcept
	{
		int8_t value = arg_value.i8;

		h_fmt_integer_base(context.output, value, context, 8, h_get_sign(value, context));
	}

	void fmt_utf8_view(type_union arg_value, const parsed_context& context) noexcept
	{
		och::utf8_view value = *(const och::utf8_view*)arg_value.ptr;

		if (value.get_codepoints() > context.precision)
			value = value.subview(0, context.precision);

		context.output.put_padded(value, context);
	}

	void fmt_utf8_string(type_union arg_value, const parsed_context& context) noexcept
	{
		och::utf8_view value(*(const och::utf8_string*)arg_value.ptr);

		fmt_utf8_view((const void*)&value, context);
	}

	void fmt_cstring(type_union arg_value, const parsed_context& context) noexcept
	{
		och::utf8_view value(reinterpret_cast<const char*>(arg_value.ptr));

		fmt_utf8_view((const void*)&value, context);
	}

	void fmt_codepoint(type_union arg_value, const parsed_context& context) noexcept
	{
		och::utf8_char value = arg_value.utf_c;

		context.output.put_padded(value, context);
	}

	void fmt_float(type_union arg_value, const parsed_context& context) noexcept
	{
		constexpr uint64_t radix_pos = 60;

		struct chonker
		{
			uint32_t bits[10]{};

			chonker(int32_t raw_exp, int32_t mnt)
			{
				if (!raw_exp)
					raw_exp = 1;
				else
					mnt |= 1 << 23;

				int32_t shift = raw_exp + 10;

				int32_t block = shift >> 5;

				int32_t rest = shift & 31;

				bits[block    ] = mnt << rest;;
				bits[block + 1] = static_cast<uint32_t>(static_cast<uint64_t>(mnt) >> (32 - rest));
			}

			__forceinline bool div10_part(uint32_t& n, uint32_t& rem)
			{
				uint64_t nl = (static_cast<uint64_t>(rem) << 32) | n;

				rem = nl % 10;

				n = static_cast<uint32_t>(nl / 10);

				return n;
			}

			bool moddiv10(int32_t& out_rem)
			{
				uint32_t rem = 0;

				bool is_nonzero = false;

				for (int i = 9; i != 4; --i)
					is_nonzero |= div10_part(bits[i], rem);

				out_rem = rem;

				return is_nonzero;
			}

			__forceinline bool mul10_part(uint32_t& n, uint32_t& rem)
			{
				uint64_t nl = static_cast<uint64_t>(n);

				nl = nl * 10 + rem;

				n = static_cast<uint32_t>(nl);

				rem = static_cast<uint32_t>(nl >> 32);

				return n;
			}

			bool modmul10(int32_t& out_rem)
			{
				uint32_t rem = 0;

				bool is_nonzero = false;

				for (int i = 0; i != 5; ++i)
					is_nonzero |= mul10_part(bits[i], rem);

				out_rem = rem;

				return is_nonzero;
			}
		};

		const int32_t value = arg_value.i32;

		const int32_t sgn = value & (1 << 31);
		const int32_t exp = ((value >> 23) & 0xFF);
		const int32_t mnt = value & 0x7FFFFF;

		if (exp == 0xFF)
		{
			if (mnt)
			{
				context.output.put_padded(och::stringview("nan"), context);

				return;
			}
			else
			{
				char buf[]{ 'X', 'i', 'n', 'f', '0' };

				char* beg = buf;

				if (sgn)
					*beg = '-';
				else if (context.flags & 1)
					*beg = '+';
				else if (context.flags & 2)
					*beg = ' ';
				else
					++beg;

				uint32_t len = static_cast<uint32_t>(buf - beg + 4);

				context.output.put_padded(och::stringview(beg, len, len), context);

				return;
			}
		}

		int32_t norm_exp = exp - 127;

		char buf[256];

		char* integral_part = buf + radix_pos;

		char* fractional_part = buf + radix_pos;

		buf[radix_pos] = '.';

		int32_t fract_digits = 0, precision = context.precision == 0xFFFF ? 6 : static_cast<int32_t>(context.precision);

#ifdef _M_AMD64
		constexpr int32_t mul_offset = 0;
		
		constexpr uint64_t mul_mask = 0xFFFF'FFFF'FFFF'FFFF;
#else
		constexpr int32_t mul_offset = 4;

		constexpr uint64_t mul_mask = (1ull << (64 - mul_offset)) - 1;
#endif // _M_AMD64

		if (norm_exp < 64 && norm_exp > -41 + mul_offset) //Fast path
		{

			uint64_t norm_mnt = (1 << 23) | mnt;

			uint64_t whole = norm_exp < 0 ? 0ull : norm_exp <= 23 ? norm_mnt >> (23 - norm_exp) : norm_mnt << (norm_exp - 23);

			uint64_t fract = norm_exp >= 23 ? 0ull : (norm_mnt << (41 + norm_exp - mul_offset)) & mul_mask;

			while (whole >= 10)
			{
				*--integral_part = '0' + (whole % 10);

				whole /= 10;
			}

			*--integral_part = '0' + static_cast<char>(whole);

			do
#ifdef _M_AMD64
			{
				uint64_t ovflo;

				fract = _umul128(10, fract, &ovflo);

				*++fractional_part = '0' + static_cast<char>(ovflo);

				++fract_digits;
			}
#else		
			{
				fract *= 10;

				*++fractional_part = '0' + static_cast<char>(fract >> (64 - mul_offset));

				fract &= (1ull << (64 - mul_offset)) - 1;

				++fract_digits;
			}
#endif // _M_AMD64
			while (fract && fract_digits < precision);
		}
		else //Slow path
		{
			chonker num(exp, mnt);

			int32_t rem;

			bool nonzero = true;

			while (nonzero)
			{
				nonzero = num.moddiv10(rem);

				*--integral_part = '0' + static_cast<char>(rem);
			}

			nonzero = true;

			while (nonzero && fract_digits <= precision)
			{
				nonzero = num.modmul10(rem);

				*++fractional_part = '0' + static_cast<char>(rem);

				++fract_digits;
			}
		}

		if (context.format_specifier == och::utf8_char('\0')) //Default formatting
		{
			if (precision != 0x7FFF) //Rounding / padding
			{
				if (fract_digits > precision) //Round...
				{
					while (fractional_part != buf + radix_pos + 1)
					{
						char falloff = *fractional_part;

						*--fractional_part += falloff >= '5';

						--fract_digits;

						if (*fractional_part != '9' + 1)
							break;
					}
				}

				if (precision > 128) //Prevent a buffer overflow
					precision = 128;

				while (fract_digits++ < precision) //Pad...
					*++fractional_part = '0';
			}

			if (sgn)
				*--integral_part = '-';
			else if (context.flags & 1)
				*--integral_part = '+';
			else if (context.flags & 2)
				*--integral_part = ' ';

			if (!precision)
				fractional_part = buf + radix_pos - 1;
		}
		else
		{
			context.output.put_padded(invalid_specifier_msg, context);

			return;
		}

		uint32_t len = static_cast<uint32_t>(fractional_part - integral_part + 1);

		context.output.put_padded(och::stringview(integral_part, len, len), context);
	}

	//TODO implement
	void fmt_double(type_union arg_value, const parsed_context& context) noexcept
	{
		arg_value;

		context.output.put_padded(och::stringview("[[fmt_double is not yet implemented]]"), context);
	}

	void fmt_date(type_union arg_value, const parsed_context& context) noexcept
	{
		//          [y]yyyy-mm-dd, hh:mm:ss.mmm
		// d   ->   [y]yyyy-mm-dd
		// t   ->   hh:mm:ss.mmm : hh:mm:ss:mmm
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

		output_buffer& out = context.output;

		uint32_t utf8_cpoints = 0;

		const char* format;

		if (context.format_specifier == och::utf8_char('\0'))
			format = "y-M-D, I:J:K.L}";
		else if (context.format_specifier == och::utf8_char('d'))
			format = "y-M-D}";
		else if (context.format_specifier == och::utf8_char('t'))
			format = "I:J:K.L}";
		else if (context.format_specifier == och::utf8_char('u'))
			format = "Y-M-DTI:J:K.Lus:U}";
		else if (context.format_specifier == och::utf8_char('x'))
			format = context.raw_context;
		else
		{
			out.put_padded(invalid_specifier_msg, context);
			return;
		}

#define OCH_FMT_2DIGIT(x) if(c & 0x20) { if(x >= 10) out.put((char)('0' + x / 10)); out.put((char)('0' + x % 10)); } else { out.put((char)('0' + x / 10)); out.put((char)('0' + x % 10)); }

		uint16_t year = value.year();
		uint16_t month = value.month();
		uint16_t weekday = value.weekday();
		uint16_t monthday = value.monthday();
		uint16_t hour = value.hour();
		uint16_t minute = value.minute();
		uint16_t second = value.second();
		uint16_t millisecond = value.millisecond();

		uint32_t cpoints = 0;

		//Count required codepoints
		{
			const char* fmt = format;

			for (char c = *fmt; c != '}'; c = *++fmt)
				switch (c)
				{
				case 'y':
					cpoints += log10(year); 	break;
				case 'Y':
					cpoints += year >= 10000 ? 5 : 4; break;
				case 'm':
					cpoints += month >= 10 ? 2 : 1; break;
				case 'N':
					cpoints += static_cast<uint32_t>(strlen(months + static_cast<ptrdiff_t>(month - 1) * 10)); break;
				case 'd':
					cpoints += 1 + (monthday >= 10); break;
				case 'W':
					cpoints += static_cast<uint32_t>(strlen(weekdays + static_cast<ptrdiff_t>(weekday) * 10)); break;
				case 'i':
					cpoints += 1 + (hour >= 10); break;
				case 'j':
					cpoints += 1 + (minute >= 10); break;
				case 'k':
					cpoints += 1 + (second >= 10); break;
				case 'l':
					cpoints += millisecond >= 100 ? 3 : millisecond >= 10 ? 2 : 1; break;
				case 'D':
				case 'I':
				case 'J':
				case 'K':
				case 'M':
					cpoints += 2; break;
				case 'L':
				case 'w':
				case 'n':
					cpoints += 3; break;
				case 'u':
					cpoints += value.is_utc() ? 1 : 3; break;
				case 'U':
					cpoints += value.is_utc() ? 0 : 2; break;
				case 's':
				case 'S':
					if ((c != 's') ^ (value.is_utc())) //Next char is inactive
					{
						if (*++fmt == 'x')
							++fmt;

						while (is_utf8_surr(fmt[1]))
							++fmt;
					}
					break;
				case 'x':
					c = *++fmt; //Fallthrough...
				default:
					cpoints += !is_utf8_surr(c); break;
				}
		}

		//Actually start formatting

		if (is_rightadj(context))
			context.output.pad(cpoints, context);

		for (char c = *format; c != '}'; c = *++format)
			switch (c)
			{
			case 'y':
				h_fmt_decimal(out, year, log10(year));
				break;

			case 'Y':
				h_fmt_decimal(out, year, log10(year));
				for (int i = log10(year); i != 4; ++i)
					out.put('0');
				break;

			case 'm':
			case 'M':
				OCH_FMT_2DIGIT(month);
				break;

			case 'n':
				for (int i = 0; i != 3; ++i)
					out.put(months[(month - 1) * 10 + i]);
				break;

			case 'N':
				out.put(och::stringview(months + (ptrdiff_t)(month - 1) * 10));
				break;

			case 'd':
			case 'D':
				OCH_FMT_2DIGIT(monthday);
				break;

			case 'w':
				for(int i = 0; i != 3; ++i)
					out.put(weekdays[weekday * 10 + i]);
				break;

			case 'W':
				out.put(och::stringview(weekdays + (ptrdiff_t)weekday * 10));
				break;

			case 'i':
			case 'I':
				OCH_FMT_2DIGIT(hour);
				break;

			case 'j':
			case 'J':
				OCH_FMT_2DIGIT(minute);
				break;

			case 'k':
			case 'K':
				OCH_FMT_2DIGIT(second);
				break;

			case 'l':
				h_fmt_decimal(out, millisecond, log10(millisecond));
				break;

			case 'L':
				h_fmt_three_digit(out, millisecond);
				break;

			case 'u':
				if (!value.is_utc())
				{
					out.put(value.utc_offset_is_negative() ? '-' : '+');
					h_fmt_two_digit(out, value.utc_offset_hours());
				}
				else
					out.put('Z');
				break;

			case 'U':
				if (!value.is_utc())
					h_fmt_two_digit(out, value.utc_offset_minutes());
				break;

			case 's':
			case 'S':
				if ((c != 's') ^ (value.is_utc())) //Next char is inactive
				{
					if (*++format == 'x')
						++format;

					while (is_utf8_surr(format[1]))
						++format;
				}
				break;

			case 'x':
				c = *++format;//Fallthrough...
			default:
				out.put(c);
				utf8_cpoints -= is_utf8_surr(c);
				break;
			}

		if (!is_rightadj(context))
			out.pad(cpoints, context);

#undef OCH_FMT_2DIGIT
	}

	//TODO improve (Not happy)
	void fmt_timespan(type_union arg_value, const parsed_context& context) noexcept
	{
		//     ->   appropriate format (with format specifier)
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
		
		output_buffer& out = context.output;

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
					out.pad(5 + (sign != 0), context);

				if (sign)
					out.put(sign);

				h_fmt_three_digit(out, value.microseconds());
				out.put('u');
				out.put('s');

				if (!is_rightadj(context))
					out.pad(5 + (sign != 0), context);
			}
			else if (value.val < 10000000ll && value.val > -10000000ll)
			{
				if (is_rightadj(context))
					out.pad(9 + (sign != 0), context);

				if (sign)
					out.put(sign);

				h_fmt_three_digit(out, value.milliseconds());

				out.put('.');

				h_fmt_three_digit(out, value.microseconds() % 1000);

				out.put('m');
				out.put('s');

				if (!is_rightadj(context))
					out.pad(9 + (sign != 0), context);
			}
			else if (value.val < 600000000ll && value.val > -600000000ll)
			{
				if (is_rightadj(context))
					out.pad(7 + (sign != 0), context);

				h_fmt_two_digit(out, value.seconds());

				out.put('.');

				h_fmt_three_digit(out, value.milliseconds() % 1000);

				out.put('s');

				if (!is_rightadj(context))
					out.pad(7 + (sign != 0), context);
			}
			else if (value.val < 60 * 600000000ll && value.val > -60 * 600000000ll)
			{
				if (is_rightadj(context))
					out.pad(12 + (sign != 0), context);

				h_fmt_two_digit(out, value.minutes());

				out.put(':');

				h_fmt_two_digit(out, value.seconds() % 60);

				out.put('.');

				h_fmt_three_digit(out, value.milliseconds() % 1000);

				out.put('m');
				out.put('i');
				out.put('n');

				if (!is_rightadj(context))
					out.pad(12 + (sign != 0), context);
			}
			else
			{
				int64_t days = value.days();

				uint32_t log10_d = log10(days);

				uint32_t chars = 6 + (days ? log10_d + 4 : 0) + (sign != 0);

				if (is_rightadj(context))
					out.pad(chars, context);

				if (sign)
					out.put(sign);

				if (days)
				{
					h_fmt_decimal(out, value.days(), log10_d);

					out.put('d');
					out.put(' ');
					out.put('+');
					out.put(' ');
				}

				h_fmt_two_digit(out, value.hours() % 24);

				out.put(':');

				h_fmt_two_digit(out, value.minutes() % 60);

				if (!is_rightadj(context))
					out.pad(chars, context);
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
				out.put_padded(invalid_specifier_msg, context);

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
				out.put_padded(invalid_specifier_msg, context);

				return;
			}
			break;
		case 'm':
			if (!context.raw_context)
			{
				out.put_padded(invalid_specifier_msg, context);

				return;
			}
			else if (context.raw_context[0] == 's')
				time_value = value.milliseconds();
			else if (context.raw_context[0] == 'i' && context.raw_context[1] == 'n')
				time_value = value.minutes();
			else
			{
				out.put_padded(invalid_specifier_msg, context);

				return;
			}
			break;

		case 'S':
			specifier = "s";
		case 's':
			time_value = value.seconds();
			break;

		case U'μ':
			specifier = reinterpret_cast<const char*>(u8"μs");
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
				out.pad(chars, context);

			if (sign)
				out.put(sign);

			if (days)
			{
				h_fmt_decimal(out, value.days(), log10_d);

				out.put('d');
				out.put(' ');
				out.put('+');
				out.put(' ');
			}

			h_fmt_two_digit(out, value.hours() % 24);

			out.put(':');

			h_fmt_two_digit(out, value.minutes() % 60);

			out.put(':');

			h_fmt_two_digit(out, value.seconds() % 60);

			out.put('.');

			h_fmt_three_digit(out, value.milliseconds() % 1000);

			out.put('.');

			h_fmt_three_digit(out, value.microseconds() % 1000);

			out.put('h');

			if (!is_rightadj(context))
				out.pad(chars, context);
		}
		return;

		case 'l':
			out.put_padded(och::stringview("[[fmt_timespan - specifier 'l' is not yet implemented]]"), context);

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

			out.put_padded(och::stringview("[[fmt_timespan - specifier 'x' is not yet implemented]]"), context);
			return;

		default:
			out.put_padded(invalid_specifier_msg, context);
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
			out.pad(chars, context);

		h_fmt_decimal(out, time_value, log10_v, sign);

		if (specifier)
			out.put(och::stringview(specifier, specifier_len, 1));

		if (!is_rightadj(context))
			out.pad(chars, context);
	}

	//TODO implement
	void fmt_highres_timespan(type_union arg_value, const parsed_context& context) noexcept
	{
		arg_value;

		context.output.put_padded(och::stringview("[[fmt_highres_timespan is not yet implemented]]"), context);

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

	void fmt_mat4(type_union arg_value, const parsed_context& context) noexcept
	{
		h_fmt_mat<4>(arg_value, context);
	}

	void fmt_mat3(type_union arg_value, const parsed_context& context) noexcept
	{
		h_fmt_mat<3>(arg_value, context);
	}

	void fmt_mat2(type_union arg_value, const parsed_context& context) noexcept
	{
		h_fmt_mat<2>(arg_value, context);
	}

	void fmt_vec4(type_union arg_value, const parsed_context& context) noexcept
	{
		h_fmt_vec<4>(arg_value, context);
	}

	void fmt_vec3(type_union arg_value, const parsed_context& context) noexcept
	{
		h_fmt_vec<3>(arg_value, context);
	}

	void fmt_vec2(type_union arg_value, const parsed_context& context) noexcept
	{
		h_fmt_vec<2>(arg_value, context);
	}

	void fmt_virtual_keycode(type_union arg_value, const parsed_context& context) noexcept
	{
		context.output.put(och::stringview(virtual_key_names[arg_value.u8]));
	}

	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*////////////////////////////////////////////////////arg_wrapper////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	arg_wrapper create_fmt_arg_wrapper(uint64_t value) noexcept { return { value, fmt_uint64 }; }

	arg_wrapper create_fmt_arg_wrapper(int64_t value) noexcept { return { value, fmt_int64 }; }

	arg_wrapper create_fmt_arg_wrapper(uint32_t value) noexcept { return { value, fmt_uint32 }; }

	arg_wrapper create_fmt_arg_wrapper(int32_t value) noexcept { return { value, fmt_int32 }; }

	arg_wrapper create_fmt_arg_wrapper(uint16_t value) noexcept { return { value, fmt_uint16 }; }

	arg_wrapper create_fmt_arg_wrapper(int16_t value) noexcept { return { value, fmt_int16 }; }

	arg_wrapper create_fmt_arg_wrapper(uint8_t value) noexcept { return { value, fmt_uint8 }; }

	arg_wrapper create_fmt_arg_wrapper(int8_t value) noexcept { return { value, fmt_int8 }; }

	arg_wrapper create_fmt_arg_wrapper(float value) noexcept { return { value, fmt_float }; }

	arg_wrapper create_fmt_arg_wrapper(double value) noexcept { return { value, fmt_double }; }

	arg_wrapper create_fmt_arg_wrapper(const char* value) noexcept { return { (const void*)value, fmt_cstring }; }

	arg_wrapper create_fmt_arg_wrapper(const och::utf8_string& value) noexcept { return { (const void*)&value, fmt_utf8_string }; }

	arg_wrapper create_fmt_arg_wrapper(const och::utf8_view& value) noexcept { return { (const void*)&value, fmt_utf8_view }; }

	arg_wrapper create_fmt_arg_wrapper(const och::date& value) noexcept { return { (const void*)&value, fmt_date }; }

	arg_wrapper create_fmt_arg_wrapper(char32_t value) noexcept { return { och::utf8_char(value), fmt_codepoint }; }

	arg_wrapper create_fmt_arg_wrapper(char value) noexcept { return { och::utf8_char(value), fmt_codepoint }; }

	arg_wrapper create_fmt_arg_wrapper(const och::utf8_char& value) noexcept { return { value, fmt_codepoint }; }

	arg_wrapper create_fmt_arg_wrapper(och::timespan value) noexcept { return { value.val, fmt_timespan }; }

	arg_wrapper create_fmt_arg_wrapper(och::highres_timespan value) noexcept { return { value.val, fmt_highres_timespan }; }

	arg_wrapper create_fmt_arg_wrapper(const och::mat4& value) noexcept { return { static_cast<const void*>(&value), fmt_mat4 }; }

	arg_wrapper create_fmt_arg_wrapper(const och::mat3& value) noexcept { return { static_cast<const void*>(&value), fmt_mat3 }; }

	arg_wrapper create_fmt_arg_wrapper(const och::mat2& value) noexcept { return { static_cast<const void*>(&value), fmt_mat2 }; }

	arg_wrapper create_fmt_arg_wrapper(const och::vec4& value) noexcept { return { static_cast<const void*>(&value), fmt_vec4 }; }

	arg_wrapper create_fmt_arg_wrapper(const och::vec3& value) noexcept { return { static_cast<const void*>(&value), fmt_vec3 }; }

	arg_wrapper create_fmt_arg_wrapper(const och::vec2& value) noexcept { return { static_cast<const void*>(&value), fmt_mat2 }; }

	arg_wrapper create_fmt_arg_wrapper(och::vk value) noexcept { return { static_cast<uint8_t>(value), fmt_virtual_keycode }; }

	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////parsed_context//////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	parsed_context::parsed_context(const char*& context, const och::range<const och::arg_wrapper> argv, output_buffer& output) : argv(argv), output(output)
	{
		width = h_parse_fmt_index_relative(context, argv);

		if (*context == '.')
		{
			++context;

			if (*context != '{' && (*context < '0' || *context > '9'))
				precision = 0x7FFF;
			else
				precision = h_parse_fmt_index_relative(context, argv);
		}

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
	uint32_t vprint(uint64_t out, const och::stringview& format, const och::range<const arg_wrapper>& argv, uint32_t buffer_bytes)
	{
		och::range<char> buffer;

		void* backing_structure = nullptr;

		if (buffer_bytes == 0xFFFF'FFFF)
		{
			char buffer_array[output_buffer::file_buffer_capacity];

			buffer = buffer_array;

			backing_structure = reinterpret_cast<void*>(out);
		}
		else if (buffer_bytes == 0xFFFF'FFFE)
		{
			buffer = och::range<char>(nullptr, nullptr);

			backing_structure = reinterpret_cast<void*>(out);
		}
		else
			buffer = och::range<char>(reinterpret_cast<char*>(out), buffer_bytes);

		output_buffer output(buffer, backing_structure);

		uint32_t arg_counter = 0;

		const char* last_fmt_end = format.raw_cbegin(), * curr = format.raw_cbegin(), * const initial_buffer_start = buffer.beg;

		while (curr < format.raw_cend())
			if (*curr++ == '{')
			{
				if (*curr == '{')
				{
					++curr;

					output.put(och::stringview(last_fmt_end, static_cast<uint32_t>(curr - 1 - last_fmt_end), 1));

					last_fmt_end = curr;

					continue;
				}

				output.put(och::stringview(last_fmt_end, static_cast<uint32_t>(curr - 1 - last_fmt_end), 1));

				uint32_t arg_idx;

				arg_idx = *curr >= '0' && *curr <= '9' ? h_parse_fmt_index(curr) : arg_counter;

				++arg_counter;

				assert(arg_idx < static_cast<uint32_t>(argv.len()));

				assert(*curr == ':' || *curr == '}');

				if (*curr == ':')
					++curr;

				parsed_context format_context(curr, argv, output);

				argv[arg_idx].formatter(argv[arg_idx].value, format_context);

				last_fmt_end = curr;
			}

		output.put(och::stringview(last_fmt_end, static_cast<uint32_t>(curr - last_fmt_end), 1));

		output.finalize();

		if (output.overrun_count)
			return static_cast<uint32_t>(output.buffer.beg - initial_buffer_start + output.overrun_count);
		else
			return static_cast<uint32_t>(output.buffer.beg - initial_buffer_start + output.overrun_count - 1);
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////print///////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	void print(const och::iohandle& out, const och::stringview& format)
	{
		uint32_t bytes_written;
		ignore_status(och::write_to_file(bytes_written, out, och::range<const uint8_t>(reinterpret_cast<const uint8_t*>(format.raw_cbegin()), reinterpret_cast<const uint8_t*>(format.raw_cend()))));
	}

	void print(const och::iohandle& out, const char* format)
	{
		print(out, och::stringview(format));
	}

	void print(const och::iohandle& out, const och::utf8_string& format)
	{
		print(out, och::stringview(format));
	}


	void print(const och::filehandle& out, const och::stringview& format)
	{
		print(out.get_handle_(), format);
	}

	void print(const och::filehandle& out, const char* format)
	{
		print(out.get_handle_(), format);
	}

	void print(const och::filehandle& out, const och::utf8_string& format)
	{
		print(out.get_handle_(), format);
	}


	void print(const och::stringview& format)
	{
		print(och::get_stdout(), format);
	}

	void print(const char* format)
	{
		print(och::get_stdout(), format);
	}

	void print(const och::utf8_string& format)
	{
		print(och::get_stdout(), format);
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



	uint32_t sprint(och::utf8_string& buf, const stringview& format)
	{
		buf += format;

		return format.get_codeunits();
	}

	uint32_t sprint(och::utf8_string& buf, const char* format)
	{
		uint32_t prev_cunits = buf.get_codeunits();

		buf += format;

		return buf.get_codeunits() - prev_cunits;
	}

	uint32_t sprint(och::utf8_string& buf, const utf8_string& format)
	{
		buf += format;

		return format.get_codeunits();
	}
}
