#include "och_err.h"

#include <Windows.h>

#if defined(OCH_USING_VULKAN)
#include <vulkan/vulkan.h>
#endif // defined(OCH_USING_VULKAN)

struct
{
	static constexpr size_t MAX_CONTEXT_STACK_DEPTH = 16;

	static constexpr size_t MAX_CUSTOM_MESSAGE_CHARS = 255;

	och::error_context m_context_stack[MAX_CONTEXT_STACK_DEPTH];

	uint32_t m_context_stack_depth = 0;

	och::error_type m_native_error_source = och::error_type::NONE;

	uint64_t m_native_error_code = 0;

	char m_custom_message[MAX_CUSTOM_MESSAGE_CHARS + 1]{};
} 
thread_local error_data;



__declspec(noinline) void och::err::push_error(const error_context& ctx, uint64_t native_error_code, error_type native_error_source) noexcept
{
	push_error(ctx, native_error_code, native_error_source, nullptr);
}

__declspec(noinline) void och::err::push_error(const error_context& ctx, uint64_t native_error_code, error_type native_error_source, const char* custom_message) noexcept
{
	if (!error_data.m_context_stack_depth)
	{
		error_data.m_context_stack[0] = ctx;

		error_data.m_context_stack_depth = 1;

		error_data.m_native_error_code = native_error_code;

		error_data.m_native_error_source = native_error_source;

		set_error_message(custom_message);
	}
	else if (error_data.m_context_stack_depth < error_data.MAX_CONTEXT_STACK_DEPTH)
		error_data.m_context_stack[error_data.m_context_stack_depth++] = ctx;
}



void och::err::reset_status() noexcept
{
	error_data.m_context_stack_depth = 0;

	error_data.m_custom_message[0] = '\0';

	error_data.m_native_error_code = 0;

	error_data.m_native_error_source = error_type::NONE;
}

uint64_t och::err::get_native_error_code() noexcept
{
	return error_data.m_native_error_code;
}

och::error_type och::err::get_error_type() noexcept
{
	return error_data.m_native_error_source;
}

uint32_t och::err::get_stack_depth() noexcept
{
	return error_data.m_context_stack_depth;
}

const och::error_context& och::err::get_error_context(uint32_t idx) noexcept
{
	return error_data.m_context_stack[idx];
}
const char* och::err::get_error_message() noexcept
{
	return error_data.m_custom_message;
}

void och::err::set_error_message(const char* msg) noexcept
{
	if (msg)
	{
		size_t i = 0;

		for (; i != error_data.MAX_CUSTOM_MESSAGE_CHARS && msg[i]; ++i)
			error_data.m_custom_message[i] = msg[i];

		error_data.m_custom_message[i] = '\0';
	}
	else
		error_data.m_custom_message[0] = '\0';
}

och::utf8_string och::err::get_error_description() noexcept
{
	switch (error_data.m_native_error_source)
	{
	case och::error_type::NONE:
		return och::utf8_string("[[No error]]");

	case och::error_type::och:
		break;

	case och::error_type::hresult:
	{
		char* message_buffer;

		if (!FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error_data.m_native_error_code & 0xFFFF, SUBLANG_NEUTRAL, reinterpret_cast<LPSTR>(&message_buffer), 0, nullptr))
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
