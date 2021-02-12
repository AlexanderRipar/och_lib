#include "utf16.h"

#include <cstdint>
#include <cstdlib>

__forceinline bool _is_surr_pair(const char16_t hi, const char16_t lo) { return hi >= 0xD800 && hi <= 0xDBFF && lo >= 0xDC00 && lo <= 0xDFFF; }

__forceinline char32_t _utf16_to_codepoint(const char16_t hi, const char16_t lo) { return _is_surr_pair(hi, lo) ? ((static_cast<char32_t>(hi - 0xD800) << 10) | (lo - 0xDC00)) + 0x10000 : static_cast<char32_t>(hi); }

uint32_t _utf16_from_codepoint(char16_t* out, char32_t cpoint)
{
	if (cpoint <= 0xFFFF)
	{
		out[0] = static_cast<char16_t>(cpoint);

		return 1;
	}

	cpoint -= 0x10000;

	out[0] = static_cast<char16_t>(0xD800 + (cpoint >> 10));

	out[1] = static_cast<char16_t>(0xDC00 + (cpoint & 0x3FF));

	return 2;
}

void _utf16_len(const char16_t* cstring, uint32_t& cunits, uint32_t& cpoints)
{
	while (cstring[cunits])
	{
		cunits += _is_surr_pair(cstring[cunits], cstring[cunits + 1]) ? 2 : 1;

		++cpoints;
	}
}

/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/*////////////////////////////////////////////////////utf16_iterator/////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

utf16_iterator::utf16_iterator(const char16_t* cstring) : cstring{ cstring } {}

char32_t utf16_iterator::operator*()
{
	return _utf16_to_codepoint(cstring[0], cstring[1]);
}

void utf16_iterator::operator++()
{
	cstring += _is_surr_pair(cstring[0], cstring[1]) ? 2 : 1;
}

bool utf16_iterator::operator!=(const utf16_iterator& rhs)
{
	return cstring < rhs.cstring;
}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/*//////////////////////////////////////////////////////utf16_view///////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

utf16_view::utf16_view() : m_ptr{ &empty_str }, m_codeunits{ 0 }, m_codepoints{ 0 } {}

utf16_view::utf16_view(const char16_t* cstring) : m_ptr{ cstring } { uint32_t cunits = 0, cpoints = 0; _utf16_len(cstring, cunits, cpoints); m_codeunits = cunits; m_codepoints = cpoints; }

utf16_view::utf16_view(const char16_t* str, uint32_t cunits, uint32_t cpoints) : m_ptr{ str }, m_codeunits{ cunits }, m_codepoints{ cpoints } {}

utf16_view utf16_view::subview(uint32_t pos, uint32_t len)
	{
		uint32_t first_cunit = 0;
		uint32_t first_cpoint = 0;

		while (m_ptr[first_cunit] && first_cpoint != pos)
		{
			first_cunit += _is_surr_pair(m_ptr[first_cunit], m_ptr[first_cunit + 1]) ? 2 : 1;

			++first_cpoint;
		}

		uint32_t last_cunit = first_cunit;
		uint32_t last_cpoint = 0;

		while (m_ptr[last_cunit] && last_cpoint != len)
		{
			last_cunit += _is_surr_pair(m_ptr[last_cunit], m_ptr[last_cunit + 1]) ? 2 : 1;

			++last_cpoint;
		}

		return utf16_view(m_ptr + first_cunit, last_cunit - first_cunit, last_cpoint);
	}



/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
/*/////////////////////////////////////////////////////utf16_string//////////////////////////////////////////////////////////*/
/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

////////////////////////////////////////////////////Constructor////////////////////////////////////////////////////////////////

utf16_string::utf16_string() : ss{ {0}, internal_buf_max } {}

utf16_string::utf16_string(const utf16_view& view)
{
	view.m_codeunits <= internal_buf_max ? construct_ss(view.m_ptr, view.m_codeunits) : construct_ls(view.m_ptr, view.m_codeunits, view.m_codepoints);
}

