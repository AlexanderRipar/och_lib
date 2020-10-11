#pragma once

#include <cstdint>
#include "och_constexpr_util.h"

namespace och
{
	void rtv_init(const char* config_filename = "rtv_configfile.txt");

	void rtv_update();

	void rtv_exit();

#ifdef OCH_RTV

	namespace internal
	{
		void _register_f32(float* ptr, const char* name);
		void _register_f64(double* ptr, const char* name);
		void _register_u8(uint8_t* ptr, const char* name);
		void _register_u16(uint16_t* ptr, const char* name);
		void _register_u32(uint32_t* ptr, const char* name);
		void _register_u64(uint64_t* ptr, const char* name);
		void _register_i8(int8_t* ptr, const char* name);
		void _register_i16(int16_t* ptr, const char* name);
		void _register_i32(int32_t* ptr, const char* name);
		void _register_i64(int64_t* ptr, const char* name);

		template<typename T>
		void _register_rtv(T* ptr, const char* name)
		{
			if constexpr (och::cxp::is_same_t<float, T>::value) _register_f32(ptr, name);
			else if constexpr (och::cxp::is_same_t<double, T>::value) _register_f64(ptr, name);
			else if constexpr (och::cxp::is_same_t<uint8_t, T>::value) _register_u8(ptr, name);
			else if constexpr (och::cxp::is_same_t<uint16_t, T>::value) _register_u16(ptr, name);
			else if constexpr (och::cxp::is_same_t<uint32_t, T>::value) _register_u32(ptr, name);
			else if constexpr (och::cxp::is_same_t<uint64_t, T>::value) _register_u64(ptr, name);
			else if constexpr (och::cxp::is_same_t<int8_t, T>::value) _register_i8(ptr, name);
			else if constexpr (och::cxp::is_same_t<int16_t, T>::value) _register_i16(ptr, name);
			else if constexpr (och::cxp::is_same_t<int32_t, T>::value) _register_i32(ptr, name);
			else if constexpr (och::cxp::is_same_t<int64_t, T>::value) _register_i64(ptr, name);
			else static_assert(false, "Invalid type passed to OCH_MAKE_HOT");
		}
	}

#define OCH_MAKE_HOT(arg) och::internal::_register_rtv(&arg, #arg);

#else

#define OCH_MAKE_HOT(arg)

#endif // OCH_RTV

}
