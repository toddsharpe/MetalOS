#include "KSpinLock.h"
#include <MetalOS.Arch.h>

KSpinLock::KSpinLock() : m_value(Unlocked)
{

}

cpu_flags_t KSpinLock::Acquire()
{
	cpu_flags_t flags = ArchDisableInterrupts();

	while (_InterlockedCompareExchange64((volatile long long*)&m_value, Locked, Unlocked) == Locked)
		ArchPause();

	return flags;
}

void KSpinLock::Release(cpu_flags_t flags)
{
	m_value = Unlocked;
	ArchRestoreFlags(flags);
}
