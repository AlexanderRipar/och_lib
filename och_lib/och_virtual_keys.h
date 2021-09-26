#define OCH_VIRTUAL_KEYS_PRESENT

#ifndef OCH_VIRTUAL_KEYS_INCLUDE_GUARD
#define OCH_VIRTUAL_KEYS_INCLUDE_GUARD

#include <cstdint>

namespace och
{
	enum class vk : uint8_t
	{
		mouse_left              = 0x01, // 
		mouse_right             = 0x02, // 
		cancel                  = 0x03, // 
		mouse_mid               = 0x04, // 
		mouse_x1                = 0x05, // Additional mouse-button
		mouse_x2                = 0x06, // Additional mouse-button
		backspace               = 0x08, // 
		tab                     = 0x09, // 
		clear                   = 0x0C, // 
		enter                   = 0x0D, // 
		shift                   = 0x10, // 
		control                 = 0x11, // 
		alt                     = 0x12, // 
		pause                   = 0x13, // 
		capslock                = 0x14, // 
		ime_kana                = 0x15, // Jp / Kr input; ? [Toggle input method (Hiragana, Full-width Katakana, Half-width Katakana)]
		ime_hangul              = 0x15, // Jp / Kr input; ? [Korean]
		ime_on                  = 0x16, // Jp / Kr input; ? [Activates input method editor]
		ime_junja               = 0x17, // Jp / Kr input; ?
		ime_final               = 0x18, // Jp / Kr input; ?
		ime_hanja               = 0x19, // Jp / Kr input; ? [Convert Hangul to korean Han script (hanji)]
		ime_kanji               = 0x19, // Jp / Kr input; ? [Convert to Japanese Han script (kanji)]
		ime_off                 = 0x1A, // Jp / Kr input; ? [Deactivates input method editor]
		escape                  = 0x1B, // 
		ime_convert             = 0x1C, // Jp / Kr input; ?
		ime_nonconvert          = 0x1D, // Jp / Kr input; ?
		ime_accept              = 0x1E, // Jp / Kr input; ?
		ime_modechange          = 0x1F, // Jp / Kr input; ?
		space                   = 0x20, // 
		pageup                  = 0x21, // 
		pagedn                  = 0x22, // 
		end                     = 0x23, // 
		home                    = 0x24, // 
		arrow_left              = 0x25, // 
		arrow_up                = 0x26, // 
		arrow_right             = 0x27, // 
		arrow_down              = 0x28, // 
		select                  = 0x29, // 
		print                   = 0x2A, // 
		execute                 = 0x2B, // 
		printscreen             = 0x2C, // 
		ins                     = 0x2D, // 
		del                     = 0x2E, // 
		help                    = 0x2F, // 
		key_0                   = 0x30, // 
		key_1                   = 0x31, // 
		key_2                   = 0x32, // 
		key_3                   = 0x33, // 
		key_4                   = 0x34, // 
		key_5                   = 0x35, // 
		key_6                   = 0x36, // 
		key_7                   = 0x37, // 
		key_8                   = 0x38, // 
		key_9                   = 0x39, // 
		key_a                   = 0x41, // 
		key_b                   = 0x42, // 
		key_c                   = 0x43, // 
		key_d                   = 0x44, // 
		key_e                   = 0x45, // 
		key_f                   = 0x46, // 
		key_g                   = 0x47, // 
		key_h                   = 0x48, // 
		key_i                   = 0x49, // 
		key_j                   = 0x4A, // 
		key_k                   = 0x4B, // 
		key_l                   = 0x4C, // 
		key_m                   = 0x4D, // 
		key_n                   = 0x4E, // 
		key_o                   = 0x4F, // 
		key_p                   = 0x50, // 
		key_q                   = 0x51, // 
		key_r                   = 0x52, // 
		key_s                   = 0x53, // 
		key_t                   = 0x54, // 
		key_u                   = 0x55, // 
		key_v                   = 0x56, // 
		key_w                   = 0x57, // 
		key_x                   = 0x58, // 
		key_y                   = 0x59, // 
		key_z                   = 0x5A, // 
		windowsleft             = 0x5B, // 
		windowsright            = 0x5C, // 
		application             = 0x5D, // 
		sleep                   = 0x5F, // 
		numpad_0                = 0x60, // 
		numpad_1                = 0x61, // 
		numpad_2                = 0x62, // 
		numpad_3                = 0x63, // 
		numpad_4                = 0x64, // 
		numpad_5                = 0x65, // 
		numpad_6                = 0x66, // 
		numpad_7                = 0x67, // 
		numpad_8                = 0x68, // 
		numpad_9                = 0x69, // 
		multiply                = 0x6A, // 
		add                     = 0x6B, // 
		separator               = 0x6C, // 
		subtract                = 0x6D, // 
		decimal                 = 0x6E, // 
		divide                  = 0x6F, // 
		f1                      = 0x70, // 
		f2                      = 0x71, // 
		f3                      = 0x72, // 
		f4                      = 0x73, // 
		f5                      = 0x74, // 
		f6                      = 0x75, // 
		f7                      = 0x76, // 
		f8                      = 0x77, // 
		f9                      = 0x78, // 
		f10                     = 0x79, // 
		f11                     = 0x7A, // 
		f12                     = 0x7B, // 
		f13                     = 0x7C, // 
		f14                     = 0x7D, // 
		f15                     = 0x7E, // 
		f16                     = 0x7F, // 
		f17                     = 0x80, // 
		f18                     = 0x81, // 
		f19                     = 0x82, // 
		f20                     = 0x83, // 
		f21                     = 0x84, // 
		f22                     = 0x85, // 
		f23                     = 0x86, // 
		f24                     = 0x87, // 
		numlock                 = 0x90, // 
		scroll_lock             = 0x91, // 
		shift_right             = 0xA1, // 
		control_left            = 0xA2, // 
		control_right           = 0xA3, // 
		alt_left                = 0xA4, // 
		alt_right               = 0xA5, // 
		browser_back            = 0xA6, // ?
		browser_forward         = 0xA7, // ?
		browser_refresh         = 0xA8, // ?
		browser_stop            = 0xA9, // ?
		browser_search          = 0xAA, // ?
		browser_favorites       = 0xAB, // ?
		browser_home            = 0xAC, // ?
		volume_mute             = 0xAD, // 
		volume_down             = 0xAE, // 
		volume_up               = 0xAF, // 
		media_nexttrack         = 0xB0, // ?
		media_prevtrack         = 0xB1, // ?
		media_stop              = 0xB2, // ?
		media_play_pause        = 0xB3, // ?
		launch_mail             = 0xB4, // ?
		launch_media_select     = 0xB5, // ?
		launch_app1             = 0xB6, // ?
		launch_app2             = 0xB7, // ?
		oem_1                   = 0xBA, // 
		oem_plus                = 0xBB, // 
		oem_comma               = 0xBC, // 
		oem_minus               = 0xBD, // 
		oem_period              = 0xBE, // 
		oem_2                   = 0xBF, // 
		oem_3                   = 0xC0, // 
		oem_4                   = 0xDB, // 
		oem_5                   = 0xDC, // 
		oem_6                   = 0xDD, // 
		oem_7                   = 0xDE, // 
		oem_9                   = 0xDF, // 
		oem_102                 = 0xE2, // 
		ime_processkey          = 0xE5, // ?
		packet                  = 0xE7, // ?
		attn                    = 0xF6, // ?
		crsel                   = 0xF7, // ?
		exsel                   = 0xF8, // ?
		erase_to_eof            = 0xF9, // ?
		play                    = 0xFA, // ?
		zoom                    = 0xFB, // ?
		pa1                     = 0xFD, // ?
		oem_clear               = 0xFE, // 
	};

