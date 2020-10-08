#pragma once

#include <chrono>

namespace och
{
	struct timer
	{
	private:
		std::chrono::steady_clock::time_point beg = std::chrono::steady_clock::now();

	public:
		uint64_t ms()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
		}

		uint64_t us()
		{
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - beg).count();
		}

		void restart()
		{
			beg = std::chrono::steady_clock::now();
		}
	};
}