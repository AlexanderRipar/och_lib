#include "och_err.h"

#include <Windows.h>

#if defined(OCH_USING_VULKAN)
#include <vulkan/vulkan.h>
#endif



const char* och::error_context::filepath() const noexcept
{
#if defined(OCH_ERROR_CONTEXT_EXTENDED) || defined(OCH_ERROR_CONTEXT_NORMAL)
	return m_filename;
#else
	return "[[Unknown]]";
#endif
}

const char* och::error_context::filename() const noexcept
{
#if defined(OCH_ERROR_CONTEXT_EXTENDED) || defined(OCH_ERROR_CONTEXT_NORMAL)
	size_t last_slash_or_backslash = 0;

	for (size_t i = 0; m_filename[i] != '\0'; ++i)
		if (m_filename[i] == '\\' || m_filename[i] == '/')
			last_slash_or_backslash = i + 1;

	return m_filename + last_slash_or_backslash;
#else
	return "[[Unknown]]";
#endif
}

const char* och::error_context::function() const noexcept
{
#if defined(OCH_ERROR_CONTEXT_EXTENDED)
	return m_function;
#else
	return "[[Unknown]]";
#endif
}

const char* och::error_context::line_content() const noexcept
{
#if defined(OCH_ERROR_CONTEXT_EXTENDED)
	return m_line_content;
#else
	return "[[Unknown]]";
#endif
}

uint32_t och::error_context::line_number() const noexcept
{
#if defined(OCH_ERROR_CONTEXT_EXTENDED) || defined(OCH_ERROR_CONTEXT_NORMAL)
	return m_line_number;
#else
	return 0;
#endif
}


#if !defined(OCH_ERROR_CONTEXT_NONE)

struct error_data_t
{
	static constexpr size_t MAX_CUSTOM_MESSAGE_CHARS = 255;

	static constexpr size_t MAX_CONTEXT_STACK_DEPTH = 16;

	och::error_context m_context_stack[MAX_CONTEXT_STACK_DEPTH]{};

	uint32_t m_context_stack_depth = 0;


	char m_custom_message[MAX_CUSTOM_MESSAGE_CHARS + 1]{};
};

thread_local error_data_t error_data;

#endif // !defined(OCH_ERROR_CONTEXT_NONE)



void och::err::reset_callstack() noexcept
{
#if !defined(OCH_ERROR_CONTEXT_NONE)
	error_data.m_context_stack_depth = 0;
#endif
}

och::range<const och::error_context> och::err::get_callstack() noexcept
{
#if !defined(OCH_ERROR_CONTEXT_NONE)
	return och::range<const och::error_context>(error_data.m_context_stack, error_data.m_context_stack_depth);
#else
	return och::range<const och::error_context>(nullptr, nullptr);
#endif
}

#if !defined(OCH_ERROR_CONTEXT_NONE)
__declspec(noinline) void och::err::impl::push_error_(const error_context& ctx) noexcept
{
	if (error_data.m_context_stack_depth < error_data.MAX_CONTEXT_STACK_DEPTH)
		error_data.m_context_stack[error_data.m_context_stack_depth++] = ctx;
}
#endif



uint32_t och::status::errcode() const noexcept { return static_cast<uint32_t>(m_errcode); }

och::error_type och::status::errtype() const noexcept { return static_cast<och::error_type>(m_errcode >> 32); }

och::utf8_view och::status::errtype_name() const noexcept
{
	switch (static_cast<och::error_type>(m_errcode >> 32))
	{
	case error_type::och:
		return "och::status";

	case error_type::hresult:
		return "HRESULT";

	case error_type::vkresult:
		return "VkResult";

	case error_type::errnum:
		return "errnum";
	}

	return "[[Unknown]]";
}

och::utf8_string och::status::description() const noexcept
{
	switch (static_cast<och::error_type>(m_errcode >> 32))
	{
	case och::error_type::och:
		
		switch (static_cast<och::error>(m_errcode))
		{
		case och::error::ok:
			return utf8_string("[[No error]]");

		case och::error::insufficient_buffer:
			return utf8_string("A buffer was too small to hold the given contents");

		case och::error::argument_invalid:
			return utf8_string("An invalid argument was passed to a function");

		case och::error::function_unavailable:
			return utf8_string("The function is not available on this system");

		case och::error::argument_too_large:
			return utf8_string("A function argument was too large");

		case och::error::no_memory:
			return utf8_string("There is not enough memory available to complete the operation");

		case och::error::no_more_data:
			return utf8_string("No more data could be found");

		default:
			break;
		}
		break;

	case och::error_type::hresult:
	{
		char* message_buffer;

		if (!FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK, nullptr, static_cast<DWORD>(m_errcode), SUBLANG_NEUTRAL, reinterpret_cast<LPSTR>(&message_buffer), 0, nullptr))
			break;

		och::utf8_string ret_str(message_buffer);

		LocalFree(reinterpret_cast<HLOCAL>(message_buffer));

		return ret_str;
	}

	case och::error_type::vkresult:
		break;
	}

	return "[[No description]]";
}
