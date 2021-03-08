#pragma once

#include <cstdint>

namespace och
{
	struct utf8_string;

	void _utf8_len(const char* cstring, uint32_t& cunits, uint32_t& cpoints, uint32_t max_cunits = ~0) noexcept;

	constexpr uint32_t _utf8_from_codepoint(char* out, char32_t cpoint) noexcept
	{
		if (cpoint > 0xFFFF)
		{
			out[0] = static_cast<char>(0xF0 | cpoint >> 18);
			out[1] = static_cast<char>(0x80 | ((cpoint >> 12) & 0x3F));
			out[2] = static_cast<char>(0x80 | ((cpoint >> 6) & 0x3F));
			out[3] = static_cast<char>(0x80 | (cpoint & 0x3F));

			return 4;
		}

		if (cpoint > 0x07FF)
		{
			out[0] = static_cast<char>(0xE0 | cpoint >> 12);
			out[1] = static_cast<char>(0x80 | ((cpoint >> 6) & 0x3F));
			out[2] = static_cast<char>(0x80 | (cpoint & 0x3F));

			return 3;
		}
		if (cpoint > 0x007F)
		{
			out[0] = static_cast<char>(0xC0 | cpoint >> 6);
			out[1] = static_cast<char>(0x80 | (cpoint & 0x3F));

			return 2;
		}

		out[0] = static_cast<char>(cpoint);

		return 1;
	}

	constexpr char32_t _utf8_to_utf32(const char* cstring) noexcept
	{
		if (!(*cstring & 0x80))
			return static_cast<char32_t>(*cstring);
		if (!(*cstring & 0x20))
			return (static_cast<char32_t>(cstring[0] & 0x1F) << 6) | (cstring[1] & 0x3F);
		if (!(*cstring & 0x10))
			return (static_cast<char32_t>(cstring[0] & 0x0F) << 12) | ((cstring[1] & 0x3F) << 6) | (cstring[2] & 0x3F);
		if (!(*cstring & 0x08))
			return (static_cast<char32_t>(cstring[0] & 0x07) << 18) | ((cstring[1] & 0x3F) << 12) | ((cstring[2] & 0x3F) << 6) | (cstring[3] & 0x3F);
		else
			return ~U'\0';
	}

	constexpr uint32_t _utf8_codepoint_bytes(unsigned char c) noexcept
	{
		if (c < 0x80)
			return 1;
		else if (c < 0xC0)
			return 2;
		else if (c < 0xE0)
			return 3;
		else
			return 4;
	}

	constexpr bool _is_utf8_surr(char c) noexcept
	{
		return (c & 0xC0) == 0x80;
	}

	struct utf8_char
	{
	private:

		union
		{
			char m_codeunits[4] alignas(uint32_t);
			uint32_t m_intval;
		};

		constexpr uint32_t _init_intval_from_cstring(const char* cstring)
		{
			uint32_t val = (uint32_t) * (const uint8_t*)cstring++;

			int32_t i = 1;

			while (i != 4 && _is_utf8_surr(*cstring))
				val |= ((uint32_t) * (const uint8_t*)cstring++) << (i++ * 8);

			return val;
		}

		constexpr uint32_t _init_intval_from_codepoint(char32_t cpoint)
		{
			if (cpoint > 0xFFFF)
				return 0x808080F0 | (uint32_t)((cpoint >> 18) | (((cpoint >> 12) & 0x3F) << 8) | (((cpoint >> 6) & 0x3F) << 16) | ((cpoint & 0x3F) << 24));
			else if (cpoint > 0x07FF)
				return 0x8080E0 | (uint32_t)((cpoint >> 12) | (((cpoint >> 6) & 0x3F) << 8) | ((cpoint & 0x3F) << 16));
			else if (cpoint > 0x007F)
				return 0x80C0 | (uint32_t)((cpoint >> 6) | ((cpoint & 0x3F) << 8));
			else
				return (uint32_t)cpoint;
		}

	public:

		constexpr utf8_char(const char* cstring) noexcept : m_intval{ _init_intval_from_cstring(cstring) } {}

		constexpr utf8_char(char32_t codepoint) noexcept : m_intval{ _init_intval_from_codepoint(codepoint) } {}

		constexpr utf8_char(char ascii_codepoint) noexcept : m_intval{ (uint32_t)ascii_codepoint } {}

		utf8_char() = default;

