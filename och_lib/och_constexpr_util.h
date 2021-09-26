#define OCH_CONSTEXPR_UTIL_PRESENT

#ifndef OCH_CONSTEXPR_UTIL_INCLUDE_GUARD
#define OCH_CONSTEXPR_UTIL_INCLUDE_GUARD

#include <cstdint>

namespace och
{
	namespace cxp
	{
		template<typename T> struct h_assert_is_int {                        };
		template<          > struct h_assert_is_int<  int8_t> { using type = int8_t; };
		template<          > struct h_assert_is_int< int16_t> { using type = int16_t; };
		template<          > struct h_assert_is_int< int32_t> { using type = int32_t; };
		template<          > struct h_assert_is_int< int64_t> { using type = int64_t; };
		template<          > struct h_assert_is_int< uint8_t> { using type = uint8_t; };
		template<          > struct h_assert_is_int<uint16_t> { using type = uint16_t; };
		template<          > struct h_assert_is_int<uint32_t> { using type = uint32_t; };
		template<          > struct h_assert_is_int<uint64_t> { using type = uint64_t; };
		template<typename T>  using   assert_is_int = typename h_assert_is_int<T>::type;

		template<typename T> struct h_get_smaller {                        };
		template<          > struct h_get_smaller< int16_t> { using type = int8_t; };
		template<          > struct h_get_smaller< int32_t> { using type = int16_t; };
		template<          > struct h_get_smaller< int64_t> { using type = int32_t; };
		template<          > struct h_get_smaller<uint16_t> { using type = uint8_t; };
		template<          > struct h_get_smaller<uint32_t> { using type = uint16_t; };
		template<          > struct h_get_smaller<uint64_t> { using type = uint32_t; };
		template<typename T>  using   get_smaller = typename h_get_smaller<T>::type;

		template<typename T> struct h_get_larger {                        };
		template<          > struct h_get_larger<  int8_t> { using type = int16_t; };
		template<          > struct h_get_larger< int16_t> { using type = int32_t; };
		template<          > struct h_get_larger< int32_t> { using type = int64_t; };
		template<          > struct h_get_larger< uint8_t> { using type = uint16_t; };
		template<          > struct h_get_larger<uint16_t> { using type = uint32_t; };
		template<          > struct h_get_larger<uint32_t> { using type = uint64_t; };
		template<typename T>  using   get_larger = typename h_get_larger<T>::type;

		template<typename T> struct h_get_sign {                       };
		template<          > struct h_get_sign<  int8_t> { using type = int8_t; };
		template<          > struct h_get_sign< int16_t> { using type = int8_t; };
		template<          > struct h_get_sign< int32_t> { using type = int8_t; };
		template<          > struct h_get_sign< int64_t> { using type = int8_t; };
		template<          > struct h_get_sign< uint8_t> { using type = uint8_t; };
		template<          > struct h_get_sign<uint16_t> { using type = uint8_t; };
		template<          > struct h_get_sign<uint32_t> { using type = uint8_t; };
		template<          > struct h_get_sign<uint64_t> { using type = uint8_t; };
		template<typename T>  using   get_sign = typename h_get_sign<T>::type;

		template<typename Sign, typename Size> struct h_bind_sign_to_size {                        };
		template<                            > struct h_bind_sign_to_size< int8_t, int8_t> { using type = int8_t; };
		template<                            > struct h_bind_sign_to_size< int8_t, uint8_t> { using type = int8_t; };
		template<                            > struct h_bind_sign_to_size< int8_t, int16_t> { using type = int16_t; };
		template<                            > struct h_bind_sign_to_size< int8_t, uint16_t> { using type = int16_t; };
		template<                            > struct h_bind_sign_to_size< int8_t, int32_t> { using type = int32_t; };
		template<                            > struct h_bind_sign_to_size< int8_t, uint32_t> { using type = int32_t; };
		template<                            > struct h_bind_sign_to_size< int8_t, int64_t> { using type = int64_t; };
		template<                            > struct h_bind_sign_to_size< int8_t, uint64_t> { using type = int64_t; };
		template<                            > struct h_bind_sign_to_size<uint8_t, int8_t> { using type = uint8_t; };
		template<                            > struct h_bind_sign_to_size<uint8_t, uint8_t> { using type = uint8_t; };
		template<                            > struct h_bind_sign_to_size<uint8_t, int16_t> { using type = uint16_t; };
		template<                            > struct h_bind_sign_to_size<uint8_t, uint16_t> { using type = uint16_t; };
		template<                            > struct h_bind_sign_to_size<uint8_t, int32_t> { using type = uint32_t; };
		template<                            > struct h_bind_sign_to_size<uint8_t, uint32_t> { using type = uint32_t; };
		template<                            > struct h_bind_sign_to_size<uint8_t, int64_t> { using type = uint64_t; };
		template<                            > struct h_bind_sign_to_size<uint8_t, uint64_t> { using type = uint64_t; };
		template<typename Sign, typename Size>  using   bind_sign_to_size = typename h_bind_sign_to_size<get_sign<Sign>, Size>::type;

		template<typename T> struct h_negate_sign {                       };
		template<          > struct h_negate_sign< int8_t> { using type = uint8_t; };
		template<          > struct h_negate_sign<uint8_t> { using type = int8_t; };
		template<typename T>  using   negate_sign = typename h_negate_sign<get_sign<T>>::type;

		static constexpr bool is_signed(int8_t) { return true; }
		static constexpr bool is_signed(uint8_t) { return false; }

		template<typename T> struct is_int { bool value = false; };
		template<          > struct is_int<  int8_t> { bool value = true; };
		template<          > struct is_int< int16_t> { bool value = true; };
		template<          > struct is_int< int32_t> { bool value = true; };
		template<          > struct is_int< int64_t> { bool value = true; };
		template<          > struct is_int< uint8_t> { bool value = true; };
		template<          > struct is_int<uint16_t> { bool value = true; };
		template<          > struct is_int<uint32_t> { bool value = true; };
		template<          > struct is_int<uint64_t> { bool value = true; };

		template<typename T> struct is_ptr_t { static constexpr bool value = false; };
		template<typename T> struct is_ptr_t<T*               > { static constexpr bool value = true; };
		template<typename T> struct is_ptr_t<T* const         > { static constexpr bool value = true; };
		template<typename T> struct is_ptr_t<T* volatile      > { static constexpr bool value = true; };
		template<typename T> struct is_ptr_t<T* const volatile> { static constexpr bool value = true; };

		template<typename T0, typename T1> struct is_same_t { static constexpr bool value = false; };
		template<typename T0             > struct is_same_t<T0, T0> { static constexpr bool value = true; };
	}
}

#endif // !OCH_CONSTEXPR_UTIL_INCLUDE_GUARD