utf16_string::utf16_string(const utf16_string& str)
{
	str.get_codeunits() <= internal_buf_max ? construct_ss(str.raw_cbegin(), str.get_codeunits()) : construct_ls(str.raw_cbegin(), str.get_codeunits(), str.get_codepoints());
}

utf16_string::utf16_string(const char16_t* cstring)
{
	uint32_t words = 0, codepts = 0;

	_utf16_len(cstring, words, codepts);

	words <= internal_buf_max ? construct_ss(cstring, words) : construct_ls(cstring, words, codepts);
}

utf16_string::~utf16_string()
{
	if (!is_sso())
		free(ls.m_ptr);
}

/////////////////////////////////////////////////////Iterators/////////////////////////////////////////////////////////////////

char16_t* utf16_string::raw_begin()
{
	return is_sso() ? ss.m_internal_buf : ls.m_ptr;
}

const char16_t* utf16_string::raw_cbegin() const
{
	return is_sso() ? ss.m_internal_buf : ls.m_ptr;
}

char16_t* utf16_string::raw_end()
{
	return is_sso() ? ss.m_internal_buf + internal_buf_max - ss.m_internal_cap : ls.m_ptr + ls.m_codeunits;
}

const char16_t* utf16_string::raw_cend() const
{
	return is_sso() ? ss.m_internal_buf + internal_buf_max - ss.m_internal_cap : ls.m_ptr + ls.m_codeunits;
}

utf16_iterator utf16_string::begin() const
{
	return utf16_iterator(raw_cbegin());
}

utf16_iterator utf16_string::end() const
{
	return utf16_iterator(raw_cend());
}

/////////////////////////////////////////////////////Size//////////////////////////////////////////////////////////////////////

uint32_t utf16_string::get_codeunits() const
{
	return is_sso() ? internal_buf_max - ss.m_internal_cap : ls.m_codeunits;
}

uint32_t utf16_string::get_codepoints() const
{
	return is_sso() ? [](const char16_t* cstring) {uint32_t cunits = 0, cpoints = 0; while(cstring[cunits]){ cunits += _is_surr_pair(cstring[cunits], cstring[cunits + 1]) ? 2 : 1; ++cpoints; } return cpoints; }(ss.m_internal_buf) : ls.m_codepoints;
}

uint32_t utf16_string::get_capacity() const
{
	return is_sso() ? internal_buf_max : ls.m_capacity;
}

uint32_t utf16_string::shrink_to_fit()
{
	if (is_sso() || try_activate_sso())
		return internal_buf_max;

	change_heap_cap(get_codeunits());

	return ls.m_capacity;
}

bool utf16_string::reserve(uint32_t n)
{
	if (n <= get_capacity())
		return true;				//min_capacity already exceeded; Nothing to do

	if (is_sso())
		return deactivate_sso(n);	//min_capacity not reached and sso active

	return change_heap_cap(n);		//min_capacity not reached and sso inactive
}

bool utf16_string::resize(uint32_t n)
{
	if (get_capacity() == least_bigger_cap(n) || (is_sso() && n <= internal_buf_max))
		return true;				//If capacity is already matched, or sso is active and a fitting capacity is requested, return true

	if (is_sso())
		return deactivate_sso(n);	//Requested capacity does not fit internal buffer; Deactivate sso

	return change_heap_cap(n);		//sso already inactive and size actually changed; Change capacity
}

bool utf16_string::empty() const
{
	return !get_codeunits();
}

bool utf16_string::is_sso() const
{
	return ss.m_internal_cap != long_string_id;
}

////////////////////////////////////////////////Element Acess//////////////////////////////////////////////////////////////////

char32_t utf16_string::at(uint32_t pos) const
{
	if (!is_sso() && get_codepoints() == get_codeunits())
		return raw_cbegin()[pos];

	utf16_iterator it(raw_cbegin());

	for (uint32_t i = 0; i != pos; ++i)
		++it;

	return *it;
}

