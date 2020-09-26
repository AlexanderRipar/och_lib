#pragma once

#include <cstdint>

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

		memrun(T* beg, T* end) : _beg{ beg }, _end{ end } {}

		memrun(T* beg, size_t len) : _beg{ beg }, _end{ beg + len } {}

		T* begin() const
		{
			return _beg;
		}

		T* end() const
		{
			return _end;
		}

		size_t len() const
		{
			return _end - _beg;
		}
	};

	template<typename T>
	struct compressed_memrun
	{
		int64_t _ptr_len_u;

		compressed_memrun(T* beg, uint16_t len) : _ptr_len_u{ (reinterpret_cast<int64_t>(beg) << 16) | len } {}

		compressed_memrun(const memrun<T>& run) : _ptr_len_u{ (reinterpret_cast<int64_t>(run._beg) << 16) | run._end - run._beg } {}

		compressed_memrun(memrun<T>&& run) : _ptr_len_u{ (reinterpret_cast<int64_t>(run._beg) << 16) | run._end - run._beg } {}

		T* begin() const
		{
			return reinterpret_cast<T*>(_ptr_len_u >> 16);
		}

		T* end() const
		{
			return reinterpret_cast<T*>(_ptr_len_u >> 16) + static_cast<uint16_t>(_ptr_len_u);
		}

		uint16_t len() const
		{
			return static_cast<uint16_t>(_ptr_len_u);
		}

		och::memrun<T> uncompress() const
		{
			return och::memrun(begin(), end());
		}
	};

	using string = memrun<const char>;

	using ministring = compressed_memrun<const char>;
}
