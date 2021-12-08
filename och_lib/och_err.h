#define OCH_ERR_PRESENT

#ifndef OCH_ERR_INCLUDE_GUARD
#define OCH_ERR_INCLUDE_GUARD

#include <cstdint>

#include "och_utf8.h"
#include "och_range.h"

#if defined(OCH_ERROR_CONTEXT_EXTENDED)

#pragma detect_mismatch("och_error_context", "extended")

#endif

#if defined(OCH_ERROR_CONTEXT_NONE)

#pragma detect_mismatch("och_error_context", "none")

#endif

#if defined(OCH_ERROR_CONTEXT_NORMAL)

#pragma detect_mismatch("och_error_context", "normal")

#endif



#if (defined(OCH_ERROR_CONTEXT_EXTENDED) && (defined(OCH_ERROR_CONTEXT_NONE) || defined(OCH_ERROR_CONTEXT_NORMAL))) || (defined(OCH_ERROR_CONTEXT_NORMAL) && defined(OCH_ERROR_CONTEXT_NONE))

#pragma message("More than one of OCH_ERROR_CONTEXT_EXTENDED, OCH_ERROR_CONTEXT_NONE and OCH_ERROR_CONTEXT_NORMAL are defined. Falling back to OCH_ERROR_CONTEXT_NORMAL behaviour")

#undef OCH_ERROR_CONTEXT_EXTENDED
#undef OCH_ERROR_CONTEXT_NONE
#undef OCH_ERROR_CONTEXT_NORMAL

#define OCH_ERROR_CONTEXT_NORMAL

#endif // (defined(OCH_ERROR_CONTEXT_EXTENDED) && (defined(OCH_ERROR_CONTEXT_NONE) || defined(OCH_ERROR_CONTEXT_NORMAL))) || (defined(OCH_ERROR_CONTEXT_NORMAL) && defined(OCH_ERROR_CONTEXT_NONE))



#define CONSTEXPR_LINE_NUM_CAT_HELPER2(x, y) x##y

#define CONSTEXPR_LINE_NUM_CAT_HELPER(x, y) CONSTEXPR_LINE_NUM_CAT_HELPER2(x, y)

#define CONSTEXPR_LINE_NUM CONSTEXPR_LINE_NUM_CAT_HELPER(__LINE__, u)



namespace och
{
	struct error_context
	{
#if defined(OCH_ERROR_CONTEXT_EXTENDED)

	private:
		const char* m_filename;
		const char* m_function;
		const char* m_line_content;
		uint32_t m_line_number;

	public:

		explicit error_context() noexcept = default;

		explicit error_context(const char* filename, const char* function, const char* line_content, uint32_t line_number) noexcept
			: m_filename{ filename }, m_function{ function }, m_line_content{ line_content }, m_line_number{ line_number } {}

#elif defined(OCH_ERROR_CONTEXT_NONE)

	public:

		explicit error_context() noexcept = default;

#else // defined(OCH_ERROR_CONTEXT_NORMAL)

	private:
		const char* m_filename;
		uint32_t m_line_number;

	public:

		explicit error_context() noexcept = default;

		explicit error_context(const char* filename, uint32_t line_number) noexcept
			: m_filename{ filename }, m_line_number{ line_number } {}

#endif // defined(OCH_ERROR_CONTEXT_[EXTENDED|NONE|NORMAL])

		const char* filepath() const noexcept;

		const char* filename() const noexcept;

		const char* function() const noexcept;

		const char* line_content() const noexcept;

		uint32_t line_number() const noexcept;
	};

#if defined(OCH_ERROR_CONTEXT_EXTENDED)

#define make_error_context(arg) och::error_context(__FILE__, __FUNCTION__, #arg, CONSTEXPR_LINE_NUM)

#elif defined(OCH_ERROR_CONTEXT_NONE)

#define make_error_context(line_content) och::error_context()

#else

#define make_error_context(line_content) och::error_context(__FILE__, CONSTEXPR_LINE_NUM)

#endif

#define check(arg) do { if(och::status s = och::register_status(och::to_status(arg), make_error_context(arg))) return s; } while (false)

#define check_msg(arg, msg) do { if(och::status s = och::register_status(och::to_status(arg), maker_error_context(arg), msg)) return s; } while (false)

#define error(arg) do { return och::register_status(och::to_status(arg), make_error_context(arg)); } while (false)

#define error_msg(arg, msg) do { return och::register_status(och::to_status(arg), make_error_context(arg), msg); } while (false)

#define msg_error(msg) static_assert(false, "The msg_error is no longer supported.")

#define ignore_status(s) if(s) och::err::reset_status();

	enum class error_type
	{
		NONE = 0,
		och = 1,
		hresult = 2,
		vkresult = 3,
	};

	struct status
	{
	private:

		uint64_t m_errcode;

	public:

		operator bool() const noexcept { return static_cast<uint32_t>(m_errcode); }

		__forceinline status() noexcept : m_errcode{ 0ull } {}

		__forceinline explicit status(uint64_t errcode) noexcept : m_errcode{ errcode } {}



		uint32_t errcode() const noexcept;

		och::error_type errtype() const noexcept;

		och::utf8_view errtype_name() const noexcept;

		och::utf8_string description() const noexcept;
	};

	namespace err
	{
		__declspec(noinline) void push_error(const error_context& ctx) noexcept;

		void reset_status() noexcept;

		och::range<const error_context> get_callstack() noexcept;

		och::utf8_view get_error_message() noexcept;

		void set_error_message(const char* msg) noexcept;

	}

	__forceinline status to_status() noexcept
	{
		return status(1ull | (static_cast<uint64_t>(error_type::och) << 32));
	}

	__forceinline status to_status(status rst) noexcept
	{
		return rst;
	}

	__forceinline status register_status(status s, const och::error_context& ctx) noexcept
	{
#if !defined(OCH_ERROR_CONTEXT_NONE)
		if (s)
			err::push_error(ctx);
#else
		ctx;
#endif
		return s;
	}

	__forceinline status register_status(status s, const och::error_context& ctx, const char* message) noexcept
	{
		if (s)
		{
#if !defined(OCH_ERROR_CONTEXT_NONE)
			err::push_error(ctx);
#else
			ctx;
#endif
			err::set_error_message(message);
		}

		return s;
	}
}

#endif // !OCH_ERR_INCLUDE_GUARD

#if !defined(OCH_ERR_WINDOWS_INTEROP_INCLUDE_GUARD) && defined(_WINDOWS_)
#define OCH_ERR_WINDOWS_INTEROP_INCLUDE_GUARD
namespace och
{
	__forceinline status to_status(HRESULT rst) noexcept
	{
		return status(static_cast<uint32_t>(rst) | (static_cast<uint64_t>(error_type::hresult) << 32));
	}
}
#endif // !defined(OCH_ERR_WINDOWS_INTEROP_INCLUDE_GUARD) && defined(_WINDOWS_)

#if !defined(OCH_ERR_VULKAN_INTEROP_INCLUDE_GUARD) && defined(OCH_USING_VULKAN) && (defined(VULKAN_CORE_H_) || defined(VULKAN_H_))
#define OCH_ERR_VULKAN_INTEROP_INCLUDE_GUARD
namespace och
{
	__forceinline status to_status(VkResult rst) noexcept
	{
		return status(static_cast<uint32_t>(rst) | (static_cast<uint64_t>(error_type::vkresult) << 32));
	}
}
#endif // !defined(OCH_ERR_VULKAN_INTEROP_INCLUDE_GUARD) && defined(OCH_USING_VULKAN) && (defined(VULKAN_CORE_H_) || defined(VULKAN_H_))
