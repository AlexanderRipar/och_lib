#pragma once

#include "och_wnd.h"

#include <cstdint>
#include <Windows.h>

#include "och_utf8.h"
#include "och_fmt.h"

och::filehandle testout("testout.txt", och::fio::access_write, och::fio::open_truncate, och::fio::open_normal);

namespace och
{
	const char* vk_names[]
	{
		"XXX",
		"mouse_left",
		"mouse_right",
		"cancel",
		"mouse_mid",
		"mouse_x1",
		"mouse_x2",
		"XXX",
		"backspace",
		"tab",
		"XXX",
		"XXX",
		"clear",
		"enter",
		"XXX",
		"XXX",
		"shift",
		"control",
		"alt",
		"pause",
		"capslock",
		"ime_kana",
		"ime_on",
		"ime_junja",
		"ime_final",
		"ime_hanja",
		"ime_off",
		"escape",
		"ime_convert",
		"ime_nonconvert",
		"ime_accept",
		"ime_modechange",
		"space",
		"pageup",
		"pagedn",
		"end",
		"home",
		"arrow_left",
		"arrow_up",
		"arrow_right",
		"arrow_down",
		"select",
		"print",
		"execute",
		"printscreen",
		"ins",
		"del",
		"help",
		"key_0",
		"key_1",
		"key_2",
		"key_3",
		"key_4",
		"key_5",
		"key_6",
		"key_7",
		"key_8",
		"key_9",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"key_a",
		"key_b",
		"key_c",
		"key_d",
		"key_e",
		"key_f",
		"key_g",
		"key_h",
		"key_i",
		"key_j",
		"key_k",
		"key_l",
		"key_m",
		"key_n",
		"key_o",
		"key_p",
		"key_q",
		"key_r",
		"key_s",
		"key_t",
		"key_u",
		"key_v",
		"key_w",
		"key_x",
		"key_y",
		"key_z",
		"windowsleft",
		"windowsright",
		"application",
		"XXX",
		"sleep",
		"numpad_0",
		"numpad_1",
		"numpad_2",
		"numpad_3",
		"numpad_4",
		"numpad_5",
		"numpad_6",
		"numpad_7",
		"numpad_8",
		"numpad_9",
		"multiply",
		"add",
		"separator",
		"subtract",
		"decimal",
		"divide",
		"f1",
		"f2",
		"f3",
		"f4",
		"f5",
		"f6",
		"f7",
		"f8",
		"f9",
		"f10",
		"f11",
		"f12",
		"f13",
		"f14",
		"f15",
		"f16",
		"f17",
		"f18",
		"f19",
		"f20",
		"f21",
		"f22",
		"f23",
		"f24",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"numlock",
		"scroll_lock",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"shift_right",
		"control_left",
		"control_right",
		"alt_left",
		"alt_right",
		"browser_back",
		"browser_forward",
		"browser_refresh",
		"browser_stop",
		"browser_search",
		"browser_favorites",
		"browser_home",
		"volume_mute",
		"volume_down",
		"volume_up",
		"media_nexttrack",
		"media_prevtrack",
		"media_stop",
		"media_play_pause",
		"launch_mail",
		"launch_media_select",
		"launch_app1",
		"launch_app2",
		"XXX",
		"XXX",
		"oem_1",
		"oem_plus",
		"oem_comma",
		"oem_minus",
		"oem_period",
		"oem_2",
		"oem_3",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"oem_4",
		"oem_5",
		"oem_6",
		"oem_7",
		"oem_9",
		"XXX",
		"XXX",
		"oem_102",
		"XXX",
		"XXX",
		"ime_processkey",
		"XXX",
		"packet",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"attn",
		"crsel",
		"exsel",
		"eof",
		"play",
		"zoom",
		"XXX",
		"pa1",
		"oem_clear",
	};

	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////////windowproc////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	LRESULT __stdcall och_window_function(HWND wnd, UINT msg, WPARAM w, LPARAM l)
	{
		och::window& dat = *reinterpret_cast<och::window*>(GetWindowLongPtrW(wnd, GWLP_USERDATA));

		switch (msg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
			dat.set_key((uint8_t)w);
			break;

		case WM_KEYUP:
		case WM_SYSKEYUP:
			dat.unset_key((uint8_t)w);
			break;

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_XBUTTONDOWN:
			dat.update_mouse_pos(l);
			dat.set_key(static_cast<uint8_t>(msg - (msg >> 1) - (msg == WM_XBUTTONDOWN && (w & (1 << 16)))));//Figure out key from low four bits of message
			break;

		case WM_LBUTTONUP:
		case WM_RBUTTONUP:
		case WM_MBUTTONUP:
		case WM_XBUTTONUP:
			dat.update_mouse_pos(l);
				dat.unset_key(static_cast<uint8_t>((msg >> 1) - (msg == WM_XBUTTONUP && (w & (1 << 16)))));//Figure out key from low four bits of message
			break;

		case WM_MOUSEHWHEEL:
			dat.update_mouse_pos(l);
			dat.m_mouse_h_scroll += static_cast<int16_t>(w >> 16);
			break;

		case WM_MOUSEWHEEL:
			dat.update_mouse_pos(l);
			dat.m_mouse_scroll += static_cast<int16_t>(w >> 16);
			break;

		case WM_MOUSEMOVE:
			dat.update_mouse_pos(l);
			break;
		}

		return DefWindowProcW(wnd, msg, w, l);
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*//////////////////////////////////////////////////////internals////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	struct _window_base_t
	{
		const uint16_t window_id = init_window_id();
		const HINSTANCE instance_handle = GetModuleHandleW(nullptr);

		ATOM init_window_id() const noexcept
		{
			WNDCLASSW w{};

			w.style = CS_OWNDC;

			w.lpfnWndProc = och_window_function;

			w.hInstance = GetModuleHandle(nullptr);

			w.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNSHADOW + 1);//(COLOR_BTNFACE + 1);
			w.lpszClassName = L"och_basic_window_class";

			return static_cast<uint16_t>(RegisterClassW(&w));
		}

		const wchar_t* get_id() const noexcept
		{
			return reinterpret_cast<const wchar_t*>(window_id);
		}

		~_window_base_t()
		{
			UnregisterClassW(get_id(), (HINSTANCE)instance_handle);
		}

	} _window_base;

