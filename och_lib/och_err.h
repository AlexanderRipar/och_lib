#define OCH_ERR_PRESENT

#ifndef OCH_ERR_INCLUDE_GUARD
#define OCH_ERR_INCLUDE_GUARD

#include <cstdint>

#include "och_utf8.h"

#define CONSTEXPR_LINE_NUM_CAT_HELPER2(x, y) x##y

#define CONSTEXPR_LINE_NUM_CAT_HELPER(x, y) CONSTEXPR_LINE_NUM_CAT_HELPER2(x, y)

#define CONSTEXPR_LINE_NUM CONSTEXPR_LINE_NUM_CAT_HELPER(__LINE__, u)

namespace och
{
#if defined(OCH_ERROR_CONTEXT_EXTENDED)

#pragma detect_mismatch("och_error_context", "extended")

	struct error_context
	{
		const char* file;
		const uint32_t line;
	};

	#define make_error_context(line_content) och::error_context(__FILE__, CONSTEXPR_LINE_NUM)

#elif defined(OCH_ERROR_CONTEXT_NONE)

#pragma detect_mismatch("och_error_context", "none")

	struct error_context {};

	#define make_error_context(line_content) och::error_context()

#else

#pragma detect_mismatch("och_error_context", "normal")

	struct error_context
	{
		const char* file;
		const char* function;
		const char* line_content;
		uint32_t line;
	};

	#define make_error_context(arg) och::error_context(__FILE__, __FUNCTION__, #arg, CONSTEXPR_LINE_NUM)

#endif // defined(OCH_ERROR_CONTEXT_[EXTENDED, NONE, NORMAL])

#define check(arg) if(och::status s = och::to_status(arg, make_error_context(arg)); s) return s;

#define check_msg(arg, msg) if(och::status s = och::to_status(arg, make_error_context(arg), msg) return s;

#define make_status(arg) och::to_status(arg, make_error_context(arg))

#define make_status_msg(arg, msg) och::to_status(arg, make_error_context(arg), msg)

#define ignore_status(s) if(s) och::err::reset_status();

	enum class error_type
	{
		NONE = 0,
		och = 1,
		hresult = 2,
		vkresult = 3,
	};

	namespace err
	{
		__declspec(noinline) void push_error(const error_context& ctx, uint64_t native_error_code, error_type native_error_source) noexcept;

		__declspec(noinline) void push_error(const error_context& ctx, uint64_t native_error_code, error_type native_error_source, const char* custom_message) noexcept;

		void reset_status() noexcept;

		uint64_t get_native_error_code() noexcept;

		error_type get_error_type() noexcept;

		uint32_t get_stack_depth() noexcept;

		const error_context& get_error_context(uint32_t idx) noexcept;

		const char* get_error_message() noexcept;

		void set_error_message(const char* msg) noexcept;

		och::utf8_string get_error_description() noexcept;
	}

	struct status
	{
	private:

		bool m_is_bad;
		
	public:

		operator bool() const noexcept
		{
			return m_is_bad;
		}

		__forceinline status() noexcept : m_is_bad{ false } {}

		__forceinline explicit status(bool is_bad) noexcept : m_is_bad{ is_bad } {}
	};

	__forceinline status to_status(status rst, const error_context& ctx) noexcept
	{
		if (rst)
			err::push_error(ctx, 1ull, error_type::och);

		return rst;
	}

	__forceinline status to_status(status rst, const error_context& ctx, const char* custom_message) noexcept
	{
		if (rst)
			err::push_error(ctx, 1ull, error_type::och, custom_message);

		return rst;
	}
}

#endif // !OCH_ERR_INCLUDE_GUARD

#if !defined(OCH_ERR_WINDOWS_INTEROP_INCLUDE_GUARD) && defined(_WINDOWS_)
#define OCH_ERR_WINDOWS_INTEROP_INCLUDE_GUARD
namespace och
{
	__forceinline status to_status(HRESULT rst, const error_context& ctx) noexcept
	{
		if (rst != S_OK)
			err::push_error(ctx, static_cast<uint64_t>(static_cast<uint32_t>(rst)), error_type::hresult);

		return status(true);
	}

	__forceinline status to_status(HRESULT rst, const error_context& ctx, const char* custom_message) noexcept
	{
		if (rst != S_OK)
			err::push_error(ctx, static_cast<uint64_t>(static_cast<uint32_t>(rst)), error_type::hresult, custom_message);

		return status(true);
	}
}
#endif // !defined(OCH_ERR_WINDOWS_INTEROP_INCLUDE_GUARD) && defined(_WINDOWS_)

#if !defined(OCH_ERR_VULKAN_INTEROP_INCLUDE_GUARD) && defined(OCH_USING_VULKAN) && (defined(VULKAN_CORE_H_) || defined(VULKAN_H_))
namespace och
{
	__forceinline status to_status(VkResult rst, const error_context& ctx) noexcept
	{
		if (rst != S_OK)
			err::push_error(ctx, static_cast<uint64_t>(static_cast<uint32_t>(rst)), error_type::vkresult);

		return status(true);
	}

	__forceinline status to_status(VkResult rst, const error_context& ctx, const char* custom_message) noexcept
	{
		if (rst != S_OK)
			err::push_error(ctx, static_cast<uint64_t>(static_cast<uint32_t>(rst)), error_type::vkresult, custom_message);

		return status(true);
	}
}
#endif // !defined(OCH_ERR_VULKAN_INTEROP_INCLUDE_GUARD) && defined(OCH_USING_VULKAN) && (defined(VULKAN_CORE_H_) || defined(VULKAN_H_))
