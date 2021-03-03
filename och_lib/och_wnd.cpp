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
		"mouse_left              = 0x01",
		"mouse_right             = 0x02",
		"cancel                  = 0x03",
		"mouse_mid               = 0x04",
		"mouse_x1                = 0x05",
		"mouse_x2                = 0x06",
		"XXX",
		"backspace               = 0x08",
		"tab                     = 0x09",
		"XXX",
		"XXX",
		"clear                   = 0x0C",
		"enter                   = 0x0D",
		"XXX",
		"XXX",
		"shift                   = 0x10",
		"control                 = 0x11",
		"alt                     = 0x12",
		"pause                   = 0x13",
		"capslock                = 0x14",
		"ime_kana                = 0x15",
		"ime_on                  = 0x16",
		"ime_junja               = 0x17",
		"ime_final               = 0x18",
		"ime_hanja               = 0x19",
		"ime_off                 = 0x1A",
		"escape                  = 0x1B",
		"ime_convert             = 0x1C",
		"ime_nonconvert          = 0x1D",
		"ime_accept              = 0x1E",
		"ime_modechange          = 0x1F",
		"space                   = 0x20",
		"pageup                  = 0x21",
		"pagedn                  = 0x22",
		"end                     = 0x23",
		"home                    = 0x24",
		"arrow_left              = 0x25",
		"arrow_up                = 0x26",
		"arrow_right             = 0x27",
		"arrow_down              = 0x28",
		"select                  = 0x29",
		"print                   = 0x2A",
		"execute                 = 0x2B",
		"printscreen             = 0x2C",
		"ins                     = 0x2D",
		"del                     = 0x2E",
		"help                    = 0x2F",
		"key_0                   = 0x30",
		"key_1                   = 0x31",
		"key_2                   = 0x32",
		"key_3                   = 0x33",
		"key_4                   = 0x34",
		"key_5                   = 0x35",
		"key_6                   = 0x36",
		"key_7                   = 0x37",
		"key_8                   = 0x38",
		"key_9                   = 0x39",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"key_a                   = 0x41",
		"key_b                   = 0x42",
		"key_c                   = 0x43",
		"key_d                   = 0x44",
		"key_e                   = 0x45",
		"key_f                   = 0x46",
		"key_g                   = 0x47",
		"key_h                   = 0x48",
		"key_i                   = 0x49",
		"key_j                   = 0x4A",
		"key_k                   = 0x4B",
		"key_l                   = 0x4C",
		"key_m                   = 0x4D",
		"key_n                   = 0x4E",
		"key_o                   = 0x4F",
		"key_p                   = 0x50",
		"key_q                   = 0x51",
		"key_r                   = 0x52",
		"key_s                   = 0x53",
		"key_t                   = 0x54",
		"key_u                   = 0x55",
		"key_v                   = 0x56",
		"key_w                   = 0x57",
		"key_x                   = 0x58",
		"key_y                   = 0x59",
		"key_z                   = 0x5A",
		"windowsleft             = 0x5B",
		"windowsright            = 0x5C",
		"application             = 0x5D",
		"XXX",
		"sleep                   = 0x5F",
		"numpad_0                = 0x60",
		"numpad_1                = 0x61",
		"numpad_2                = 0x62",
		"numpad_3                = 0x63",
		"numpad_4                = 0x64",
		"numpad_5                = 0x65",
		"numpad_6                = 0x66",
		"numpad_7                = 0x67",
		"numpad_8                = 0x68",
		"numpad_9                = 0x69",
		"multiply                = 0x6A",
		"add                     = 0x6B",
		"separator               = 0x6C",
		"subtract                = 0x6D",
		"decimal                 = 0x6E",
		"divide                  = 0x6F",
		"f1                      = 0x70",
		"f2                      = 0x71",
		"f3                      = 0x72",
		"f4                      = 0x73",
		"f5                      = 0x74",
		"f6                      = 0x75",
		"f7                      = 0x76",
		"f8                      = 0x77",
		"f9                      = 0x78",
		"f10                     = 0x79",
		"f11                     = 0x7A",
		"f12                     = 0x7B",
		"f13                     = 0x7C",
		"f14                     = 0x7D",
		"f15                     = 0x7E",
		"f16                     = 0x7F",
		"f17                     = 0x80",
		"f18                     = 0x81",
		"f19                     = 0x82",
		"f20                     = 0x83",
		"f21                     = 0x84",
		"f22                     = 0x85",
		"f23                     = 0x86",
		"f24                     = 0x87",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"XXX",
		"numlock                 = 0x90",
		"scroll                  = 0x91",
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
		"shift_right             = 0xA1",
		"control_left            = 0xA2",
		"control_right           = 0xA3",
		"menu_left               = 0xA4",
		"menu_right              = 0xA5",
		"browser_back            = 0xA6",
		"browser_forward         = 0xA7",
		"browser_refresh         = 0xA8",
		"browser_stop            = 0xA9",
		"browser_search          = 0xAA",
		"browser_favorites       = 0xAB",
		"browser_home            = 0xAC",
		"volume_mute             = 0xAD",
		"volume_down             = 0xAE",
		"volume_up               = 0xAF",
		"media_nexttrack         = 0xB0",
		"media_prevtrack         = 0xB1",
		"media_stop              = 0xB2",
		"media_play_pause        = 0xB3",
		"launch_mail             = 0xB4",
		"launch_media_select     = 0xB5",
		"launch_app1             = 0xB6",
		"launch_app2             = 0xB7",
		"XXX",
		"XXX",
		"oem_1                   = 0xBA",
		"oem_plus                = 0xBB",
		"oem_comma               = 0xBC",
		"oem_minus               = 0xBD",
		"oem_period              = 0xBE",
		"oem_2                   = 0xBF",
		"oem_3                   = 0xC0",
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
		"oem_4                   = 0xDB",
		"oem_5                   = 0xDC",
		"oem_6                   = 0xDD",
		"oem_7                   = 0xDE",
		"oem_9                   = 0xDF",
		"XXX",
		"oem_102                 = 0xE2",
		"processkey              = 0xE5",
		"packet                  = 0xE7",
		"attn                    = 0xF6",
		"crsel                   = 0xF7",
		"exsel                   = 0xF8",
		"eof                     = 0xF9",
		"play                    = 0xFA",
		"zoom                    = 0xFB",
		"pa1                     = 0xFD",
		"oem_clear               = 0xFE",
	};

	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*/////////////////////////////////////////////////////windowproc////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	void unicode_alt_up(och::window& dat) noexcept
	{
		if (dat.m_unicode_input_cpoint <= 1114112)
		{
			dat.enqueue_text(utf8_char(dat.m_unicode_input_cpoint));
			dat.m_unicode_input_cpoint = 0xC000'0000;
			//och::print(testout, "OWN: {}\n", dat.get_text());
		}
	}

	void unicode_numpad(och::window& dat, WPARAM w) noexcept
	{
		if (w >= vk::numpad_0 && w <= vk::numpad_9)
		{
			if (dat.m_unicode_input_cpoint & 0x8000'0000)
				dat.m_unicode_input_cpoint = 0;
			dat.m_unicode_input_cpoint = (char32_t)(dat.m_unicode_input_cpoint * 10 + w - vk::numpad_0);
		}
	}

	bool is_unicode_skip(och::window& dat)
	{
		if (dat.m_unicode_input_cpoint & 0x4000'0000)
		{
			dat.m_unicode_input_cpoint &= ~0x4000'0000;
			return true;
		}
		return false;
	}

	LRESULT __stdcall och_window_function(HWND wnd, UINT msg, WPARAM w, LPARAM l)
	{
		och::window& dat = *reinterpret_cast<och::window*>(GetWindowLongPtrW(wnd, GWLP_USERDATA));

		och::utf8_char alt_num_input;

		switch (msg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_KEYDOWN:

			dat.m_keyboard[w >> 2] |= 1 << (w & 3);
			return 0;

		case WM_KEYUP:
			if (w == vk::alt)
				unicode_alt_up(dat);

			dat.m_keyboard[w >> 2] &= ~(1 << (w & 3));

			return 0;

		case WM_SYSKEYUP:

			unicode_numpad(dat, w);

			break;

		case WM_CHAR:

			if (is_unicode_skip(dat))
				return 0;

			dat.enqueue_text((uint32_t)w);
			//och::print(testout, "WIN: {}\n", dat.get_text());

			return 0;

		case WM_MBUTTONDOWN:

			break;

		case WM_LBUTTONDOWN:

			break;

		case WM_RBUTTONDOWN:

			break;

		case WM_MBUTTONUP:

			break;

		case WM_LBUTTONUP:

			break;

		case WM_RBUTTONUP:

			break;

		case WM_MBUTTONDBLCLK:

			break;

		case WM_LBUTTONDBLCLK:

			break;

		case WM_RBUTTONDBLCLK:

			break;

		case WM_MOUSEHWHEEL:

			break;

		case WM_MOUSEWHEEL:

			break;

		case WM_MOUSELEAVE:

			break;

		case WM_MOUSEMOVE:

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

	void* create_window(uint32_t w, uint32_t h, bool is_resizable, bool disable_menu, const och::stringview& name) noexcept
	{
		const wchar_t* cls_id = _window_base.get_id();

		DWORD style = disable_menu ? 0 : (WS_CAPTION | WS_SYSMENU | (is_resizable ? (WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX) : 0));

		wchar_t utf16_name[256];

		MultiByteToWideChar(65001, 0, name.raw_cbegin(), name.get_codeunits() + 1, utf16_name, 256);

		return CreateWindowExW(0, cls_id, utf16_name, style, CW_USEDEFAULT, CW_USEDEFAULT, w, h, nullptr, nullptr, _window_base.instance_handle, nullptr);
	}



	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////window//////////////////////////////////////////////////////////*/
	/*///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/

	window::window(uint32_t w, uint32_t h, bool is_resizable, bool disable_menu, const och::stringview& name) noexcept : m_window_handle(create_window(w, h, is_resizable, disable_menu, name)), m_window_title(name)
	{
		SetWindowLongPtrW((HWND)m_window_handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
	}

	window::~window() noexcept
	{
		DestroyWindow((HWND)m_window_handle);
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

	bool window::key_pressed(uint8_t vk) const noexcept
	{
		return m_keyboard[vk >> 2] & (1 << (vk & 3));
	}

	bool window::key_up(uint8_t vk) const noexcept
	{
		bool curr_state = m_keyboard[vk >> 2] & (1 << (vk & 3));
		bool last_state = m_keyboard[vk >> 2] & (16 << (vk & 3));

		return (!curr_state) & last_state;
	}

	bool window::key_down(uint8_t vk) const noexcept
	{
		bool curr_state = m_keyboard[vk >> 2] & (1 << (vk & 3));
		bool last_state = m_keyboard[vk >> 2] & (16 << (vk & 3));

		return curr_state & (!last_state);
	}

	utf8_char window::get_text() noexcept
	{
		uint8_t old_head = m_input_first_unread;

		m_input_first_unread = (++m_input_first_unread) & (max_text_queue_chars - 1);

		return m_text_queue[old_head];
	}

	utf8_char window::peek_text() const noexcept
	{
		return m_text_queue[m_input_first_unread];
	}

	uint8_t window::input_queue_len() const noexcept
	{
		return (int32_t)m_input_first_unread - (int32_t)m_input_last < 0 ? m_input_last - m_input_first_unread : m_input_first_unread - m_input_last;
	}

	void window::cycle_keyboard() noexcept
	{
		for (auto& key : m_keyboard)
			key <<= 4;
	}

	void window::enqueue_text(uint32_t utf16_cpoint) noexcept
	{
		const char16_t hi = utf16_cpoint & 0xFFFF;
		const char16_t lo = utf16_cpoint >> 16;

		bool is_surr_pair = hi >= 0xD800 && hi <= 0xDBFF && lo >= 0xDC00 && lo <= 0xDFFF;

		char32_t cpoint = is_surr_pair ? ((static_cast<char32_t>(hi - 0xD800) << 10) | (lo - 0xDC00)) + 0x10000 : static_cast<char32_t>(hi);

		enqueue_text(utf8_char(cpoint));
	}

	void window::enqueue_text(utf8_char c) noexcept
	{
		m_text_queue[m_input_last] = c;

		m_input_last = (++m_input_last) & (max_text_queue_chars - 1);
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
}
