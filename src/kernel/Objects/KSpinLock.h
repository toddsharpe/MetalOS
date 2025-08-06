#pragma once

#include "kernel/Arch_x64/Arch.h"
#include "x64/intrin.h"

class KSpinLock
{
public:
	constexpr KSpinLock() : m_value()
	{

	}

	cpu_flags_t Acquire()
	{
		cpu_flags_t flags = ArchDisableInterrupts();
		while (_InterlockedCompareExchange64((volatile long long*)&m_value, Locked, Unlocked) == Locked)
			_pause();
		return flags;
	}

	void Release(const cpu_flags_t flags)
	{
		_InterlockedExchange64((volatile long long*)&m_value, Unlocked);
		ArchRestoreFlags(flags);
	}

private:
	static const int Locked = 1;
	static const int Unlocked = 0;

	uint64_t m_value;
};
