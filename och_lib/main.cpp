#include <Windows.h>

#include "och_matmath.h"
#include "och_constexpr_util.h"
#include "och_fio.h"
#include "och_matmath.h"
#include "och_range.h"
#include "och_time.h"
#include "och_timer.h"
#include "och_type_union.h"
#include "och_utf8.h"
#include "och_utf16.h"
#include "och_virtual_keys.h"
#include "och_err.h"
#include "och_fmt.h"

och::status s2()
{
	och::iohandle h;

	check(och::open_file(h, "PLOOBL", och::fio::access::read, och::fio::open::append, och::fio::open::fail));

	return {};
}

och::status s1()
{
	check(s2());

	return {};
}

och::status s0()
{
	check(s1());

	return {};
}

int main()
{
	//och::file_search search1;
	//
	//och::status s1 = search1.create("Hello", och::fio::search::all, nullptr);

	och::file_search search2;

	och::status s2 = search2.create("C:\\Users\\alex_2\\Documents\\_Programming\\incomplete_and_ideas", och::fio::search::all, ".txt");

	if (!s2)
	{
		while (search2.has_more())
		{
			och::print("{}\n", search2.curr_name());

			check(search2.advance());
		}
	}
	else
	{
		och::print("{}\n", s2.description());
	}
}
