#include "och_utf8.h"

#include <cstdint>
#include <cstdlib>

namespace och
{
	void _utf8_len(const char* str, uint32_t& cunits, uint32_t& cpoints, uint32_t max_cpoints) noexcept
	{
		while (cpoints < max_cpoints && str[cunits])
		{
			++cunits;

			cpoints += !is_utf8_surr(str[cunits]);
		}
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////////utf8_iterator/////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	utf8_iterator::utf8_iterator(const char* cstring) noexcept : cstring{ cstring } {}

	utf8_char utf8_iterator::operator*() const noexcept
	{
		return utf8_char(cstring);
	}

	void utf8_iterator::operator++() noexcept
	{
		while (is_utf8_surr(*++cstring));
	}

	bool utf8_iterator::operator!=(const utf8_iterator& rhs) const noexcept
	{
		return cstring < rhs.cstring;
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*//////////////////////////////////////////////////////utf8_view////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	utf8_view::utf8_view(const utf8_string& string) noexcept : m_ptr{ string.raw_cbegin() }, m_codeunits{ string.get_codeunits() }, m_codepoints{ string.get_codepoints() } {}

	utf8_view utf8_view::subview(uint32_t pos, uint32_t len) const noexcept
	{
		uint32_t prev_cpoints = get_codepoints();

		if (pos >= prev_cpoints)
			return utf8_string();

		uint32_t first_cunit = 0, first_cpoint = 0;

		_utf8_len(raw_cbegin(), first_cunit, first_cpoint, pos);

		uint32_t last_cunit = first_cunit, last_cpoint = first_cpoint;

		if (len >= prev_cpoints - pos)
			return utf8_view(raw_cbegin() + first_cunit, get_codeunits() - first_cunit, prev_cpoints - pos);

		_utf8_len(raw_cbegin(), last_cunit, last_cpoint, pos + len);

		return utf8_view(raw_cbegin() + first_cunit, last_cunit - first_cunit, len);
	}

	uint32_t utf8_view::get_codepoints() const noexcept
	{
		return m_codepoints;
	}

	uint32_t utf8_view::get_codeunits() const noexcept
	{
		return m_codeunits;
	}

	const char* utf8_view::raw_cbegin() const noexcept
	{
		return m_ptr;
	}

	const char* utf8_view::raw_cend() const noexcept
	{
		return m_ptr + m_codeunits;
	}

	utf8_iterator utf8_view::begin() const noexcept
	{
		return utf8_iterator(m_ptr);
	}

	utf8_iterator utf8_view::end() const noexcept
	{
		return utf8_iterator(m_ptr + m_codeunits);
	}

	utf8_char utf8_view::at(uint32_t pos) const noexcept
	{
		utf8_iterator it = begin();

		for (uint32_t i = 0; i != pos; ++i)
			++it;

		return *it;
	}

	bool utf8_view::operator==(const utf8_view& rhs) const noexcept
	{
		if (get_codeunits() != rhs.get_codeunits() || get_codepoints() != rhs.get_codepoints())
			return false;

		for (uint32_t i = 0; i != get_codeunits(); ++i)
			if (raw_cbegin()[i] != rhs.raw_cbegin()[i])
				return false;

		return true;
	}

	bool utf8_view::operator!=(const utf8_view& rhs) const noexcept
	{
		return !operator==(rhs);
	}

	bool utf8_view::operator>(const utf8_view& rhs) const noexcept
	{
		uint32_t min_cps = get_codepoints() < rhs.get_codepoints() ? get_codepoints() : rhs.get_codepoints();

		const char* l_ptr = raw_cbegin();
		const char* r_ptr = rhs.raw_cbegin();

		for (uint32_t i = 0; i != min_cps; ++i)
			if (l_ptr[i] != r_ptr[i])
				return l_ptr[i] > r_ptr[i];

		return get_codepoints() > rhs.get_codepoints();
	}

	bool utf8_view::operator>=(const utf8_view& rhs) const noexcept
	{
		return !operator<(rhs);
	}

	bool utf8_view::operator<(const utf8_view& rhs) const noexcept
	{
		uint32_t min_cps = get_codepoints() < rhs.get_codepoints() ? get_codepoints() : rhs.get_codepoints();

		const char* l_ptr = raw_cbegin();
		const char* r_ptr = rhs.raw_cbegin();

		for (uint32_t i = 0; i != min_cps; ++i)
			if (l_ptr[i] != r_ptr[i])
				return l_ptr[i] < r_ptr[i];

		return get_codepoints() < rhs.get_codepoints();
	}

	bool utf8_view::operator<=(const utf8_view& rhs) const noexcept
	{
		return !operator>(rhs);
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////////utf8_string///////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	//////////////////////////////////////////////////Constructor//////////////////////////////////////////////////////////////

	utf8_string::utf8_string() noexcept : ss{ { '\0' }, MAX_SSO_CUNITS } {}

	utf8_string::utf8_string(const utf8_view& view) noexcept
	{
		const uint32_t cunits = view.get_codeunits();

		if (cunits <= MAX_SSO_CUNITS)
			construct_ss(view.raw_cbegin(), cunits);
		else
			construct_ls(view.raw_cbegin(), cunits, view.get_codepoints());
	}

	utf8_string::utf8_string(const utf8_string& str) noexcept
	{
		const uint32_t cunits = str.get_codeunits();

		if (cunits <= MAX_SSO_CUNITS)
			construct_ss(str.raw_cbegin(), cunits);
		else
			construct_ls(str.raw_cbegin(), cunits, str.get_codepoints());
	}

	utf8_string::utf8_string(const char* cstring) noexcept
	{
		uint32_t cunits = 0, cpoints = 0;

		_utf8_len(cstring, cunits, cpoints);

		if (cunits <= MAX_SSO_CUNITS)
			construct_ss(cstring, cunits);
		else
			construct_ls(cstring, cunits, cpoints);
	}

	utf8_string::~utf8_string() noexcept
	{
		if (!is_sso())
			free(ls.m_ptr);
	}

	///////////////////////////////////////////////////Iterators///////////////////////////////////////////////////////////////

	char* utf8_string::raw_begin() noexcept
	{
		if (is_sso())
			return ss.m_internal_buf;
		else
			return ls.m_ptr;
	}

	const char* utf8_string::raw_cbegin() const noexcept
	{
		if (is_sso())
			return ss.m_internal_buf;
		else
			return ls.m_ptr;
	}

	char* utf8_string::raw_end() noexcept
	{
		if (is_sso())
			return ss.m_internal_buf + MAX_SSO_CUNITS - ss.m_internal_cap;
		else
			return ls.m_ptr + ls.m_codeunits;
	}

	const char* utf8_string::raw_cend() const noexcept
	{
		if (is_sso())
			return ss.m_internal_buf + MAX_SSO_CUNITS - ss.m_internal_cap;
		else
			return ls.m_ptr + ls.m_codeunits;
	}

	utf8_iterator utf8_string::begin() const noexcept
	{
		return utf8_iterator(raw_cbegin());
	}

	utf8_iterator utf8_string::end() const noexcept
	{
		return utf8_iterator(raw_cend());
	}

	///////////////////////////////////////////////////Size////////////////////////////////////////////////////////////////////

	uint32_t utf8_string::get_codeunits() const noexcept
	{
		if (is_sso())
			return MAX_SSO_CUNITS - ss.m_internal_cap;
		else
			return ls.m_codeunits;
	}

	uint32_t utf8_string::get_codepoints() const noexcept
	{
		if (is_sso())
		{
			uint32_t cpoints = 0;

			for (const char* str = ss.m_internal_buf; *str != '\0'; ++str)
				cpoints += (*str++ & 0xC0) != 0x80;

			return cpoints;
		}
		else
		{
			return ls.m_codepoints;
		}
	}

	uint32_t utf8_string::get_capacity() const noexcept
	{
		if (is_sso())
			return MAX_SSO_CUNITS;
		else
			return ls.m_capacity;
	}

	uint32_t utf8_string::shrink_to_fit() noexcept
	{
		if (is_sso() || try_activate_sso())
			return MAX_SSO_CUNITS;

		change_heap_cap(get_codeunits() + 1);

		return ls.m_capacity;
	}

	bool utf8_string::reserve(uint32_t cunits) noexcept
	{
		if (cunits < get_capacity())
			return true;						//min_capacity already exceeded; Nothing to do

		if (is_sso())
			return deactivate_sso(cunits + 1);	//min_capacity not reached and SSO active

		return change_heap_cap(cunits + 1);		//min_capacity not reached and SSO inactive
	}

	bool utf8_string::resize(uint32_t cunits) noexcept
	{
		if (get_capacity() == least_bigger_capacity(cunits + 1))
			return true;							//If capacity is already matched, or sso is active and a fitting capacity is requested, return true

		if (is_sso())
		{
			if (cunits <= MAX_SSO_CUNITS)
			{
				ss.m_internal_cap = static_cast<uint8_t>(MAX_SSO_CUNITS - cunits);

				ss.m_internal_buf[cunits] = '\0';

				return true;
			}

			if (!deactivate_sso(cunits + 1))		//Requested capacity does not fit internal buffer; Deactivate sso
				return false;
		}
		else
		{
			if (!change_heap_cap(cunits + 1))			//sso already inactive and size actually changed; Change capacity
				return false;
		}

		ls.m_ptr[cunits] = '\0';

		return true;
	}

	bool utf8_string::empty() const noexcept
	{
		return !get_codeunits();
	}

	bool utf8_string::is_sso() const noexcept
	{
		return ss.m_internal_cap != LONG_STRING_MARKER;
	}

	//////////////////////////////////////////////Element Acess////////////////////////////////////////////////////////////////

	utf8_char utf8_string::at(uint32_t pos) const noexcept
	{
		if (!is_sso() && get_codepoints() == get_codeunits())
			return raw_cbegin()[pos];

		utf8_iterator it(raw_cbegin());

		for (uint32_t i = 0; i != pos; ++i)
			++it;

		return *it;
	}

	utf8_char utf8_string::front() const noexcept
	{
		return _utf8_to_utf32(raw_cbegin());
	}

	utf8_char utf8_string::back() const noexcept
	{
		const char* end = raw_cend() - 1;

		while ((*end & 0xC0) == 0x80)
			--end;

		return _utf8_to_utf32(end);

	}

	///////////////////////////////////////////////Modifiers///////////////////////////////////////////////////////////////////

	void utf8_string::operator=(const utf8_view& view) noexcept
	{
		set_to(view.raw_cbegin(), view.get_codeunits(), view.get_codepoints());
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

	void utf8_string::operator=(const char* cstring) noexcept
	{
		uint32_t cunits = 0, cpoints = 0;

		_utf8_len(cstring, cunits, cpoints);

		set_to(cstring, cunits, cpoints);
	}

	void utf8_string::operator+=(utf8_char cpoint) noexcept
	{
		append(cpoint.cbegin(), cpoint.get_codeunits(), 1);
	}

	void utf8_string::operator+=(const utf8_view& view) noexcept
	{
		append(view.raw_cbegin(), view.get_codeunits(), view.get_codepoints());
	}

	void utf8_string::operator+=(const utf8_string& str) noexcept
	{
		append(str.raw_cbegin(), str.get_codeunits(), str.get_codepoints());
	}

	void utf8_string::operator+=(const char* cstring) noexcept
	{
		uint32_t cunits = 0, cpoints = 0;

		_utf8_len(cstring, cunits, cpoints);

		append(cstring, cunits, cpoints);
	}

	uint32_t utf8_string::pop(uint32_t n) noexcept
	{
		uint32_t curr_cunit = get_codeunits();

		uint32_t codepts_popped = 0;

		while (curr_cunit > 0 && codepts_popped < n)
		{
			codepts_popped += !is_utf8_surr(raw_cbegin()[--curr_cunit]);
		}

		raw_begin()[curr_cunit] = '\0';

		set_codeunits(curr_cunit);

		set_codepoints(get_codepoints() - codepts_popped);

		return codepts_popped;
	}

	void utf8_string::clear() noexcept
	{
		if (!is_sso())
			free(ls.m_ptr);

		ss.m_internal_buf[0] = '\0';

		ss.m_internal_cap = MAX_SSO_CUNITS;
	}

	void utf8_string::erase(uint32_t pos, uint32_t len) noexcept
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

	bool utf8_string::insert(uint32_t pos, utf8_char cpoint) noexcept
	{
		return insert(cpoint.cbegin(), cpoint.get_codeunits(), 1, pos);
	}

	bool utf8_string::insert(uint32_t pos, const utf8_view& view) noexcept
	{
		return insert(view.raw_cbegin(), view.get_codeunits(), view.get_codepoints(), pos);
	}

	bool utf8_string::insert(uint32_t pos, const char* cstring) noexcept
	{
		uint32_t added_cunits = 0, added_cpoints = 0;

		_utf8_len(cstring, added_cunits, added_cpoints);

		return insert(cstring, added_cunits, added_cpoints, pos);
	}

	bool utf8_string::replace(uint32_t pos, utf8_char cpoint) noexcept
	{
		return replace(cpoint.cbegin(), cpoint.get_codeunits(), 1, pos);
	}

	bool utf8_string::replace(uint32_t pos, const utf8_view& view) noexcept
	{
		return replace(view.raw_cbegin(), view.get_codeunits(), view.get_codepoints(), pos);
	}

	bool utf8_string::replace(uint32_t pos, const char* cstring) noexcept
	{
		uint32_t added_cunits = 0, added_cpoints = 0;

		_utf8_len(cstring, added_cunits, added_cpoints);

		return replace(cstring, added_cunits, added_cpoints, pos);
	}

	bool utf8_string::operator==(const utf8_string& rhs) const noexcept
	{
		if (get_codeunits() != rhs.get_codeunits() || get_codepoints() != rhs.get_codepoints())
			return false;

		for (uint32_t i = 0; i != get_codeunits(); ++i)
			if (raw_cbegin()[i] != rhs.raw_cbegin()[i])
				return false;

		return true;
	}

	bool utf8_string::operator!=(const utf8_string& rhs) const noexcept
	{
		return !operator==(rhs);
	}

	bool utf8_string::operator>(const utf8_string& rhs) const noexcept
	{
		uint32_t min_cps = get_codepoints() < rhs.get_codepoints() ? get_codepoints() : rhs.get_codepoints();

		const char* l_ptr = raw_cbegin();
		const char* r_ptr = rhs.raw_cbegin();

		for (uint32_t i = 0; i != min_cps; ++i)
			if (l_ptr[i] != r_ptr[i])
				return l_ptr[i] > r_ptr[i];

		return get_codepoints() > rhs.get_codepoints();
	}

	bool utf8_string::operator>=(const utf8_string& rhs) const noexcept
	{
		return !operator<(rhs);
	}

	bool utf8_string::operator<(const utf8_string& rhs) const noexcept
	{
		uint32_t min_cps = get_codepoints() < rhs.get_codepoints() ? get_codepoints() : rhs.get_codepoints();

		const char* l_ptr = raw_cbegin();
		const char* r_ptr = rhs.raw_cbegin();

		for (uint32_t i = 0; i != min_cps; ++i)
			if (l_ptr[i] != r_ptr[i])
				return l_ptr[i] < r_ptr[i];

		return get_codepoints() < rhs.get_codepoints();
	}

	bool utf8_string::operator<=(const utf8_string& rhs) const noexcept
	{
		return !operator>(rhs);
	}

	////////////////////////////////////////////////Operations/////////////////////////////////////////////////////////////////

	utf8_string utf8_string::substr(uint32_t pos, uint32_t len) const noexcept
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

	void utf8_string::fmt_prepare_for_raw_write(uint32_t n)
	{
		set_codeunits(get_codeunits() + n);
		set_codepoints(get_codepoints() + n);
	}

	/////////////////////////////////////////////////Private///////////////////////////////////////////////////////////////////

	utf8_string::utf8_string(const char* cstring, uint32_t cunits, uint32_t cpoints) noexcept
	{
		if (cunits <= MAX_SSO_CUNITS)
			construct_ss(cstring, cunits);
		else
			construct_ls(cstring, cunits, cpoints);
	}

	void utf8_string::construct_ss(const char* cstring, uint32_t cunits) noexcept
	{
		for (uint32_t i = 0; i != cunits; ++i)
			ss.m_internal_buf[i] = cstring[i];

		ss.m_internal_buf[cunits] = '\0';

		ss.m_internal_cap = static_cast<uint8_t>(MAX_SSO_CUNITS - cunits);
	}

	void utf8_string::construct_ls(const char* cstring, uint32_t cunits, uint32_t cpoints) noexcept
	{
		ls.m_capacity = least_bigger_capacity(cunits + 1);

		ls.m_ptr = static_cast<char*>(malloc(ls.m_capacity * sizeof(char)));

		if (!ls.m_ptr)
		{
			ss.m_internal_cap = MAX_SSO_CUNITS;

			return;
		}

		for (uint32_t i = 0; i != cunits; ++i)
			ls.m_ptr[i] = cstring[i];

		ls.m_ptr[cunits] = '\0';

		ls.m_codeunits = cunits;

		ls.m_codepoints = cpoints;

		ss.m_internal_cap = LONG_STRING_MARKER;
	}

	bool utf8_string::set_to(const char* cstring, uint32_t cunits, uint32_t cpoints) noexcept
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

	bool utf8_string::append(const char* cstring, uint32_t added_cunits, uint32_t added_cpoints) noexcept
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

	bool utf8_string::insert(const char* cstring, uint32_t added_cunits, uint32_t added_cpoints, uint32_t pos) noexcept
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

	bool utf8_string::replace(const char* cstring, uint32_t added_cunits, uint32_t added_cpoints, uint32_t pos) noexcept
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

	void utf8_string::step(uint32_t& cunit, uint32_t& cpoint, uint32_t last_cpoint) const noexcept
	{
		while (cpoint < last_cpoint)
		{
			cunit += _utf8_codepoint_bytes(raw_cbegin()[cunit]);

			++cpoint;
		}
	}

	bool utf8_string::deactivate_sso(uint32_t bytes) noexcept
	{
		uint32_t alloc_size = least_bigger_capacity(bytes);

		char* tmp_ptr = static_cast<char*>(malloc(alloc_size * sizeof(char)));

		if (!tmp_ptr)			//Allocation failed...
			return false;

		const uint32_t curr_cunits = MAX_SSO_CUNITS - ss.m_internal_cap;

		uint32_t curr_cpoints = 0;

		for (const char* str = ss.m_internal_buf; *str != '\0'; ++str)
			curr_cpoints += (*str++ & 0xC0) != 0x80;

		for (uint32_t i = 0; i != curr_cunits + 1; ++i)
			tmp_ptr[i] = ss.m_internal_buf[i];

		ls.m_ptr = tmp_ptr;
		ls.m_codeunits = curr_cunits;
		ls.m_codepoints = curr_cpoints;
		ls.m_capacity = alloc_size;
		ss.m_internal_cap = LONG_STRING_MARKER;

		return true;
	}

	bool utf8_string::try_activate_sso() noexcept
	{
		if (!is_sso() && get_codeunits() <= MAX_SSO_CUNITS)
		{
			char* tmp_ptr = ls.m_ptr;

			uint32_t tmp_len = ls.m_codeunits;

			for (uint32_t i = 0; i != tmp_len + 1; ++i)
				ss.m_internal_buf[i] = tmp_ptr[i];

			ss.m_internal_cap = static_cast<uint8_t>(MAX_SSO_CUNITS - tmp_len);

			free(tmp_ptr);

			return true;
		}

		return false;
	}

	bool utf8_string::change_heap_cap(uint32_t bytes) noexcept
	{
		uint32_t alloc_size = least_bigger_capacity(bytes);

		char* tmp_ptr = static_cast<char*>(realloc(ls.m_ptr, alloc_size));

		if (!tmp_ptr)
			return false;		//Reallocation failed...

		ls.m_ptr = tmp_ptr;
		ls.m_capacity = alloc_size;

		return true;
	}

	uint32_t utf8_string::least_bigger_capacity(uint32_t required_bytes) noexcept
	{
		if (required_bytes <= MIN_HEAP_CAPACITY)
			return MIN_HEAP_CAPACITY;

		--required_bytes;

		required_bytes |= required_bytes >> 1;
		required_bytes |= required_bytes >> 2;
		required_bytes |= required_bytes >> 4;
		required_bytes |= required_bytes >> 8;
		required_bytes |= required_bytes >> 16;

		return ++required_bytes;
	}

	void utf8_string::set_codeunits(uint32_t cunits) noexcept
	{
		if (is_sso())
			ss.m_internal_cap = static_cast<uint8_t>(MAX_SSO_CUNITS - cunits);
		else
			ls.m_codeunits = cunits;
	}

	void utf8_string::set_codepoints(uint32_t cpoints) noexcept
	{
		if (!is_sso())
			ls.m_codepoints = cpoints;
	}
}