		constexpr uint32_t get_codeunits() const noexcept
		{
			return m_intval <= 0xFF ? 1 : m_intval <= 0xFFFF ? 2 : m_intval <= 0xFFFFFF ? 3 : 4;
		}

		constexpr char32_t codepoint() const noexcept
		{
			return _utf8_to_utf32(m_codeunits);
		}

		constexpr uint32_t intval() const noexcept
		{
			return m_intval;
		}

		constexpr const char* cbegin() const noexcept
		{
			return m_codeunits;
		}

		constexpr const char* cend() const noexcept
		{
			return m_codeunits + get_codeunits();
		}

		constexpr const char* begin() noexcept
		{
			return m_codeunits;
		}

		constexpr const char* end() noexcept
		{
			return m_codeunits + get_codeunits();
		}

		static constexpr utf8_char from_raw_value(uint32_t intval)
		{
			utf8_char c{ U'\0' };
			c.m_intval = intval;
			return c;
		}

		constexpr uint32_t get_raw_value() const noexcept
		{
			return m_intval;
		}

		constexpr bool operator==(const utf8_char& rhs) const noexcept
		{
			return m_intval == rhs.m_intval;
		}

		constexpr bool operator!=(const utf8_char& rhs) const noexcept
		{
			return m_intval != rhs.m_intval;
		}

		constexpr bool operator>(const utf8_char& rhs) const noexcept
		{
			return m_intval > rhs.m_intval;
		}

		constexpr bool operator>=(const utf8_char& rhs) const noexcept
		{
			return m_intval >= rhs.m_intval;
		}

		constexpr bool operator<(const utf8_char& rhs) const noexcept
		{
			return m_intval < rhs.m_intval;
		}

		constexpr bool operator<=(const utf8_char& rhs) const noexcept
		{
			return m_intval <= rhs.m_intval;
		}
	};

	struct utf8_iterator
	{
		const char* cstring;

		utf8_iterator(const char* cstring) noexcept;

		utf8_char operator*() const noexcept;

		void operator++() noexcept;

		bool operator!=(const utf8_iterator& rhs) const noexcept;
	};

	struct utf8_view
	{
	private:

		const char* m_ptr;
		uint32_t m_codeunits;
		uint32_t m_codepoints;

		constexpr uint32_t _init_codeunits(const char* cstring) const noexcept
		{
			uint32_t cunits = 0;

			while (*cstring++)
				++cunits;

			return cunits;
		}

		constexpr uint32_t _init_codepoints(const char* cstring) const noexcept
		{
			uint32_t cpoints = 0;

			while (*cstring)
				cpoints += !_is_utf8_surr(*cstring++);

			return cpoints;
		}

	public:

		constexpr utf8_view(const char* cstring) noexcept :
			m_ptr{ cstring },
			m_codeunits{ _init_codeunits(cstring) },
			m_codepoints{ _init_codepoints(cstring) } {}

		constexpr utf8_view(const char* cstring, uint32_t codeunits, uint32_t codepoints) noexcept : m_ptr{ cstring }, m_codeunits{ codeunits }, m_codepoints{ codepoints } {}

		constexpr utf8_view(const utf8_view& view) = default;

		utf8_view(const utf8_string& string) noexcept;

		utf8_view subview(uint32_t pos, uint32_t len = ~0) const noexcept;

		uint32_t get_codepoints() const noexcept;

		uint32_t get_codeunits() const noexcept;

		const char* raw_cbegin() const noexcept;

		const char* raw_cend() const noexcept;

		utf8_iterator begin() const noexcept;

		utf8_iterator end() const noexcept;

		utf8_char at(uint32_t pos) const noexcept;
	};

	using stringview = utf8_view;

	struct utf8_string
	{
		union
		{
			struct
			{
				char* m_ptr;
				uint32_t m_codeunits;
				uint32_t m_codepoints;
				uint32_t m_capacity;
				uint32_t reserved;
			} ls;

			struct
			{
				char m_internal_buf[sizeof(ls) - 1];	//Internal storage for sso
				uint8_t m_internal_cap;					//number of chars left available in sso
			} ss;
		};

		static constexpr uint32_t min_heap_cap = 128;
		static constexpr uint32_t internal_buf_max = sizeof(ss.m_internal_buf);
		static constexpr uint8_t long_string_id = 0xFF;

		////////////////////////////////////////////////////Constructor////////////////////////////////////////////////////////////

		utf8_string() noexcept;

		utf8_string(const utf8_view& view) noexcept;

		utf8_string(const utf8_string& str) noexcept;

