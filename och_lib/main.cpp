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
	return make_status(0x0101);
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
	if (s0())
	{
		och::print("Error 0x{:X} from {}\n\n{}\n\n", och::err::get_native_error_code(), static_cast<uint32_t>(och::err::get_error_type()), och::err::get_error_description());

		och::range<const och::error_context> callstack = och::err::get_callstack();

		for(auto& ctx : callstack)
			och::print("File: {}\nFunction: {}\nLine: {} ({})\n\n", ctx.filename(), ctx.function(), ctx.line_number(), ctx.line_content());
	}
	else
		och::print("All good");
}
