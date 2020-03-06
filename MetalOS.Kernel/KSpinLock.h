#pragma once

#include "MetalOS.Kernel.h"

class KSpinLock
{
public:
	static const int Locked = 1;
	static const int Unlocked = 0;

	KSpinLock();

	cpu_flags_t Acquire();
	void Release(cpu_flags_t flags);

private:
	size_t m_value;
};
