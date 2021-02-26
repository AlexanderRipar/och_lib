#pragma once

#include "och_utf8.h"
#include "och_fio.h"

namespace och
{
	template<uint16_t N, bool Reverse>
	struct utf8_buffer
	{
		const uint16_t size = N;

		uint16_t m_codeunits = 0;

		uint16_t m_codepoints = 0;

		uint16_t m_head = Reverse ? N - 1 : 0;

		char m_buf[N];

		bool push(const utf8_string& string) noexcept
		{
			return push(string.raw_cbegin(), string.get_codeunits());
		}

		bool push(const utf8_view& view) noexcept
		{
			return push(view.raw_cbegin(), view.get_codeunits());
		}

		bool push(const char* cstring) noexcept
		{
			uint32_t cunits = 0, cpoints = 0;

			_utf8_len(cstring, cunits, cpoints);

			return push(cstring, cunits, cpoints);
		}

		bool push(utf8_char c) noexcept
		{
			return push(c.cbegin(), c.get_codeunits());
		}

		bool fill(utf8_char c, uint32_t n)
		{
			const uint32_t codeunits_added = n * c.get_codeunits();

			if constexpr (Reverse)
			{
				if (m_head - codeunits_added >= N)
					return false;

				m_head -= codeunits_added;
			}
			else
			{
				if (m_head + codeunits_added >= N)
					return false;

				m_head += (uint16_t)codeunits_added;
			}

			for (uint32_t i = 0; i != n; ++i)
				for (uint32_t j = 0; j != c.get_codeunits(); ++j)
					m_buf[m_head + i * c.get_codeunits() + j] = c.cbegin()[j];

			return true;
		}

		void pop(const char* cstring, uint32_t n)
		{
			if constexpr (Reverse)
				while (n && m_head != N)
					n -= !_is_utf8_surr(m_buf[m_head++]);
			else
				while (n && m_head != ~0)
					n -= !_is_utf8_surr(m_buf[m_head--]);

		}

		void clear() noexcept
		{
			if constexpr (Reverse)
				m_head = N - 1;
			else
				m_head = 0;
		}

		void flush(och::iohandle outfile)
		{
			if constexpr (Reverse)
			{
				och::write_to_file(outfile, och::range<const char>(m_buf + m_head + 1, m_buf + N));

				m_head = N - 1;
			}
			else
			{
				och::write_to_file(outfile, och::range<const char>(m_buf, m_buf + m_head));

				m_head = 0;
			}
		}

		void flush(const och::filehandle& outfile)
		{
			flush(outfile.handle);
		}

		char* reserve(uint16_t codeunits_to_reserve, och::iohandle outfile)
		{
			char* ret = m_buf + m_head;

			if constexpr (Reverse)
			{
				if (m_head - codeunits_to_reserve >= N)
					flush(outfile);

				m_head -= codeunits_to_reserve;
			}
			else
			{
				if (m_head + codeunits_to_reserve >= N)
					flush(outfile);

				m_head += codeunits_to_reserve;
			}

			return ret;
		}

	private:

		bool push(const char* cstring, uint32_t codeunits_added) noexcept
		{
			if constexpr (Reverse)
			{
				if (m_head - codeunits_added >= N)
					return false;

				m_head -= codeunits_added;

				for (uint32_t i = 0; i != codeunits_added; ++i)
					m_buf[m_head + i] = cstring[i];
			}
			else
			{
				if (m_head + codeunits_added >= N)
					return false;

				for (uint32_t i = 0; i != codeunits_added; ++i)
					m_buf[m_head + i] = cstring[i];

				m_head += (uint16_t)codeunits_added;
			}

			return true;
		}
	};

	template<uint16_t N>
	using forward_utf8_buffer = utf8_buffer<N, false>;

	template<uint16_t N>
	using reverse_utf8_buffer = utf8_buffer<N, true>;
}
