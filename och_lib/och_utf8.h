#pragma once

#include <cstdint>

namespace och
{
	struct utf8_string;

	struct utf8_iterator;

	void _utf8_len(const char* cstring, uint32_t& cunits, uint32_t& cpoints, uint32_t max_cunits = ~0) noexcept;

	uint32_t _utf8_from_codepoint(char* out, char32_t cpoint) noexcept;

	char32_t _utf8_to_utf32(const char* cstring) noexcept;

	uint32_t _utf8_codepoint_bytes(const char* cstring) noexcept;

	bool _is_utf8_surr(char c) noexcept;

	struct utf8_codepoint
	{
		char utf8[4] alignas(char32_t) { 0, 0, 0, 0 };

		utf8_codepoint(const char* cstring) noexcept;

		utf8_codepoint(char32_t codepoint) noexcept;

		utf8_codepoint() = default;

		uint32_t get_codeunits() const noexcept;
	};
	
	struct utf8_view
	{
		const char* m_ptr;
		uint32_t m_codeunits;
		uint32_t m_codepoints;

		utf8_view(const char* cstring) noexcept;

		utf8_view(const char* cstring, uint32_t codeunits, uint32_t codepoints) noexcept;

		utf8_view(const utf8_string& string) noexcept;

		utf8_view subview(uint32_t pos, uint32_t len) const noexcept;

		uint32_t get_codepoints() const noexcept;

		uint32_t get_codeunits() const noexcept;
		
		const char* raw_cbegin() const noexcept;

		const char* raw_cend() const noexcept;

		utf8_iterator begin() const noexcept;

		utf8_iterator end() const noexcept;
	};

	struct utf8_iterator
	{
		const char* cstring;

		utf8_iterator(const char* cstring) noexcept;

		char32_t operator*() noexcept;

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

		char32_t at(uint32_t pos) const noexcept;

		char32_t front() const noexcept;

		char32_t back() const noexcept;

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

		bool insert(uint32_t pos, char32_t codept) noexcept;

		bool insert(uint32_t pos, const utf8_view& view) noexcept;

		bool insert(uint32_t pos, const char* cstr) noexcept;

		bool replace(uint32_t pos, char32_t codept) noexcept;

		bool replace(uint32_t pos, const utf8_view& view) noexcept;

		bool replace(uint32_t pos, const char* cstr) noexcept;

		bool operator==(const utf8_string& rhs) const noexcept;

		bool operator!=(const utf8_string& rhs) const noexcept;

		bool operator>(const utf8_string& rhs) const noexcept;

		bool operator>=(const utf8_string& rhs) const noexcept;

		bool operator<(const utf8_string& rhs) const noexcept;

		bool operator<=(const utf8_string& rhs) const noexcept;

		//////////////////////////////////////////////////Operations///////////////////////////////////////////////////////////////

		utf8_string substr(uint32_t pos, uint32_t len = static_cast<uint32_t>(-1)) const noexcept;

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