char32_t utf16_string::front() const
{
	return _utf16_to_codepoint(raw_cbegin()[0], raw_cbegin()[1]);
}

char32_t utf16_string::back() const
{
	const char16_t* end = raw_cend() - 1;

	if (empty())
		return U'\0';

	if (get_codeunits() >= 2 && _is_surr_pair(end[-1], end[0]))
		return _utf16_to_codepoint(end[-1], end[0]);

	return static_cast<char32_t>(end[0]);

}

/////////////////////////////////////////////////Modifiers/////////////////////////////////////////////////////////////////////

void utf16_string::operator=(const utf16_view& view) noexcept
{
	set_to(view.m_ptr, view.m_codeunits, view.m_codepoints);
}

void utf16_string::operator=(const utf16_string& string) noexcept
{
	set_to(string.raw_cbegin(), string.get_codeunits(), string.get_codepoints());
}

void utf16_string::operator=(utf16_string&& string) noexcept
{
	if (!is_sso())
		free(ls.m_ptr);

	ls = string.ls;

	string.ls.m_ptr = nullptr;//To avoid freeing in dtor
}

void utf16_string::operator=(const char16_t* cstring)
{
	uint32_t cunits = 0, cpoints = 0;

	_utf16_len(cstring, cunits, cpoints);

	set_to(cstring, cunits, cpoints);
}

void utf16_string::operator+=(char32_t cpoint)
{
	char16_t buf[2];

	append(buf, _utf16_from_codepoint(buf, cpoint), 1);
}

void utf16_string::operator+=(const utf16_view& view)
{
	append(view.m_ptr, view.m_codeunits, view.m_codepoints);
}

void utf16_string::operator+=(const utf16_string& str)
{
	append(str.raw_cbegin(), str.get_codeunits(), str.get_codepoints());
}

void utf16_string::operator+=(const char16_t* cstring)
{
	uint32_t cunits = 0, cpoints = 0;

	_utf16_len(cstring, cunits, cpoints);

	append(cstring, cunits, cpoints);
}

uint32_t utf16_string::pop(uint32_t n)
{
	uint32_t curr_cunit = get_codeunits();

	uint32_t codepts_popped = 0;

	while (curr_cunit > 0 && codepts_popped < n)
	{
		curr_cunit -= curr_cunit > 1 && _is_surr_pair(raw_cbegin()[curr_cunit - 2], raw_cbegin()[curr_cunit - 1]) ? 2 : 1;

		++codepts_popped;
	}

	raw_begin()[curr_cunit] = u'\0';

	set_codeunits(curr_cunit);

	set_codepoints(get_codepoints() - codepts_popped);

	return codepts_popped;
}

void utf16_string::clear()
{
	if (!is_sso())
		free(ls.m_ptr);

	ss.m_internal_buf[0] = u'\0';

	ss.m_internal_cap = internal_buf_max;
}

void utf16_string::erase(uint32_t pos, uint32_t len)
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

bool utf16_string::insert(uint32_t pos, char32_t cpoint)
{
	char16_t buf[2];

	return insert(buf, _utf16_from_codepoint(buf, cpoint), 1, pos);
}

bool utf16_string::insert(uint32_t pos, const utf16_view& view)
{
	return insert(view.m_ptr, view.m_codeunits, view.m_codepoints, pos);
}

bool utf16_string::insert(uint32_t pos, const char16_t* cstring)
{
	uint32_t added_cunits = 0, added_cpoints = 0;

	_utf16_len(cstring, added_cunits, added_cpoints);

	return insert(cstring, added_cunits, added_cpoints, pos);
}

bool utf16_string::replace(uint32_t pos, char32_t cpoint)
{
	char16_t buf[2];

	return replace(buf, _utf16_from_codepoint(buf, cpoint), 1, pos);
}

bool utf16_string::replace(uint32_t pos, const utf16_view& view)
{
	return replace(view.m_ptr, view.m_codeunits, view.m_codepoints, pos);
}

