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

int main()
{
	const char* directory = "C:\\Users\\alex_2\\Documents\\_TU_Informatik"; // "C:\\Users\\alex_2\\Documents\\_Programming\\incomplete_and_ideas\\";
	
	och::recursive_file_search search;
	
	if (och::status rst = search.create(directory, och::fio::search::all, nullptr, 0))
	{
		och::print("Could not create search: {}\n", rst.description());
	
		return 0;
	}
	
	och::print("Starting enumeration:\n\n");
	
	uint32_t file_cnt = 0, dir_cnt = 0, hidden_cnt = 0;
	
	while (search.has_more())
	{
		file_cnt += search.curr_is_file();
	
		dir_cnt += search.curr_is_directory();
	
		hidden_cnt += search.curr_is_hidden();
	
		och::print("{} {}\n", search.curr_is_directory() ? "[[D]]" : "[[F]]", search.curr_path());
	
		if (och::status rst = search.advance())
		{
			och::print("Could not advance: {} ({})\n", rst.description(), rst.errcode());
	
			break;
		}
	}
	
	och::print("\n\n{} elements found in total.\n", file_cnt + dir_cnt);
	
	if(file_cnt)
		och::print("{} of which were files.\n", file_cnt);
	
	if (dir_cnt)
		och::print("{} of which were directories.\n", dir_cnt);
	
	if (hidden_cnt)
		och::print("{} of which were hidden.\n", hidden_cnt);
}
