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
		constexpr range(T(&arr)[Len]) noexcept : beg{ arr }, end{ arr + Len } {}

		constexpr range(T* beg, T* end) noexcept : beg{ beg }, end{ end } {}

		constexpr range(T* beg, size_t len) noexcept : beg{ beg }, end{ beg + len } {}
		
		constexpr range() = default;

		[[nodiscard]] size_t len() const noexcept { return end - beg; }

		[[nodiscard]] size_t bytes() const noexcept { return end - beg * sizeof(T); }

		[[nodiscard]] T& operator[](size_t i) noexcept { return beg[i]; }

		[[nodiscard]] const T& operator[](size_t i) const noexcept { return beg[i]; }
	};

	template<typename T>
	struct compressed_range
	{
		int64_t _ptr_len_u;

		compressed_range(T* beg, T* end) noexcept : _ptr_len_u{ (reinterpret_cast<int64_t>(beg) << 16) | (beg - end) } {}

		compressed_range(T* beg, uint16_t len) noexcept : _ptr_len_u{ (reinterpret_cast<int64_t>(beg) << 16) | len } {}

		compressed_range(const range<T>& run) noexcept : _ptr_len_u{ (reinterpret_cast<int64_t>(run.beg) << 16) | run.end - run.beg } {}

		constexpr compressed_range() noexcept = default;

		[[nodiscard]] T* begin() const noexcept
		{
			return reinterpret_cast<T*>(_ptr_len_u >> 16);
		}

		[[nodiscard]] T* end() const noexcept
		{
			return reinterpret_cast<T*>(_ptr_len_u >> 16) + static_cast<uint16_t>(_ptr_len_u);
		}

		[[nodiscard]] uint16_t len() const noexcept
		{
			return static_cast<uint16_t>(_ptr_len_u);
		}

		[[nodiscard]] och::range<T> uncompress() const noexcept
		{
			return och::range<T>(begin(), end());
		}
	};

	template<typename T> [[nodiscard]] T* begin(och::range<T>& r) noexcept { return r.beg; }
	template<typename T> [[nodiscard]] T* end(och::range<T>& r) noexcept { return r.end; }
	template<typename T> [[nodiscard]] const T* begin(const och::range<T>& r) noexcept { return r.beg; }
	template<typename T> [[nodiscard]] const T* end(const och::range<T>& r) noexcept { return r.end; }
}