bool utf16_string::replace(uint32_t pos, const char16_t* cstring)
{
	uint32_t added_cunits = 0, added_cpoints = 0;

	_utf16_len(cstring, added_cunits, added_cpoints);

	return replace(cstring, added_cunits, added_cpoints, pos);
}

bool utf16_string::operator==(const utf16_string& rhs) const
{
	if (get_codeunits() != rhs.get_codeunits() || get_codepoints() != rhs.get_codepoints())
		return false;

	for (uint32_t i = 0; i != get_codeunits(); ++i)
		if (raw_cbegin()[i] != rhs.raw_cbegin()[i])
			return false;

	return true;
}

bool utf16_string::operator!=(const utf16_string& rhs) const
{
	return !operator==(rhs);
}

bool utf16_string::operator>(const utf16_string& rhs) const
{
	uint32_t min_cps = get_codepoints() < rhs.get_codepoints() ? get_codepoints() : rhs.get_codepoints();

	utf16_iterator l_it = begin();
	utf16_iterator r_it = rhs.begin();

	for (uint32_t i = 0; i != min_cps; ++i)
	{
		char32_t l_c = *l_it;
		char32_t r_c = *r_it;

		if (l_c != r_c)
			return l_c > r_c;

		++l_it;
		++r_it;
	}

	return get_codepoints() > rhs.get_codepoints();
}

bool utf16_string::operator>=(const utf16_string& rhs) const
{
	return !operator<(rhs);
}

bool utf16_string::operator<(const utf16_string& rhs) const
{
	uint32_t min_cps = get_codepoints() < rhs.get_codepoints() ? get_codepoints() : rhs.get_codepoints();

	utf16_iterator l_it = begin();
	utf16_iterator r_it = rhs.begin();

	for (uint32_t i = 0; i != min_cps; ++i)
	{
		char32_t l_c = *l_it;
		char32_t r_c = *r_it;

		if (l_c != r_c)
			return l_c < r_c;

		++l_it;
		++r_it;
	}

	return get_codepoints() < rhs.get_codepoints();
}

bool utf16_string::operator<=(const utf16_string& rhs) const
{
	return !operator>(rhs);
}

//////////////////////////////////////////////////Operations///////////////////////////////////////////////////////////////////

utf16_string utf16_string::substr(uint32_t pos, uint32_t len) const
{
	uint32_t prev_cpoints = get_codepoints();

	if (pos >= prev_cpoints)
		return utf16_string();

	uint32_t first_cunit = 0, first_cpoint = 0;

	step(first_cunit, first_cpoint, pos);

	uint32_t last_cunit = first_cunit, last_cpoint = first_cpoint;

	if (len >= prev_cpoints - pos)
		return utf16_string(raw_cbegin() + first_cunit, get_codeunits() - first_cunit, prev_cpoints - pos);

	step(last_cunit, last_cpoint, pos + len);

	return utf16_string(raw_cbegin() + first_cunit, last_cunit - first_cunit, len);
}

///////////////////////////////////////////////////Private/////////////////////////////////////////////////////////////////////

utf16_string::utf16_string(const char16_t* cstring, uint32_t cunits, uint32_t cpoints)
{
	cunits <= internal_buf_max ? construct_ss(cstring, cunits) : construct_ls(cstring, cunits, cpoints);
}

void utf16_string::construct_ss(const char16_t* cstring, uint32_t cunits)
{
	for (uint32_t i = 0; i != cunits; ++i)
		ss.m_internal_buf[i] = cstring[i];

	ss.m_internal_buf[cunits] = u'\0';

	ss.m_internal_cap = static_cast<uint16_t>(internal_buf_max - cunits);
}

void utf16_string::construct_ls(const char16_t* cstring, uint32_t cunits, uint32_t cpoints)
{
	ls.m_capacity = least_bigger_cap(cunits);

	ls.m_ptr = (char16_t*)malloc(ls.m_capacity * sizeof(char16_t));

	if (!ls.m_ptr)
	{
		ss.m_internal_cap = internal_buf_max;

		return;
	}

	for (uint32_t i = 0; i != cunits; ++i)
		ls.m_ptr[i] = cstring[i];

	ls.m_ptr[cunits] = u'\0';

	ls.m_codeunits = cunits;

	ls.m_codepoints = cpoints;

	ss.m_internal_cap = long_string_id;
}

