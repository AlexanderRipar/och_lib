#pragma once

#include <cstdint>

#include "och_utf8.h"

namespace och
{
	namespace vk
	{
		enum virtual_keys : uint8_t
		{
			mouse_left              = 0x01,
			mouse_right             = 0x02,
			cancel                  = 0x03,
			mouse_mid               = 0x04,
			mouse_x1                = 0x05,
			mouse_x2                = 0x06,
			backspace               = 0x08,
			tab                     = 0x09,
			clear                   = 0x0C,
			enter                   = 0x0D,
			shift                   = 0x10,
			control                 = 0x11,
			alt                     = 0x12,
			pause                   = 0x13,
			capslock                = 0x14,
			ime_kana                = 0x15,
			ime_hangul              = 0x15,
			ime_on                  = 0x16,
			ime_junja               = 0x17,
			ime_final               = 0x18,
			ime_hanja               = 0x19,
			ime_kanji               = 0x19,
			ime_off                 = 0x1A,
			escape                  = 0x1B,
			ime_convert             = 0x1C,
			ime_nonconvert          = 0x1D,
			ime_accept              = 0x1E,
			ime_modechange          = 0x1F,
			space                   = 0x20,
			pageup                  = 0x21,
			pagedn                  = 0x22,
			end                     = 0x23,
			home                    = 0x24,
			arrow_left              = 0x25,
			arrow_up                = 0x26,
			arrow_right             = 0x27,
			arrow_down              = 0x28,
			select                  = 0x29,
			print                   = 0x2A,
			execute                 = 0x2B,
			printscreen             = 0x2C,
			ins                     = 0x2D,
			del                     = 0x2E,
			help                    = 0x2F,
			key_0                   = 0x30,
			key_1                   = 0x31,
			key_2                   = 0x32,
			key_3                   = 0x33,
			key_4                   = 0x34,
			key_5                   = 0x35,
			key_6                   = 0x36,
			key_7                   = 0x37,
			key_8                   = 0x38,
			key_9                   = 0x39,
			key_a                   = 0x41,
			key_b                   = 0x42,
			key_c                   = 0x43,
			key_d                   = 0x44,
			key_e                   = 0x45,
			key_f                   = 0x46,
			key_g                   = 0x47,
			key_h                   = 0x48,
			key_i                   = 0x49,
			key_j                   = 0x4A,
			key_k                   = 0x4B,
			key_l                   = 0x4C,
			key_m                   = 0x4D,
			key_n                   = 0x4E,
			key_o                   = 0x4F,
			key_p                   = 0x50,
			key_q                   = 0x51,
			key_r                   = 0x52,
			key_s                   = 0x53,
			key_t                   = 0x54,
			key_u                   = 0x55,
			key_v                   = 0x56,
			key_w                   = 0x57,
			key_x                   = 0x58,
			key_y                   = 0x59,
			key_z                   = 0x5A,
			windowsleft             = 0x5B,
			windowsright            = 0x5C,
			application             = 0x5D,
			sleep                   = 0x5F,
			numpad_0                = 0x60,
			numpad_1                = 0x61,
			numpad_2                = 0x62,
			numpad_3                = 0x63,
			numpad_4                = 0x64,
			numpad_5                = 0x65,
			numpad_6                = 0x66,
			numpad_7                = 0x67,
			numpad_8                = 0x68,
			numpad_9                = 0x69,
			multiply                = 0x6A,
			add                     = 0x6B,
			separator               = 0x6C,
			subtract                = 0x6D,
			decimal                 = 0x6E,
			divide                  = 0x6F,
			f1                      = 0x70,
			f2                      = 0x71,
			f3                      = 0x72,
			f4                      = 0x73,
			f5                      = 0x74,
			f6                      = 0x75,
			f7                      = 0x76,
			f8                      = 0x77,
			f9                      = 0x78,
			f10                     = 0x79,
			f11                     = 0x7A,
			f12                     = 0x7B,
			f13                     = 0x7C,
			f14                     = 0x7D,
			f15                     = 0x7E,
			f16                     = 0x7F,
			f17                     = 0x80,
			f18                     = 0x81,
			f19                     = 0x82,
			f20                     = 0x83,
			f21                     = 0x84,
			f22                     = 0x85,
			f23                     = 0x86,
			f24                     = 0x87,
			numlock                 = 0x90,
			scroll                  = 0x91,
			shift_right             = 0xA1,
			control_left            = 0xA2,
			control_right           = 0xA3,
			menu_left               = 0xA4,
			menu_right              = 0xA5,
			browser_back            = 0xA6,
			browser_forward         = 0xA7,
			browser_refresh         = 0xA8,
			browser_stop            = 0xA9,
			browser_search          = 0xAA,
			browser_favorites       = 0xAB,
			browser_home            = 0xAC,
			volume_mute             = 0xAD,
			volume_down             = 0xAE,
			volume_up               = 0xAF,
			media_nexttrack         = 0xB0,
			media_prevtrack         = 0xB1,
			media_stop              = 0xB2,
			media_play_pause        = 0xB3,
			launch_mail             = 0xB4,
			launch_media_select     = 0xB5,
			launch_app1             = 0xB6,
			launch_app2             = 0xB7,
			oem_1                   = 0xBA,
			oem_plus                = 0xBB,
			oem_comma               = 0xBC,
			oem_minus               = 0xBD,
			oem_period              = 0xBE,
			oem_2                   = 0xBF,
			oem_3                   = 0xC0,
			oem_4                   = 0xDB,
			oem_5                   = 0xDC,
			oem_6                   = 0xDD,
			oem_7                   = 0xDE,
			oem_9                   = 0xDF,
			oem_102                 = 0xE2,
			processkey              = 0xE5,
			packet                  = 0xE7,
			attn                    = 0xF6,
			crsel                   = 0xF7,
			exsel                   = 0xF8,
			eof                     = 0xF9,
			play                    = 0xFA,
			zoom                    = 0xFB,
			pa1                     = 0xFD,
			oem_clear               = 0xFE,
		};
	}


	struct window
	{
		static constexpr uint8_t max_text_queue_chars{ 32 };

		uint8_t m_keyboard[64] {};
		
		utf8_char m_text_queue[max_text_queue_chars]; //utf-8

		void* m_window_handle;

		och::stringview m_window_title;

		uint8_t m_input_first_unread = 0;

		uint8_t m_input_last = 0;

		uint32_t m_style_prefullscreen = 0;
		uint16_t m_width_prefullscreen = 0;
		uint16_t m_heigth_prefullscreen = 0;
		int16_t m_x_prefullscreen = 0;
		int16_t m_y_prefullscreen = 0;

		char32_t m_unicode_input_cpoint = 0x8000'0000;

		window(uint32_t w, uint32_t h, bool resizable, bool disable_menu, const och::stringview& name = { nullptr, 0, 0 }) noexcept;

		~window() noexcept;

		window& operator=(const window&) = delete;

		window(const window&) = delete;

		bool toggle_fullscreen() noexcept;

		bool minimize() const noexcept;

		bool maximize() const noexcept;

		bool show() const noexcept;

		bool key_pressed(uint8_t vk) const noexcept;

		bool key_up(uint8_t vk) const noexcept;

		bool key_down(uint8_t vk) const noexcept;

		uint8_t input_queue_len() const noexcept;

		utf8_char get_text() noexcept;

		utf8_char peek_text() const noexcept;

		void enqueue_text(uint32_t utf16_cpoint) noexcept;

		void enqueue_text(utf8_char c) noexcept;

		void update() noexcept;

		void cycle_keyboard() noexcept;
	};

	constexpr int window_sz = sizeof(window);
}
