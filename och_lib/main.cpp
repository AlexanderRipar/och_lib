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
#include "och_fmt.h"
#include "och_err.h"

och::status test1()
{
	return make_status(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
}

int main()
{
	if (test1())
	{
		och::print("Error 0x{:X} from {}\n\n{}\n\n", och::err::get_native_error_code(), static_cast<uint32_t>(och::err::get_error_type()), och::err::get_error_description());

		for (uint32_t i = 0, lim = och::err::get_stack_depth(); i != lim; ++i)
		{
			const och::error_context& ctx = och::err::get_error_context(i);

			och::print("File: {}\nFunction: {}\nLine: {} ({})\n\n", ctx.file, ctx.function, ctx.line, ctx.line_content);
		}
	}
	else
		och::print("All good");
}
