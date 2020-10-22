#pragma once

#include <cstdint>

namespace och
{
	uint32_t initialize_sockets() noexcept;

	void test_net(const char* addr, const char* port);
}
