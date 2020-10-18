#pragma once

#include <intrin.h>

namespace och
{
	template <typename T>
	struct page
	{
		union
		{
			      T dat[4096 / sizeof(      T)] alignas(4096);
			__m128i xmm[4096 / sizeof(__m128i)] alignas(4096);
			__m256i ymm[4096 / sizeof(__m256i)] alignas(4096);
			__m512i zmm[4096 / sizeof(__m512i)] alignas(4096);
		};
	};

	template<>
	struct page<float>
	{
		union
		{
			float dat[4096 / sizeof(float)] alignas(4096);
			__m128 xmm[4096 / sizeof(__m128)] alignas(4096);
			__m256 ymm[4096 / sizeof(__m256)] alignas(4096);
			__m512 zmm[4096 / sizeof(__m512)] alignas(4096);
		};
	};

	template<>
	struct page<double>
	{
		union
		{
			double dat[4096 / sizeof(double)] alignas(4096);
			__m128 xmm[4096 / sizeof(__m128)] alignas(4096);
			__m256 ymm[4096 / sizeof(__m256)] alignas(4096);
			__m512 zmm[4096 / sizeof(__m512)] alignas(4096);
		};
	};
}