	constexpr const char* virtual_key_names[]
	{
		"0x00",
		"mouse_left",
		"mouse_right",
		"cancel",
		"mouse_mid",
		"mouse_x1",
		"mouse_x2",
		"0x07",
		"backspace",
		"tab",
		"0x0A",
		"0x0B",
		"clear",
		"enter",
		"0x0E",
		"0x0F",
		"shift",
		"control",
		"alt",
		"pause",
		"capslock",
		"ime_kana/ime_hangul",
		"ime_on",
		"ime_junja",
		"ime_final",
		"ime_hanja/ime_kanji",
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
		"0x3A",
		"0x3B",
		"0x3C",
		"0x3D",
		"0x3E",
		"0x3F",
		"0x40",
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
		"0x5E",
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
		"0x88",
		"0x89",
		"0x8A",
		"0x8B",
		"0x8C",
		"0x8D",
		"0x8E",
		"0x8F",
		"numlock",
		"scroll_lock",
		"0x92",
		"0x93",
		"0x94",
		"0x95",
		"0x96",
		"0x97",
		"0x98",
		"0x99",
		"0x9A",
		"0x9B",
		"0x9C",
		"0x9D",
		"0x9E",
		"0x9F",
		"0xA0",
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
		"0xB8",
		"0xB9",
		"oem_1",
		"oem_plus",
		"oem_comma",
		"oem_minus",
		"oem_period",
		"oem_2",
		"oem_3",
		"0xC1",
		"0xC2",
		"0xC3",
		"0xC4",
		"0xC5",
		"0xC6",
		"0xC7",
		"0xC8",
		"0xC9",
		"0xCA",
		"0xCB",
		"0xCC",
		"0xCD",
		"0xCE",
		"0xCF",
		"0xD0",
		"0xD1",
		"0xD2",
		"0xD3",
		"0xD4",
		"0xD5",
		"0xD6",
		"0xD7",
		"0xD8",
		"0xD9",
		"0xDA",
		"oem_4",
		"oem_5",
		"oem_6",
		"oem_7",
		"oem_9",
		"0xE0",
		"0xE1",
		"oem_102",
		"0xE3",
		"0xE4",
		"ime_processkey",
		"0xE6",
		"packet",
		"0xE8",
		"0xE9",
		"0xEA",
		"0xEB",
		"0xEC",
		"0xED",
		"0xEE",
		"0xEF",
		"0xF0",
		"0xF1",
		"0xF2",
		"0xF3",
		"0xF4",
		"0xF5",
		"attn",
		"crsel",
		"exsel",
		"erase_to_eof",
		"play",
		"zoom",
		"0xFC",
		"pa1",
		"oem_clear",
		"0xFF",
	};
 }

#endif // !OCH_VIRTUAL_KEYS_INCLUDE_GUARD
