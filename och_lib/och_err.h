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

#define to_status(arg) och::err::as_status(arg, make_error_context(arg))

#define check(arg) do { if(och::status s = to_status(arg)) return s; } while (false)

#define ignore_status(s) if(s) och::err::reset_callstack();

	enum class error_type : uint32_t
	{
		och = 0,
		hresult = 1,
		vkresult = 2,
		errnum = 3,
	};

	enum class error : uint32_t
	{
		ok = 0,
		insufficient_buffer,
		argument_invalid,
		argument_too_large,
		function_unavailable,
		no_memory,
		no_more_data,
		not_found,
	};

	struct status
	{
	private:

		uint64_t m_errcode;

	public:

		operator bool() const noexcept { return static_cast<uint32_t>(m_errcode); }

		__forceinline status() noexcept : m_errcode{ 0ull } {}

		__forceinline explicit status(uint32_t errcode, error_type type) noexcept : m_errcode{ static_cast<uint32_t>(errcode) | (static_cast<uint64_t>(type) << 32) } {}

		__forceinline status(och::error err) noexcept : m_errcode{ static_cast<uint32_t>(err) | static_cast<uint64_t>(error_type::och) << 32 } {}


		uint32_t errcode() const noexcept;

		och::error_type errtype() const noexcept;

		och::utf8_view errtype_name() const noexcept;

		och::utf8_string description() const noexcept;

		bool operator==(const error& rhs) const noexcept
		{
			return m_errcode == static_cast<uint64_t>(rhs);
		}

		bool operator!=(const error& rhs) const noexcept
		{
			return m_errcode != static_cast<uint64_t>(rhs);
		}
	};

	namespace err
	{
		namespace impl
		{
#if !defined(OCH_ERROR_CONTEXT_NONE)
			__declspec(noinline) void push_error_(const error_context& ctx) noexcept;
#endif

			__forceinline void register_status_(status s, const error_context& ctx) noexcept
			{
#if !defined(OCH_ERROR_CONTEXT_NONE)
				if (s)
					err::impl::push_error_(ctx);
#else
				s; ctx; // To suppress unused arguments warning
#endif
			}
		}

		void reset_callstack() noexcept;

		och::range<const error_context> get_callstack() noexcept;

		template<typename T>
		__forceinline status as_status(T rst, const och::error_context& ctx) noexcept
		{
			static_assert(false, "Could not find matching status_from_(T, ctx) specialization.");
		}

		template<>
		__forceinline status as_status(status rst, const och::error_context& ctx) noexcept
		{
			err::impl::register_status_(rst, ctx);

			return rst;
		}

		template<>
		__forceinline status as_status(error rst, const och::error_context& ctx) noexcept
		{
			status s(rst);

			err::impl::register_status_(s, ctx);

			return s;
		}
	}
}

#endif // !OCH_ERR_INCLUDE_GUARD

#if !defined(OCH_ERR_WINDOWS_INTEROP_INCLUDE_GUARD) && defined(_WINDOWS_)
#define OCH_ERR_WINDOWS_INTEROP_INCLUDE_GUARD

#define status_from_lasterr to_status(HRESULT_FROM_WIN32(GetLastError()))

namespace och::err
{
	template<>
	__forceinline status as_status(HRESULT rst, const och::error_context& ctx) noexcept
	{
		status s(rst, error_type::hresult);

		err::impl::register_status_(s, ctx);

		return s;
	}
}
#endif // !defined(OCH_ERR_WINDOWS_INTEROP_INCLUDE_GUARD) && defined(_WINDOWS_)

#if !defined(OCH_ERR_VULKAN_INTEROP_INCLUDE_GUARD) && defined(OCH_USING_VULKAN) && (defined(VULKAN_CORE_H_) || defined(VULKAN_H_))
#define OCH_ERR_VULKAN_INTEROP_INCLUDE_GUARD
namespace och::err
{
	template<>
	__forceinline status as_status(VkResult rst, const och::error_context& ctx) noexcept
	{
		status s(rst, error_type::vkresult);

		err::impl::register_status_(s, ctx);

		return s;
	}
}
#endif // !defined(OCH_ERR_VULKAN_INTEROP_INCLUDE_GUARD) && defined(OCH_USING_VULKAN) && (defined(VULKAN_CORE_H_) || defined(VULKAN_H_))

#if !defined(OCH_ERR_LINUX_INTEROP_INCLUDE_GUARD) && defined(__linux__)
#define OCH_ERR_LINUX_INTEROP_INCLUDE_GUARD
namespace och::err
{
	template<>
	__forceinline status as_status(errno_t rst, const och::error_context& ctx) noexcept
	{
		status s(rst, error_type::errnum);

		err::impl::register_status_(s, ctx);

		return s;
	}
}
#endif // !defined(OCH_ERR_UNIX_INTEROP_INCLUDE_GUARD) && defined(__linux__)
