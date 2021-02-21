#pragma once

#include <cstdint>
#include <cstring>

#include "och_constexpr_util.h"
#include <type_traits>

namespace och
{
	constexpr uint16_t _const_strlen_u16(const char* str)
	{
		uint16_t len = 0;

		while (*(str++))
			++len;

		return len;
	}

	template<typename T>
	struct range
	{
		T* beg, * end;

		template<size_t Len>
		constexpr range(T(&arr)[Len]) : beg{ arr }, end{ arr + Len } {}

		constexpr range(T* beg, T* end) : beg{ beg }, end{ end } {}

		constexpr range(T* beg, size_t len) : beg{ beg }, end{ beg + len } {}
		
		constexpr range() = default;

		template<typename U = T>
		range(std::enable_if_t<std::is_same<U, const char*>::value, const char*> cstr) : beg{ cstr }, end{ cstr + strlen(cstr) } { }

		[[nodiscard]] size_t len() const { return end - beg; }

		[[nodiscard]] size_t bytes() const { return end - beg * sizeof(T); }

		[[nodiscard]] T& operator[](size_t i) { return beg[i]; }

		[[nodiscard]] const T& operator[](size_t i) const { return beg[i]; }
	};

	template<typename T>
	struct compressed_range
	{
		int64_t _ptr_len_u;

		compressed_range(T* beg, T* end) : _ptr_len_u{ (reinterpret_cast<int64_t>(beg) << 16) | (beg - end) } {}

		compressed_range(T* beg, uint16_t len) : _ptr_len_u{ (reinterpret_cast<int64_t>(beg) << 16) | len } {}

		compressed_range(const range<T>& run) : _ptr_len_u{ (reinterpret_cast<int64_t>(run.beg) << 16) | run.end - run.beg } {}

		compressed_range(range<T>&& run) : _ptr_len_u{ (reinterpret_cast<int64_t>(run.beg) << 16) | run.end - run.beg } {}

		constexpr compressed_range() = default;

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

		[[nodiscard]] och::range<T> uncompress() const
		{
			return och::range<T>(begin(), end());
		}
	};

	template<typename T> [[nodiscard]] T* begin(och::range<T>& r) { return r.beg; }
	template<typename T> [[nodiscard]] T* end(och::range<T>& r) { return r.end; }
	template<typename T> [[nodiscard]] const T* begin(const och::range<T>& r) { return r.beg; }
	template<typename T> [[nodiscard]] const T* end(const och::range<T>& r) { return r.end; }

	template<typename T> [[nodiscard]] T* begin(och::compressed_range<T>& r) { return r.begin(); }
	template<typename T> [[nodiscard]] T* end(och::compressed_range<T>& r) { return r.end(); }
	template<typename T> [[nodiscard]] const T* begin(const och::compressed_range<T>& r) { return r.begin(); }
	template<typename T> [[nodiscard]] const T* end(const och::compressed_range<T>& r) { return r.end(); }

	using stringview = range<const char>;

	using mini_stringview = compressed_range<const char>;
}
