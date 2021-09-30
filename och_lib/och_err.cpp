#include "och_err.h"

#include <Windows.h>

#ifndef OCH_ERROR_CONTEXT_NONE

struct error_data_t
{
	static constexpr uint32_t MAX_CALLSTACK_DEPTH = 32;

	och::error_context m_callstack[MAX_CALLSTACK_DEPTH];

	uint32_t m_curr_callstack_idx;

	och::status m_error_code = och::status::ok;

	och::source_error_type m_source_api = och::source_error_type::NONE;

	uint32_t m_native_error_code;

	void start(och::status e, och::source_error_type source, uint32_t native_error_code, const och::error_context& ctx) noexcept
	{
		m_callstack[0] = ctx;

		m_error_code = e;

		m_source_api = source;

		m_native_error_code = native_error_code;

		m_curr_callstack_idx = 1;
	}

	void add(const och::error_context& ctx) noexcept
	{
		if (m_curr_callstack_idx >= MAX_CALLSTACK_DEPTH)
			return;

		m_callstack[m_curr_callstack_idx++] = ctx;
	}
};

thread_local error_data_t error_data;



namespace och::err
{
	source_error_type get_native_error_type() noexcept
	{
		return error_data.m_source_api;
	}

	uint32_t get_native_error_code() noexcept
	{
		return error_data.m_native_error_code;
	}



	__declspec(noinline) void reset_error() noexcept
	{
		error_data.m_curr_callstack_idx = 0;

		error_data.m_error_code = status::ok;
	}

	__declspec(noinline) void register_error(status e, source_error_type source, uint32_t native_error_code, const error_context& ctx) noexcept
	{
		if (source != source_error_type::NONE)
			error_data.start(e, source, native_error_code, ctx);
		else
			error_data.add(ctx);
	}

	__declspec(noinline) status to_error(HRESULT e) noexcept
	{
		switch (e)
		{
		default:
			break;
		}

		return status::other;
	}

	#ifdef OCH_USING_VULKAN

		__declspec(noinline) error to_error(VkResult e) noexcept
		{
			switch (e)
			{
			default:
				break;
			}

			return error::other;
		}

	#endif // OCH_USING_VULKAN
}

#endif // !OCH_ERROR_CONTEXT_NONE
