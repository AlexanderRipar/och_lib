#pragma once

#include <cstdint>

struct utf8_view
{
	const char* m_ptr;
	uint32_t m_codeunits;
	uint32_t m_codepoints;
};

struct utf8_iterator
{
	const char* cstring;

	utf8_iterator(const char* cstring);

	char32_t operator*();

	void operator++();

	bool operator!=(const utf8_iterator& rhs);
};

struct utf8_string
{
	union
	{
		struct
		{
			char* m_ptr;
			uint32_t m_codeunits;
			uint32_t m_capacity;
			uint32_t m_codepoints;
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

	utf8_string();

	utf8_string(const utf8_view& view);

	utf8_string(const utf8_string& str);

	utf8_string(const char* cstr);

	~utf8_string();

	///////////////////////////////////////////////////Iterators///////////////////////////////////////////////////////////////

	char* raw_begin();

	const char* raw_cbegin() const;

	char* raw_end();

	const char* raw_cend() const;

	utf8_iterator begin() const;

	utf8_iterator end() const;

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

	void operator=(const utf8_view& view) noexcept;

	void operator=(const utf8_string& string) noexcept;

	void operator=(utf8_string&& string) noexcept;

	void operator=(const char* cstr);

	void operator+=(char32_t codept);

	void operator+=(const utf8_view& view);

	void operator+=(const utf8_string& str);

	void operator+=(const char* cstr);

	uint32_t pop(uint32_t n);

	void clear();

	void erase(uint32_t pos, uint32_t len);

	bool insert(uint32_t pos, char32_t codept);

	bool insert(uint32_t pos, const utf8_view& view);

	bool insert(uint32_t pos, const char* cstr);

	bool replace(uint32_t pos, char32_t codept);

	bool replace(uint32_t pos, const utf8_view& view);

	bool replace(uint32_t pos, const char* cstr);

	bool operator==(const utf8_string& rhs) const;

	bool operator!=(const utf8_string& rhs) const;

	bool operator>(const utf8_string& rhs) const;

	bool operator>=(const utf8_string& rhs) const;

	bool operator<(const utf8_string& rhs) const;

	bool operator<=(const utf8_string& rhs) const;

	//////////////////////////////////////////////////Operations///////////////////////////////////////////////////////////////

	utf8_string substr(uint32_t pos, uint32_t len = static_cast<uint32_t>(-1)) const;

private:

	utf8_string(const char* cstr, uint32_t wds, uint32_t cps);

	void construct_ss(const char* cstring, uint32_t words);

	void construct_ls(const char* cstring, uint32_t words, uint32_t codepoints);

	bool set_to(const char* cstring, uint32_t words, uint32_t codepts);

	bool append(const char* cstring, uint32_t words, uint32_t codepts);

	bool insert(const char* cstring, uint32_t new_words, uint32_t new_codepts, uint32_t pos);

	bool replace(const char* cstring, uint32_t new_words, uint32_t new_codepts, uint32_t pos);

	void step(uint32_t& wd_idx, uint32_t& cp_idx, uint32_t last_cp_idx) const;

	bool deactivate_sso(uint32_t alloc_size);

	bool try_activate_sso();

	bool change_heap_cap(uint32_t new_cap);

	uint32_t least_bigger_cap(uint32_t required);

	void set_codeunits(uint32_t wds);

	void set_codepoints(uint32_t cps);
};
