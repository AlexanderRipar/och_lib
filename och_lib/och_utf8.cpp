#include "och_utf8.h"

#include <cstdint>
#include <cstdlib>

namespace och
{
	void _utf8_len(const char* str, uint32_t& cunits, uint32_t& cpoints, uint32_t max_cpoints)
	{
		while (cpoints < max_cpoints && str[cunits])
		{
			++cunits;

			cpoints += ((str[cunits] & 0xC0) != 0x80);
		}
	}

	uint32_t _utf8_from_codepoint(char* out, char32_t cpoint)
	{
		if (cpoint > 0xFFFF)
		{
			out[0] = static_cast<char>(0xF0 |   cpoint >> 18         );
			out[1] = static_cast<char>(0x80 | ((cpoint >> 12) & 0x3F));
			out[2] = static_cast<char>(0x80 | ((cpoint >>  6) & 0x3F));
			out[3] = static_cast<char>(0x80 |  (cpoint        & 0x3F));

			return 4;
		}

		if (cpoint > 0x07FF)
		{
			out[0] = static_cast<char>(0xE0 |   cpoint >> 12         );
			out[1] = static_cast<char>(0x80 | ((cpoint >>  6) & 0x3F));
			out[2] = static_cast<char>(0x80 |  (cpoint        & 0x3F));

			return 3;
		}
		if (cpoint > 0x007F)
		{
			out[0] = static_cast<char>(0xC0 |   cpoint >> 6          );
			out[1] = static_cast<char>(0x80 |  (cpoint        & 0x3F));

			return 2;
		}

		out[0] = static_cast<char>(cpoint);

		return 1;
	}

