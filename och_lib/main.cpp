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

#include <Windows.h>

#include "och_err.h"

och::error error_test_callee()
{
	check((HRESULT)1);

	return {};
}

och::error error_test_caller()
{
	check(error_test_callee());

	return {};
}

int main()
{
	if (error_test_caller() != och::error::success)
	{

	}

	return 0;
}
