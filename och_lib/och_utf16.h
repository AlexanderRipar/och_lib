#pragma once

#include <cstdint>

namespace och
{
	struct utf16_iterator
	{
		const char16_t* cstring;

		utf16_iterator(const char16_t* cstring);

		char32_t operator*();

		void operator++();

		bool operator!=(const utf16_iterator& rhs);
	};

	struct utf16_view
	{
		static const char16_t empty_str = u'\0';

		const char16_t* m_ptr;
		uint32_t m_codeunits;
		uint32_t m_codepoints;

		utf16_view();

		utf16_view(const char16_t* cstring);

		utf16_view(const char16_t* cstring, uint32_t cunits, uint32_t cpoints);

		utf16_view subview(uint32_t pos, uint32_t len = ~0);
	};

	struct utf16_string
	{
		union
		{
			struct
			{
				char16_t* m_ptr;
				uint32_t m_codeunits;										//char16_ts used in string (excluding terminating u'\0'
				uint32_t m_capacity;										//char16_ts currently storable without reallocating
				uint32_t m_codepoints;										//number of actual unicode-codepoints encoded in string
				int32_t m_resesrved;										//Gets String to 3*8 bytes, and allows storing up to 11 characters in internal buffer, while also serving to distinguish sso
			} ls;

			struct
			{
				char16_t m_internal_buf[sizeof(ls) / sizeof(char16_t) - 1];	//Internal storage for sso
				uint16_t m_internal_cap;									//number of char16_ts left available in sso
			} ss;
		};

		static constexpr uint32_t min_heap_cap = 128;
		static constexpr uint32_t internal_buf_max = sizeof(ss.m_internal_buf) / sizeof(char16_t);
		static constexpr uint16_t long_string_id = 0xFFFF;

		////////////////////////////////////////////////////Constructor////////////////////////////////////////////////////////////

		utf16_string();

		utf16_string(const utf16_view& view);

		utf16_string(const utf16_string& str);

		utf16_string(const char16_t* cstr);

	private:

	public:

		~utf16_string();

		///////////////////////////////////////////////////Iterators///////////////////////////////////////////////////////////////

		char16_t* raw_begin();

		const char16_t* raw_cbegin() const;

		char16_t* raw_end();

		const char16_t* raw_cend() const;

		utf16_iterator begin() const;

		utf16_iterator end() const;

		/////////////////////////////////////////////////////Size//////////////////////////////////////////////////////////////////

		uint32_t get_codeunits() const;

		uint32_t get_codepoints() const;

		uint32_t get_capacity() const;

		uint32_t shrink_to_fit();

		bool reserve(uint32_t n);

		bool resize(uint32_t n);

		bool empty() const;

		bool is_sso() const;

		//////////////////////////////////////////////////Element Acess///////////////////////////////////////////////////////////////

		char32_t at(uint32_t pos) const;

		char32_t front() const;

		char32_t back() const;

		///////////////////////////////////////////////////Modifiers///////////////////////////////////////////////////////////////

		void operator=(const utf16_view& view) noexcept;

		void operator=(const utf16_string& string) noexcept;

		void operator=(utf16_string&& string) noexcept;

		void operator=(const char16_t* cstr);

		void operator+=(char32_t codept);

		void operator+=(const utf16_view& view);

		void operator+=(const utf16_string& str);

		void operator+=(const char16_t* cstr);

		uint32_t pop(uint32_t n);

		void clear();

		void erase(uint32_t pos, uint32_t len);

		bool insert(uint32_t pos, char32_t codept);

		bool insert(uint32_t pos, const utf16_view& view);

		bool insert(uint32_t pos, const char16_t* cstr);

		bool replace(uint32_t pos, char32_t codept);

		bool replace(uint32_t pos, const utf16_view& view);

		bool replace(uint32_t pos, const char16_t* cstr);

		bool operator==(const utf16_string& rhs) const;

		bool operator!=(const utf16_string& rhs) const;

		bool operator>(const utf16_string& rhs) const;

		bool operator>=(const utf16_string& rhs) const;

		bool operator<(const utf16_string& rhs) const;

		bool operator<=(const utf16_string& rhs) const;

		//////////////////////////////////////////////////Operations///////////////////////////////////////////////////////////////

		utf16_string substr(uint32_t pos, uint32_t len = ~0) const;

	private:

		utf16_string(const char16_t* cstring, uint32_t cunits, uint32_t cpoints);

		void construct_ss(const char16_t* cstring, uint32_t cunits);

		void construct_ls(const char16_t* cstring, uint32_t cunits, uint32_t cpoints);

		bool set_to(const char16_t* cstring, uint32_t cunits, uint32_t cpoints);

		bool append(const char16_t* cstring, uint32_t added_cunits, uint32_t added_cpoints);

		bool insert(const char16_t* cstring, uint32_t added_cunits, uint32_t added_cpoints, uint32_t pos);

		bool replace(const char16_t* cstring, uint32_t added_cunits, uint32_t added_cpoints, uint32_t pos);

		void step(uint32_t& cunit, uint32_t& cpoint, uint32_t last_cpoint) const;

		bool deactivate_sso(uint32_t cunits);

		bool try_activate_sso();

		bool change_heap_cap(uint32_t cunits);

		uint32_t least_bigger_cap(uint32_t required);

		void set_codeunits(uint32_t cunits);

		void set_codepoints(uint32_t cpoints);
	};
}
