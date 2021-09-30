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

och::status error_test_callee()
{
	return {};
}

och::status error_test_caller()
{
	check(error_test_callee());

	return {};
}

int main()
{
	if (error_test_caller() != och::status::ok)
	{

	}

	return 0;
}