	void* create_window(uint32_t w, uint32_t h, bool is_resizable, bool disable_menu, const och::stringview& title) noexcept
	{
		const wchar_t* cls_id = _window_base.get_id();

		DWORD style = disable_menu ? 0 : (WS_CAPTION | WS_SYSMENU | (is_resizable ? (WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX) : 0));

		wchar_t utf16_name[256];

		uint32_t utf16_units = MultiByteToWideChar(65001, 0, title.raw_cbegin(), title.get_codeunits(), utf16_name, 25);

		if (!utf16_units && title.get_codeunits())
		{
			utf16_name[0] = u'E';
			utf16_name[1] = u'r';
			utf16_name[2] = u'r';
			utf16_name[3] = u'o';
			utf16_name[4] = u'r';
			utf16_name[5] = u'\0';
		}
		else
			utf16_name[utf16_units] = 0;

		return CreateWindowExW(0, cls_id, utf16_name, style, CW_USEDEFAULT, CW_USEDEFAULT, w, h, nullptr, nullptr, _window_base.instance_handle, nullptr);
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////window//////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	window::window(uint32_t w, uint32_t h, bool is_resizable, bool disable_menu, const och::stringview& title) noexcept : m_window_handle(create_window(w, h, is_resizable, disable_menu, title))
	{
		SetWindowLongPtrW((HWND)m_window_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}

	window::~window() noexcept
	{
		DestroyWindow((HWND)m_window_handle);
	}

	bool window::set_title(const utf8_view& title) noexcept
	{
		wchar_t utf16_name[256];

		uint32_t utf16_units = MultiByteToWideChar(65001, 0, title.raw_cbegin(), title.get_codeunits(), utf16_name, 25);

		if (!utf16_units && title.get_codeunits())
			return false;

		utf16_name[utf16_units] = 0;

		return SetWindowTextW((HWND)m_window_handle, utf16_name);
	}

	bool window::toggle_fullscreen() noexcept
	{
		uint32_t style = (uint32_t)GetWindowLongPtrW((HWND)m_window_handle, GWL_STYLE);

		if (style & WS_POPUP)//Exiting fullscreen
		{
			//Restore style
			SetWindowLongPtrW((HWND)m_window_handle, GWL_STYLE, m_style_prefullscreen);

			//Restore position
			SetWindowPos((HWND)m_window_handle, HWND_TOP, m_x_prefullscreen, m_y_prefullscreen, m_width_prefullscreen, m_heigth_prefullscreen, SWP_FRAMECHANGED);
		}
		else//Entering fullscreen
		{
			//Save window-position for eventually exiting fullscreen
			RECT r;

			GetWindowRect((HWND)m_window_handle, &r);

			m_width_prefullscreen = (uint16_t)(r.right - r.left);
			m_heigth_prefullscreen = (uint16_t)(r.bottom - r.top);
			m_x_prefullscreen = (int16_t)r.left;
			m_y_prefullscreen = (int16_t)r.top;
			m_style_prefullscreen = (uint32_t)GetWindowLongPtrW((HWND)m_window_handle, GWL_STYLE);

			//Set window-style to popup-window, to remove frame
			SetWindowLongPtrW((HWND)m_window_handle, GWL_STYLE, WS_POPUP);

			HMONITOR m = MonitorFromWindow((HWND)m_window_handle, 0);
			MONITORINFO mi{ sizeof(mi) };
			GetMonitorInfoA(m, &mi);
			const RECT& mr = mi.rcMonitor;

			//This is (probably) necessary, and a call to ShowWindow with SW_MAXIMIZE won't suffice, 
			//as SWP_FRAMECHANGED is (theoretically) needed to update the cached window-frame
			SetWindowPos((HWND)m_window_handle, HWND_TOP, mr.left, mr.top, mr.right - mr.left, mr.bottom - mr.top, SWP_FRAMECHANGED);
		}

		return true;
	}

	bool window::minimize() const noexcept
	{
		return ShowWindow((HWND)m_window_handle, SW_MINIMIZE);
	}

	bool window::maximize() const noexcept
	{
		return ShowWindow((HWND)m_window_handle, SW_MAXIMIZE);
	}

	bool window::show() const noexcept
	{
		return ShowWindow((HWND)m_window_handle, SW_NORMAL);
	}

	bool window::key_is_down(uint8_t vk) const noexcept
	{
		return m_keyboard[vk >> 6] & (1ull << (vk & 63));
	}

	void window::update() noexcept
	{
		MSG m{};

		while (GetMessageW(&m, (HWND)m_window_handle, 0, 0) > 0)
		{
			TranslateMessage(&m);

			DispatchMessageW(&m);
		}
	}

	void window::set_key(uint8_t vk) noexcept
	{
		m_keyboard[vk >> 6] |= 1ull << (vk & 63);
	}

	void window::unset_key(uint8_t vk) noexcept
	{
		m_keyboard[vk >> 6] &= ~(1ull << (vk & 63));
	}

	void window::update_mouse_pos(int64_t lparam) noexcept
	{
		m_mouse_x = static_cast<int16_t>(lparam & 0xFFFF);
		m_mouse_y = static_cast<int16_t>((lparam >> 16) & 0xFFFF);
	}
}
