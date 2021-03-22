#pragma once

#include <cstdint>

#include "och_utf8.h"

#include "och_virtual_keys.h"

namespace och
{
	namespace evnt
	{
		enum evnt
		{
			key_dn,
			key_up,
			scroll,
			leave,
			enter,
		};
	}

	struct window
	{
		uint64_t m_keyboard[4] {};
		
		void* m_window_handle;

		int16_t m_mouse_x = -1;
		int16_t m_mouse_y = -1;
		int16_t m_mouse_scroll = 0;
		int16_t m_mouse_h_scroll = 0;

		uint32_t m_style_prefullscreen = 0;
		uint16_t m_width_prefullscreen = 0;
		uint16_t m_heigth_prefullscreen = 0;
		int16_t m_x_prefullscreen = 0;
		int16_t m_y_prefullscreen = 0;

		window(uint32_t w, uint32_t h, bool resizable, bool disable_menu, const och::stringview& title = { nullptr, 0, 0 }) noexcept;

		~window() noexcept;

		window& operator=(const window&) = delete;

		window(const window&) = delete;

		bool set_title(const utf8_view& title) noexcept;

		bool toggle_fullscreen() noexcept;

		bool minimize() const noexcept;

		bool maximize() const noexcept;

		bool show() const noexcept;

		bool key_is_down(uint8_t vk) const noexcept;

		void update() noexcept;

		void set_key(uint8_t vk) noexcept;

		void unset_key(uint8_t vk) noexcept;

		void update_mouse_pos(int64_t param) noexcept;
	};

	constexpr int temp_window_sz = sizeof(window);
}
