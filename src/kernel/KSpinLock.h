#pragma once

#include "MetalOS.Kernel.h"

class KSpinLock
{
public:
	KSpinLock();

	cpu_flags_t Acquire();
	void Release(cpu_flags_t flags);

private:
	static const int Locked = 1;
	static const int Unlocked = 0;

	size_t m_value;
	
	//Only 64bit exchange instructions are implemented
	static_assert(std::numeric_limits<size_t>::digits == 64);
};
