#include "och_wnd.h"

#include <cstdint>
#include <Windows.h>

struct window_template
{
	uint32_t style;
	WNDPROC window_function;
	int32_t global_bytes;
	int32_t instance_bytes;
	HINSTANCE instance;
	HICON icon;
	HCURSOR cursor;
	HBRUSH brush;
	const char* menu_name;
	const char* class_name;
};