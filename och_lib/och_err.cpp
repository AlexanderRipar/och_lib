#include "och_err.h"

#include <Windows.h>

#ifndef OCH_ERROR_CONTEXT_NONE

struct error_data_t
{
	static constexpr uint32_t MAX_CALLSTACK_DEPTH = 32;

	och::error_context m_callstack[MAX_CALLSTACK_DEPTH];

	uint32_t m_curr_callstack_idx;

	och::error m_error_code = och::error::success;

	och::error_source_api m_source_api = och::error_source_api::NONE;

	uint32_t m_native_error_code;

	void start(och::error e, och::error_source_api source, uint32_t native_error_code, const och::error_context& ctx) noexcept
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
	error_source_api get_native_error_type() noexcept
	{
		return error_data.m_source_api;
	}

	uint32_t get_native_error_code() noexcept
	{
		return error_data.m_native_error_code;
	}



	__declspec(noinline) void register_error(error e, error_source_api source, uint32_t native_error_code, const error_context& ctx) noexcept
	{
		if (source != error_source_api::NONE)
			error_data.start(e, source, native_error_code, ctx);
		else
			error_data.add(ctx);
	}

	__declspec(noinline) error to_error(HRESULT e) noexcept
	{
		switch (e)
		{
		default:
			break;
		}

		return error::other;
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
