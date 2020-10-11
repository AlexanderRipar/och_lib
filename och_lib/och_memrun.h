#pragma once

#include <cstdint>
#include <cstring>

#include "och_constexpr_util.h"
#include <type_traits>

namespace och
{
	constexpr uint16_t _const_strlen_u16(const char* str)
	{
		int len = 0;

		while (*(str++))
			++len;

		return len;
	}

	template<typename T>
	struct memrun
	{
		T* _beg, * _end;

		template<size_t Len>
		memrun(T(&arr)[Len]) : _beg{ arr }, _end{ arr + Len } {}

		memrun(T* beg, T* end) : _beg{ beg }, _end{ end } {}

		memrun(T* beg, size_t len) : _beg{ beg }, _end{ beg + len } {}

		memrun(const char* cstr) : _beg{ cstr }, _end{ cstr + strlen(cstr) } { static_assert(std::is_same<const char, T>::value, "och::memrun<T>(const char*) may only be used with T = const char"); }

		memrun() : _beg{ nullptr }, _end{ nullptr } {}

		[[nodiscard]] T* begin() const
		{
			return _beg;
		}

		[[nodiscard]] T* end() const
		{
			return _end;
		}

		[[nodiscard]] size_t len() const
		{
			return _end - _beg;
		}

		[[nodiscard]] T& operator[](size_t i)
		{
			return _beg[i];
		}
	};

	template<typename T>
	struct compressed_memrun
	{
		int64_t _ptr_len_u;

		compressed_memrun(T* beg, T* end) : _ptr_len_u{ (reinterpret_cast<int64_t>(beg) << 16) | (beg - end) } {}

		compressed_memrun(T* beg, uint16_t len) : _ptr_len_u{ (reinterpret_cast<int64_t>(beg) << 16) | len } {}

		compressed_memrun(const memrun<T>& run) : _ptr_len_u{ (reinterpret_cast<int64_t>(run._beg) << 16) | run._end - run._beg } {}

		compressed_memrun(memrun<T>&& run) : _ptr_len_u{ (reinterpret_cast<int64_t>(run._beg) << 16) | run._end - run._beg } {}

		//compressed_memrun(const char* cstr) : _ptr_len_u{ (reinterpret_cast<int64_t>(cstr) << 16) | _const_strlen_u16(cstr) } { static_assert(std::is_same<const char, T>::value, "och::compressed_memrun<T>(const char*) may only be used with T = const char"); }

		[[nodiscard]] T* begin() const
		{
			return reinterpret_cast<T*>(_ptr_len_u >> 16);
		}

		[[nodiscard]] T* end() const
		{
			return reinterpret_cast<T*>(_ptr_len_u >> 16) + static_cast<uint16_t>(_ptr_len_u);
		}

		[[nodiscard]] uint16_t len() const
		{
			return static_cast<uint16_t>(_ptr_len_u);
		}

		[[nodiscard]] och::memrun<T> uncompress() const
		{
			return och::memrun(begin(), end());
		}
	};

	using string = memrun<const char>;

	using ministring = compressed_memrun<const char>;
}