bool utf16_string::set_to(const char16_t* cstring, uint32_t cunits, uint32_t cpoints)
{
	if (!resize(cunits))
		return false;

	for (uint32_t i = 0; i != cunits; ++i)
		raw_begin()[i] = cstring[i];

	raw_begin()[cunits] = u'\0';

	set_codeunits(cunits);

	set_codepoints(cpoints);

	return true;
}

bool utf16_string::append(const char16_t* cstring, uint32_t added_cunits, uint32_t added_cpoints)
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

bool utf16_string::insert(const char16_t* cstring, uint32_t added_cunits, uint32_t added_cpoints, uint32_t pos)
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

bool utf16_string::replace(const char16_t* cstring, uint32_t added_cunits, uint32_t added_cpoints, uint32_t pos)
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

void utf16_string::step(uint32_t& cunit, uint32_t& cpoint, uint32_t last_cpoint) const
{
	while (cpoint < last_cpoint)
	{
		cunit += _is_surr_pair(raw_cbegin()[cunit], raw_cbegin()[cunit + 1]) ? 2 : 1;

		++cpoint;
	}
}

bool utf16_string::deactivate_sso(uint32_t cunits)
{
	uint32_t alloc_size = least_bigger_cap(cunits);

	char16_t* tmp_ptr = (char16_t*)malloc(alloc_size * sizeof(char16_t));

	if (!tmp_ptr)			//Allocation failed...
		return false;

	for (uint32_t i = 0; i != internal_buf_max - ss.m_internal_cap + 1; ++i)
		tmp_ptr[i] = ss.m_internal_buf[i];

	ls.m_ptr = tmp_ptr;
	ls.m_codeunits = internal_buf_max - ss.m_internal_cap;
	ls.m_capacity = alloc_size;
	ls.m_codepoints = [](const char16_t* cstring) {uint32_t cunits = 0, cpoints = 0; while (cstring[cunits]) { cunits += _is_surr_pair(cstring[cunits], cstring[cunits + 1]) ? 2 : 1; ++cpoints; } return cpoints; }(tmp_ptr);
	ss.m_internal_cap = long_string_id;

	return true;
}

bool utf16_string::try_activate_sso()
{
	bool can_activate_sso = !is_sso() && get_capacity() <= internal_buf_max;

	if (can_activate_sso)
	{
		char16_t* tmp_ptr = ls.m_ptr;

		uint32_t tmp_len = ls.m_codeunits;

		for (uint32_t i = 0; i != tmp_len + 1; ++i)
			ss.m_internal_buf[i] = tmp_ptr[i];

		ss.m_internal_cap = static_cast<uint16_t>(internal_buf_max - tmp_len);

		free(tmp_ptr);

		return true;
	}

	return can_activate_sso;
}

bool utf16_string::change_heap_cap(uint32_t cunits)
{
	uint32_t alloc_size = least_bigger_cap(cunits);

	char16_t* tmp_ptr = (char16_t*)realloc(ls.m_ptr, alloc_size * sizeof(char16_t));

	if (!tmp_ptr)
		return false;		//Reallocation failed...

	ls.m_ptr = tmp_ptr;
	ls.m_capacity = alloc_size;

	return true;
}

uint32_t utf16_string::least_bigger_cap(uint32_t required)
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

void utf16_string::set_codeunits(uint32_t cunits)
{
	is_sso() ? ss.m_internal_cap = static_cast<uint16_t>(internal_buf_max - cunits) : ls.m_codeunits = cunits;
}

void utf16_string::set_codepoints(uint32_t cpoints)
{
	if (!is_sso()) ls.m_codepoints = cpoints;
}