	char32_t _utf8_to_utf32(const char* str)
	{
		if (!(*str & 0x80))
			return static_cast<char32_t>(*str);
		if (!(*str & 0x20))
			return (static_cast<char32_t>(str[0] & 0x1F) << 6) | (str[1] & 0x3F);
		if (!(*str & 0x10))
			return (static_cast<char32_t>(str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
		if (!(*str & 0x08))
			return (static_cast<char32_t>(str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
		else
			return ~U'\0';
	}

	uint32_t _utf8_codepoint_bytes(const char* str)
	{
		if (!(*str & 0x80))
			return 1;
		else if (!(*str & 0x20))
			return 2;
		else if (!(*str & 0x10))
			return 3;
		else if (!(*str & 0x08))
			return 4;
		else
			return 1;
	}

	bool _is_utf8_surr(char c)
	{
		return (c & 0xC0) == 0x80;
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////////utf8_codepoint////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	utf8_codepoint::utf8_codepoint(const char* cstring) noexcept
	{
		*utf8 = *cstring;

		uint32_t cunits = 1;

		while (cunits != 4 && _is_utf8_surr(cstring[cunits]))
		{
			utf8[cunits] = cstring[cunits];
			++cunits;
		}
	}

	utf8_codepoint::utf8_codepoint(char32_t codepoint) noexcept
	{
		uint32_t cunits = _utf8_from_codepoint(utf8, codepoint);
	}

	uint32_t utf8_codepoint::get_codeunits() const noexcept
	{
		uint32_t cunits = 0;

		while (cunits != 4 && utf8[cunits])
			++cunits;

		return cunits;
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////////utf8_iterator/////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	utf8_iterator::utf8_iterator(const char* cstring) : cstring{ cstring } {}

	char32_t utf8_iterator::operator*()
	{
		return _utf8_to_utf32(cstring);
	}

	void utf8_iterator::operator++()
	{
		while ((*++cstring & 0xC0) == 0x80);
	}

	bool utf8_iterator::operator!=(const utf8_iterator& rhs)
	{
		return cstring < rhs.cstring;
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*//////////////////////////////////////////////////////utf8_view////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////////utf8_string//////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	////////////////////////////////////////////////////Constructor////////////////////////////////////////////////////////////////

	utf8_string::utf8_string() : ss{ {0}, internal_buf_max } {}

	utf8_string::utf8_string(const utf8_view& view)
	{
		view.m_codeunits <= internal_buf_max ? construct_ss(view.m_ptr, view.m_codeunits) : construct_ls(view.m_ptr, view.m_codeunits, view.m_codepoints);
	}

	utf8_string::utf8_string(const utf8_string& str)
	{
		str.get_codeunits() <= internal_buf_max ? construct_ss(str.raw_cbegin(), str.get_codeunits()) : construct_ls(str.raw_cbegin(), str.get_codeunits(), str.get_codepoints());
	}

	utf8_string::utf8_string(const char* cstring)
	{
		uint32_t words = 0, codepts = 0;

		_utf8_len(cstring, words, codepts);

		words <= internal_buf_max ? construct_ss(cstring, words) : construct_ls(cstring, words, codepts);
	}

	utf8_string::~utf8_string()
	{
		if (!is_sso())
			free(ls.m_ptr);
	}

	/////////////////////////////////////////////////////Iterators/////////////////////////////////////////////////////////////////

	char* utf8_string::raw_begin()
	{
		return is_sso() ? ss.m_internal_buf : ls.m_ptr;
	}

	const char* utf8_string::raw_cbegin() const
	{
		return is_sso() ? ss.m_internal_buf : ls.m_ptr;
	}

	char* utf8_string::raw_end()
	{
		return is_sso() ? ss.m_internal_buf + internal_buf_max - ss.m_internal_cap : ls.m_ptr + ls.m_codeunits;
	}

	const char* utf8_string::raw_cend() const
	{
		return is_sso() ? ss.m_internal_buf + internal_buf_max - ss.m_internal_cap : ls.m_ptr + ls.m_codeunits;
	}

	utf8_iterator utf8_string::begin() const
	{
		return utf8_iterator(raw_cbegin());
	}

	utf8_iterator utf8_string::end() const
	{
		return utf8_iterator(raw_cend());
	}

	/////////////////////////////////////////////////////Size//////////////////////////////////////////////////////////////////////

	uint32_t utf8_string::get_codeunits() const
	{
		return is_sso() ? internal_buf_max - ss.m_internal_cap : ls.m_codeunits;
	}

	uint32_t utf8_string::get_codepoints() const
	{
		return is_sso() ? [](const char* str) {uint32_t cps = 0; while (*str)cps += ((*(str++) & 0xC0) != 0x80); return cps; }(ss.m_internal_buf) : ls.m_codepoints;
	}

	uint32_t utf8_string::get_capacity() const
	{
		return is_sso() ? internal_buf_max : ls.m_capacity;
	}

	uint32_t utf8_string::shrink_to_fit()
	{
		if (is_sso() || try_activate_sso())
			return internal_buf_max;

		change_heap_cap(get_codeunits());

		return ls.m_capacity;
	}

	bool utf8_string::reserve(uint32_t n)
	{
		if (n <= get_capacity())
			return true;				//min_capacity already exceeded; Nothing to do

		if (is_sso())
			return deactivate_sso(n);	//min_capacity not reached and SSO active

		return change_heap_cap(n);		//min_capacity not reached and SSO inactive
	}

	bool utf8_string::resize(uint32_t n)
	{
		if (get_capacity() == least_bigger_cap(n) || (is_sso() && n <= internal_buf_max))
			return true;				//If capacity is already matched, or sso is active and a fitting capacity is requested, return true

		if (is_sso())
			return deactivate_sso(n);	//Requested capacity does not fit internal buffer; Deactivate sso

		return change_heap_cap(n);		//sso already inactive and size actually changed; Change capacity
	}

	bool utf8_string::empty() const
	{
		return !get_codeunits();
	}

	bool utf8_string::is_sso() const
	{
		return ss.m_internal_cap != long_string_id;
	}

	////////////////////////////////////////////////Element Acess//////////////////////////////////////////////////////////////////

	char32_t utf8_string::at(uint32_t pos) const
	{
		if (!is_sso() && get_codepoints() == get_codeunits())
			return raw_cbegin()[pos];

		utf8_iterator it(raw_cbegin());

		for (uint32_t i = 0; i != pos; ++i)
			++it;

		return *it;
	}

	char32_t utf8_string::front() const
	{
		return _utf8_to_utf32(raw_cbegin());
	}

	char32_t utf8_string::back() const
	{
		const char* end = raw_cend() - 1;

		while ((*end & 0xC0) == 0x80)
			--end;

		return _utf8_to_utf32(end);

	}

	/////////////////////////////////////////////////Modifiers/////////////////////////////////////////////////////////////////////

	void utf8_string::operator=(const utf8_view& view) noexcept
	{
		set_to(view.m_ptr, view.m_codeunits, view.m_codepoints);
	}

	void utf8_string::operator=(const utf8_string& string) noexcept
	{
		set_to(string.raw_cbegin(), string.get_codeunits(), string.get_codepoints());
	}

	void utf8_string::operator=(utf8_string&& string) noexcept
	{
		if (!is_sso())
			free(ls.m_ptr);

		ls = string.ls;

		string.ls.m_ptr = nullptr;//To avoid freeing in dtor
	}

	void utf8_string::operator=(const char* cstring)
	{
		uint32_t cunits = 0, cpoints = 0;

		_utf8_len(cstring, cunits, cpoints);

		set_to(cstring, cunits, cpoints);
	}

	void utf8_string::operator+=(char32_t cpoint)
	{
		char buf[4];

		append(buf, _utf8_from_codepoint(buf, cpoint), 1);
	}

	void utf8_string::operator+=(const utf8_view& view)
	{
		append(view.m_ptr, view.m_codeunits, view.m_codepoints);
	}

	void utf8_string::operator+=(const utf8_string& str)
	{
		append(str.raw_cbegin(), str.get_codeunits(), str.get_codepoints());
	}

	void utf8_string::operator+=(const char* cstring)
	{
		uint32_t cunits = 0, cpoints = 0;

		_utf8_len(cstring, cunits, cpoints);

		append(cstring, cunits, cpoints);
	}

	uint32_t utf8_string::pop(uint32_t n)
	{
		uint32_t curr_cunit = get_codeunits();

		uint32_t codepts_popped = 0;

		while (curr_cunit > 0 && codepts_popped < n)
		{
			codepts_popped += !_is_utf8_surr(raw_cbegin()[--curr_cunit]);
		}

		raw_begin()[curr_cunit] = '\0';

		set_codeunits(curr_cunit);

		set_codepoints(get_codepoints() - codepts_popped);

		return codepts_popped;
	}

	void utf8_string::clear()
	{
		if (!is_sso())
			free(ls.m_ptr);

		ss.m_internal_buf[0] = '\0';

		ss.m_internal_cap = internal_buf_max;
	}

	void utf8_string::erase(uint32_t pos, uint32_t len)
	{
		uint32_t prev_cunits = get_codeunits();

		uint32_t prev_cpoints = get_codepoints();

		if (pos >= prev_cpoints)
			return;

		if (pos + len > prev_cpoints)
			len = prev_cpoints - pos;

		uint32_t first_cunit = 0, cpoint_index = 0;

		step(first_cunit, cpoint_index, pos);

		uint32_t last_cunit = first_cunit;

		step(last_cunit, cpoint_index, pos + len);

		uint32_t removed_cunits = last_cunit - first_cunit;

		for (uint32_t i = first_cunit; i != prev_cunits - removed_cunits + 1; ++i)
			raw_begin()[i] = raw_cbegin()[i + removed_cunits];

		set_codeunits(prev_cunits - removed_cunits);

		set_codepoints(prev_cpoints - len);
	}

	bool utf8_string::insert(uint32_t pos, char32_t cpoint)
	{
		char buf[4];

		return insert(buf, _utf8_from_codepoint(buf, cpoint), 1, pos);
	}

	bool utf8_string::insert(uint32_t pos, const utf8_view& view)
	{
		return insert(view.m_ptr, view.m_codeunits, view.m_codepoints, pos);
	}

	bool utf8_string::insert(uint32_t pos, const char* cstring)
	{
		uint32_t added_cunits = 0, added_cpoints = 0;

		_utf8_len(cstring, added_cunits, added_cpoints);

		return insert(cstring, added_cunits, added_cpoints, pos);
	}

	bool utf8_string::replace(uint32_t pos, char32_t cpoint)
	{
		char buf[4];

		return replace(buf, _utf8_from_codepoint(buf, cpoint), 1, pos);
	}

	bool utf8_string::replace(uint32_t pos, const utf8_view& view)
	{
		return replace(view.m_ptr, view.m_codeunits, view.m_codepoints, pos);
	}

	bool utf8_string::replace(uint32_t pos, const char* cstring)
	{
		uint32_t added_cunits = 0, added_cpoints = 0;

		_utf8_len(cstring, added_cunits, added_cpoints);

		return replace(cstring, added_cunits, added_cpoints, pos);
	}

	bool utf8_string::operator==(const utf8_string& rhs) const
	{
		if (get_codeunits() != rhs.get_codeunits() || get_codepoints() != rhs.get_codepoints())
			return false;

		for (uint32_t i = 0; i != get_codeunits(); ++i)
			if (raw_cbegin()[i] != rhs.raw_cbegin()[i])
				return false;

		return true;
	}

	bool utf8_string::operator!=(const utf8_string& rhs) const
	{
		return !operator==(rhs);
	}

	bool utf8_string::operator>(const utf8_string& rhs) const
	{
		uint32_t min_cps = get_codepoints() < rhs.get_codepoints() ? get_codepoints() : rhs.get_codepoints();

		const char* l_ptr = raw_cbegin();
		const char* r_ptr = rhs.raw_cbegin();

		for (uint32_t i = 0; i != min_cps; ++i)
			if (l_ptr[i] != r_ptr[i])
				return l_ptr[i] > r_ptr[i];

		return get_codepoints() > rhs.get_codepoints();
	}

	bool utf8_string::operator>=(const utf8_string& rhs) const
	{
		return !operator<(rhs);
	}

	bool utf8_string::operator<(const utf8_string& rhs) const
	{
		uint32_t min_cps = get_codepoints() < rhs.get_codepoints() ? get_codepoints() : rhs.get_codepoints();

		const char* l_ptr = raw_cbegin();
		const char* r_ptr = rhs.raw_cbegin();

		for (uint32_t i = 0; i != min_cps; ++i)
			if (l_ptr[i] != r_ptr[i])
				return l_ptr[i] < r_ptr[i];

		return get_codepoints() < rhs.get_codepoints();
	}

	bool utf8_string::operator<=(const utf8_string& rhs) const
	{
		return !operator>(rhs);
	}

	//////////////////////////////////////////////////Operations///////////////////////////////////////////////////////////////////

	utf8_string utf8_string::substr(uint32_t pos, uint32_t len) const
	{
		uint32_t prev_cpoints = get_codepoints();

		if (pos >= prev_cpoints)
			return utf8_string();

		uint32_t first_cunit = 0, first_cpoint = 0;

		step(first_cunit, first_cpoint, pos);

		uint32_t last_cunit = first_cunit, last_cpoint = first_cpoint;

		if (len >= prev_cpoints - pos)
			return utf8_string(raw_cbegin() + first_cunit, get_codeunits() - first_cunit, prev_cpoints - pos);

		step(last_cunit, last_cpoint, pos + len);

		return utf8_string(raw_cbegin() + first_cunit, last_cunit - first_cunit, len);
	}

	///////////////////////////////////////////////////Private/////////////////////////////////////////////////////////////////////

	utf8_string::utf8_string(const char* cstring, uint32_t cunits, uint32_t cpoints)
	{
		cunits <= internal_buf_max ? construct_ss(cstring, cunits) : construct_ls(cstring, cunits, cpoints);
	}

	void utf8_string::construct_ss(const char* cstring, uint32_t cunits)
	{
		for (uint32_t i = 0; i != cunits; ++i)
			ss.m_internal_buf[i] = cstring[i];

		ss.m_internal_buf[cunits] = '\0';

		ss.m_internal_cap = static_cast<uint8_t>(internal_buf_max - cunits);
	}

	void utf8_string::construct_ls(const char* cstring, uint32_t cunits, uint32_t cpoints)
	{
		ls.m_capacity = least_bigger_cap(cunits);

		ls.m_ptr = (char*)malloc(ls.m_capacity * sizeof(char));

		if (!ls.m_ptr)
		{
			ss.m_internal_cap = internal_buf_max;

			return;
		}

		for (uint32_t i = 0; i != cunits; ++i)
			ls.m_ptr[i] = cstring[i];

		ls.m_ptr[cunits] = '\0';

		ls.m_codeunits = cunits;

		ls.m_codepoints = cpoints;

		ss.m_internal_cap = long_string_id;
	}

	bool utf8_string::set_to(const char* cstring, uint32_t cunits, uint32_t cpoints)
	{
		if (!resize(cunits))
			return false;

		for (uint32_t i = 0; i != cunits; ++i)
			raw_begin()[i] = cstring[i];

		raw_begin()[cunits] = '\0';

		set_codeunits(cunits);

		set_codepoints(cpoints);

		return true;
	}

	bool utf8_string::append(const char* cstring, uint32_t added_cunits, uint32_t added_cpoints)
	{
		uint32_t prev_cunits = get_codeunits();
		uint32_t prev_cpoints = get_codepoints();

		if (!reserve(prev_cunits + added_cunits))
			return false;

		for (uint32_t i = 0; i != added_cunits; ++i)
			raw_begin()[prev_cunits + i] = cstring[i];

		raw_begin()[prev_cunits + added_cunits] = u'\0';

		set_codeunits(prev_cunits + added_cunits);

		set_codepoints(prev_cpoints + added_cpoints);

		return true;
	}

	bool utf8_string::insert(const char* cstring, uint32_t added_cunits, uint32_t added_cpoints, uint32_t pos)
	{
		uint32_t prev_cunits = get_codeunits();
		uint32_t prev_cpoints = get_codepoints();

		if (!reserve(prev_cunits + added_cunits))
			return false;

		if (pos >= get_codepoints())
			pos = get_codepoints() - 1;

		uint32_t first_cunit = 0, first_cpoint = 0;

		step(first_cunit, first_cpoint, pos);

		for (uint32_t i = prev_cunits; i != first_cunit - 1; --i)
			raw_begin()[i + added_cunits] = raw_begin()[i];

		for (uint32_t i = 0; i != added_cunits; ++i)
			raw_begin()[i + first_cunit] = cstring[i];

		set_codeunits(prev_cunits + added_cunits);
		set_codepoints(prev_cpoints + added_cpoints);

		return true;
	}

	bool utf8_string::replace(const char* cstring, uint32_t added_cunits, uint32_t added_cpoints, uint32_t pos)
	{
		uint32_t prev_cunits = get_codeunits();

		if (pos + added_cpoints > get_codepoints())
			return false;

		uint32_t first_cunit = 0, cpoint_index = 0;

		step(first_cunit, cpoint_index, pos);

		uint32_t last_cunit = first_cunit;

		step(last_cunit, cpoint_index, pos + added_cpoints);

		int32_t cunit_diff = added_cunits - last_cunit + first_cunit;

		if (cunit_diff > 0)												//Fewer codeunits removed than added
		{
			if (!reserve(get_codeunits() + cunit_diff))
				return false;

			for (uint32_t i = get_codeunits(); i != last_cunit - 1; --i)
				raw_begin()[i + cunit_diff] = raw_begin()[i];
		}
		else if (cunit_diff < 0)										//More codeunits removed than added
			for (uint32_t i = last_cunit; i != prev_cunits + 1; ++i)
				raw_begin()[i + cunit_diff] = raw_begin()[i];

		for (uint32_t i = 0; i != added_cunits; ++i)
			raw_begin()[i + first_cunit] = cstring[i];

		set_codeunits(prev_cunits + cunit_diff);

		return true;
	}

	void utf8_string::step(uint32_t& cunit, uint32_t& cpoint, uint32_t last_cpoint) const
	{
		while (cpoint < last_cpoint)
		{
			cunit += _utf8_codepoint_bytes(raw_cbegin() + cunit);

			++cpoint;
		}
	}

	bool utf8_string::deactivate_sso(uint32_t cunits)
	{
		uint32_t alloc_size = least_bigger_cap(cunits);

		char* tmp_ptr = (char*)malloc(alloc_size * sizeof(char));

		if (!tmp_ptr)			//Allocation failed...
			return false;

		for (uint32_t i = 0; i != internal_buf_max - ss.m_internal_cap + 1; ++i)
			tmp_ptr[i] = ss.m_internal_buf[i];

		ls.m_ptr = tmp_ptr;
		ls.m_codeunits = internal_buf_max - ss.m_internal_cap;
		ls.m_capacity = alloc_size;
		ls.m_codepoints = [](const char* str) {uint32_t cps = 0; while (*str)cps += ((*(str++) & 0xC0) != 0x80); return cps; }(tmp_ptr);
		ss.m_internal_cap = long_string_id;

		return true;
	}

	bool utf8_string::try_activate_sso()
	{
		bool can_activate_sso = !is_sso() && get_capacity() <= internal_buf_max;

		if (can_activate_sso)
		{
			char* tmp_ptr = ls.m_ptr;

			uint32_t tmp_len = ls.m_codeunits;

			for (uint32_t i = 0; i != tmp_len + 1; ++i)
				ss.m_internal_buf[i] = tmp_ptr[i];

			ss.m_internal_cap = static_cast<uint8_t>(internal_buf_max - tmp_len);

			free(tmp_ptr);

			return true;
		}

		return can_activate_sso;
	}

	bool utf8_string::change_heap_cap(uint32_t cunits)
	{
		uint32_t alloc_size = least_bigger_cap(cunits);

		char* tmp_ptr = (char*)realloc(ls.m_ptr, alloc_size * sizeof(char));

		if (!tmp_ptr)
			return false;		//Reallocation failed...

		ls.m_ptr = tmp_ptr;
		ls.m_capacity = alloc_size;

		return true;
	}

	uint32_t utf8_string::least_bigger_cap(uint32_t required)
	{
		if (required-- <= min_heap_cap)
			return min_heap_cap - 1;

		required |= required >> 1;
		required |= required >> 2;
		required |= required >> 4;
		required |= required >> 8;
		required |= required >> 16;

		return ++required;
	}

	void utf8_string::set_codeunits(uint32_t cunits)
	{
		is_sso() ? ss.m_internal_cap = static_cast<uint8_t>(internal_buf_max - cunits) : ls.m_codeunits = cunits;
	}

	void utf8_string::set_codepoints(uint32_t cpoints)
	{
		if (!is_sso()) ls.m_codepoints = cpoints;
	}
}