		utf8_string(const char* cstr) noexcept;

		~utf8_string() noexcept;

		///////////////////////////////////////////////////Iterators///////////////////////////////////////////////////////////////

		char* raw_begin() noexcept;

		const char* raw_cbegin() const noexcept;

		char* raw_end() noexcept;

		const char* raw_cend() const noexcept;

		utf8_iterator begin() const noexcept;

		utf8_iterator end() const noexcept;

		/////////////////////////////////////////////////////Size//////////////////////////////////////////////////////////////////

		uint32_t get_codeunits() const noexcept;

		uint32_t get_codepoints() const noexcept;

		uint32_t get_capacity() const noexcept;

		uint32_t shrink_to_fit() noexcept;

		bool reserve(uint32_t n) noexcept;

		bool resize(uint32_t n) noexcept;

		bool empty() const noexcept;

		bool is_sso() const noexcept;

		//////////////////////////////////////////////////Element Acess///////////////////////////////////////////////////////////////

		utf8_char at(uint32_t pos) const noexcept;

		utf8_char front() const noexcept;

		utf8_char back() const noexcept;

		///////////////////////////////////////////////////Modifiers///////////////////////////////////////////////////////////////

		void operator=(const utf8_view& view) noexcept;

		void operator=(const utf8_string& string) noexcept;

		void operator=(utf8_string&& string) noexcept;

		void operator=(const char* cstr) noexcept;

		void operator+=(utf8_char codept) noexcept;

		void operator+=(const utf8_view& view) noexcept;

		void operator+=(const utf8_string& str) noexcept;

		void operator+=(const char* cstr) noexcept;

		uint32_t pop(uint32_t n) noexcept;

		void clear() noexcept;

		void erase(uint32_t pos, uint32_t len) noexcept;

		bool insert(uint32_t pos, utf8_char codept) noexcept;

		bool insert(uint32_t pos, const utf8_view& view) noexcept;

		bool insert(uint32_t pos, const char* cstr) noexcept;

		bool replace(uint32_t pos, utf8_char codept) noexcept;

		bool replace(uint32_t pos, const utf8_view& view) noexcept;

		bool replace(uint32_t pos, const char* cstr) noexcept;

		bool operator==(const utf8_string& rhs) const noexcept;

		bool operator!=(const utf8_string& rhs) const noexcept;

		bool operator>(const utf8_string& rhs) const noexcept;

		bool operator>=(const utf8_string& rhs) const noexcept;

		bool operator<(const utf8_string& rhs) const noexcept;

		bool operator<=(const utf8_string& rhs) const noexcept;

		//////////////////////////////////////////////////Operations///////////////////////////////////////////////////////////////

		utf8_string substr(uint32_t pos, uint32_t len = ~0) const noexcept;

	private:

		utf8_string(const char* cstr, uint32_t wds, uint32_t cps) noexcept;

		void construct_ss(const char* cstring, uint32_t words) noexcept;

		void construct_ls(const char* cstring, uint32_t words, uint32_t codepoints) noexcept;

		bool set_to(const char* cstring, uint32_t words, uint32_t codepts) noexcept;

		bool append(const char* cstring, uint32_t words, uint32_t codepts) noexcept;

		bool insert(const char* cstring, uint32_t new_words, uint32_t new_codepts, uint32_t pos) noexcept;

		bool replace(const char* cstring, uint32_t new_words, uint32_t new_codepts, uint32_t pos) noexcept;

		void step(uint32_t& wd_idx, uint32_t& cp_idx, uint32_t last_cp_idx) const noexcept;

		bool deactivate_sso(uint32_t alloc_size) noexcept;

		bool try_activate_sso() noexcept;

		bool change_heap_cap(uint32_t new_cap) noexcept;

		uint32_t least_bigger_cap(uint32_t required) noexcept;

		void set_codeunits(uint32_t wds) noexcept;

		void set_codepoints(uint32_t cps) noexcept;
	};

	using string = utf8_string;

	template<size_t N>
	uint32_t most_codepoints(const char* (&arr)[N])
	{
		uint32_t longest_cpoints = 0;

		for (auto& str : arr)
		{
			uint32_t curr_cpoints = 0;
			uint32_t cunits = 0;

			while (str[cunits])
			{
				++cunits;

				curr_cpoints += !_is_utf8_surr(str[cunits]);
			}

			if (curr_cpoints > longest_cpoints)
				longest_cpoints = curr_cpoints;
		}

		return longest_cpoints;
	}
}
