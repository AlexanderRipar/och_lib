#pragma once

#include <cstdint>
#include "och_utf8.h"

namespace och
{
	struct type_union
	{
		union
		{
			uint64_t u64;
			int64_t i64;
			uint32_t u32;
			int32_t i32;
			uint16_t u16;
			int16_t i16;
			uint8_t u8;
			int8_t i8;
			float f32;
			double f64;
			och::utf8_char c;
			const void* ptr;
		};

		type_union(uint64_t u64) : u64{ u64 } {}

		type_union(int64_t i64) : i64{ i64 } {}

		type_union(uint32_t u32) : u32{ u32 } {}

		type_union(int32_t i32) : i32{ i32 } {}

		type_union(uint16_t u16) : u16{ u16 } {}

		type_union(int16_t i16) : i16{ i16 } {}

		type_union(uint8_t u8) : u8{ u8 } {}

		type_union(int8_t i8) : i8{ i8 } {}

		type_union(float f32) : f32{ f32 } {}

		type_union(double f64) : f64{ f64 } {}

		type_union(och::utf8_char c) : c{ c } {}

		type_union(const void* ptr) : ptr{ ptr } {}

		type_union() = default;
	};

	static_assert(sizeof(type_union) == sizeof(uint64_t));
}
