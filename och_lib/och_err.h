#define OCH_ERR_PRESENT

#ifndef OCH_ERR_INCLUDE_GUARD
#define OCH_ERR_INCLUDE_GUARD

#include <cstdint>

#include "och_utf8.h"
#include "och_range.h"

#if !defined(NDEBUG) && !defined(OCH_ERROR_CONTEXT_NONE) && !defined(OCH_ERROR_CONTEXT_NORMAL)
	#define OCH_ERROR_CONTEXT_EXTENDED
#endif // !defined(NDEBUG) && !defined(OCH_ERROR_CONTEXT_NONE) && !defined(OCH_ERROR_CONTEXT_NORMAL)

#if defined(_WINDOWS_)
	#define OCH_ERR_HRESULT
#endif // defined(_WINDOWS_)

#if (defined(VULKAN_CORE_H_) || defined(VULKAN_H_)) && defined(OCH_USING_VULKAN)
	#define OCH_ERR_VKRESULT
#endif // (defined(VULKAN_CORE_H_) || defined(VULKAN_H_)) && defined(OCH_USING_VULKAN)

namespace och
{
	enum class error_source_api : uint32_t
	{
		NONE             = 0,
		hresult          = 1,
		vkresult         = 2,
	};

	enum class error : uint32_t
	{
		success = 0,
		other = 1,
		permission_denied,
		not_found,
		invalid_argument,
		out_of_bounds
	};

#ifdef OCH_ERROR_CONTEXT_EXTENDED

	#pragma detect_mismatch("och_error_context", "extended")
	
		struct error_context
		{
			const char* file;
			const char* calling_function;
			const char* calling_line_content;
			uint32_t line_number;
		};
	
	#define check(macro_defined_argument) { if (auto macro_defined_result = macro_defined_argument; och::err::is_error(macro_defined_result)) { och::error macro_defined_error = och::err::to_error(macro_defined_result); och::err::register_error(macro_defined_error, och::err::get_error_type(macro_defined_result), static_cast<uint32_t>(macro_defined_result), och::error_context(__FILE__, __FUNCTION__, #macro_defined_argument, __LINE__)); return macro_defined_error; } }

#elif defined(OCH_ERROR_CONTEXT_NONE)

#pragma detect_mismatch("och_error_context", "none")
	
	struct error_context {};
	
	#define check(macro_defined_argument) { if(och::err::is_error(macro_defined_argument)) return och::error::other;

#else

	#pragma detect_mismatch("och_error_context", "normal")

	#if !defined(OCH_ERROR_CONTEXT_NORMAL)
		#pragma warning("None of the OCH_ERROR_CONTEXT_* macros are defined. Defaulting to OCH_ERROR_CONTEXT_NORMAL")
	#endif // !defined(OCH_ERROR_CONTEXT_NORMAL)
	
	struct error_context
	{
		const char* file;
		uint32_t line_number;
	};
	
	#define check(macro_defined_argument) {if (auto macro_defined_result = macro_defined_argument; och::err::is_error(macro_defined_result)) { och::error macro_defined_error = och::err::to_error(macro_defined_result); och::err::register_error(macro_defined_error, och::err::get_error_type(macro_defined_result), static_cast<uint32_t>(macro_defined_result), och::error_context(__FILE__, __LINE__)); return macro_defined_error; } }

#endif // OCH_ERROR_CONTEXT_*


	namespace err
	{
		error_source_api get_native_error_type() noexcept;

		uint32_t get_native_error_code() noexcept;



		__declspec(noinline) void register_error(error e, error_source_api source, uint32_t native_error_code, const error_context& ctx) noexcept;

		__forceinline bool is_error(error e) noexcept
		{
			return e != error::success;
		}

		__forceinline error_source_api get_error_type(error e) noexcept
		{
			e; return error_source_api::NONE;
		}

		__forceinline error to_error(error e) noexcept
		{
			return e;
		}

		#ifdef OCH_ERR_HRESULT

			__forceinline bool is_error(HRESULT e) noexcept
			{
				return e != S_OK;
			}

			__forceinline error_source_api get_error_type(HRESULT e) noexcept
			{
				e; return error_source_api::hresult;
			}

			__declspec(noinline) error to_error(HRESULT e) noexcept;

		#endif // OCH_ERR_HRESULT



		#ifdef OCH_ERR_VKRESULT

			__forceinline bool is_error(VkResult e) noexcept
			{
				return e != VK_SUCCESS;
			}

			__forceinline error_source_api get_error_type(VkResult e) noexcept
			{
				e; return error_source_api::vkresult;
			}

			__declspec(noinline) error to_error(VkResult e) noexcept;

		#endif // OCH_ERR_VKRESULT
	}
}

#endif // !OCH_ERR_INCLUDE_GUARD
