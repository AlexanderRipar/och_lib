#pragma once

#include "och_fmt.h"

constexpr const char* find_substr_after(const char* str, char c)
{
	int last_occ = -1;

	for (int i = 0; str[i]; ++i)
		if (str[i] == c)
			last_occ = i;

	return str + last_occ + 1;
}

#define S(x) #x
#define S_(x) S(x)
#define S__LINE__ S_(__LINE__)

//#define OCH_DEPRECATED_ERRLOG(errmsg) printf("\nERROR: %s\n\t+----------------------------\n\t| File: %s\n\t| Func: " __FUNCTION__ "\n\t| Line: " S__LINE__ "\n\t+----------------------------\n", (errmsg), find_substr_after(__FILE__, '\\'));

#define OCH_ERRLOG(errmsg) och::print("\nERROR: {}\n\t+----------------------------\n\type| File: {}\n\t| Func: {}\n\t| Line: %d\n\t+----------------------------\n", (errmsg), find_substr_after(__FILE__, '\\'), __FUNCSIG__, __LINE__);

#define OCH_DEBUG 1

#ifdef OCH_DEBUG

#define OCH_IF_DEBUG(arg) arg

#else

#define OCH_IF_DEBUG(arg)

#endif // OCH_DEBUG
