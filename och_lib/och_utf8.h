#pragma once

#include <cstdint>

namespace och
{
	struct utf8_string;

	struct utf8_iterator;

	void _utf8_len(const char* cstring, uint32_t& cunits, uint32_t& cpoints, uint32_t max_cunits = ~0) noexcept;

	uint32_t _utf8_from_codepoint(char* out, char32_t cpoint) noexcept;

	char32_t _utf8_to_utf32(const char* cstring) noexcept;

	uint32_t _utf8_codepoint_bytes(char c) noexcept;

	bool _is_utf8_surr(char c) noexcept;

	struct utf8_char
	{
		union
		{
			char m_codeunits[4]{ 0, 0, 0, 0 };
			uint32_t m_intval;
		};

		utf8_char(const char* cstring) noexcept;

		utf8_char(char32_t codepoint) noexcept;

		utf8_char(char ascii_codepoint) noexcept;

		utf8_char() = default;

		uint32_t get_codeunits() const noexcept;

		char32_t codepoint() const noexcept;

		const char* cbegin() const noexcept;

		const char* cend() const noexcept;

		const char* begin() const noexcept;

		const char* end() const noexcept;

		bool operator==(const utf8_char& rhs) const noexcept;

		bool operator!=(const utf8_char& rhs) const noexcept;

		bool operator>(const utf8_char& rhs) const noexcept;

		bool operator>=(const utf8_char& rhs) const noexcept;

		bool operator<(const utf8_char& rhs) const noexcept;

		bool operator<=(const utf8_char& rhs) const noexcept;
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

	struct utf8_iterator
	{
		const char* cstring;

		utf8_iterator(const char* cstring) noexcept;

		utf8_char operator*() noexcept;

		void operator++() noexcept;

		bool operator!=(const utf8_iterator& rhs) noexcept;
	};

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

		void operator+=(char32_t codept) noexcept;

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
}
