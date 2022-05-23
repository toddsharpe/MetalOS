#include "KSemaphore.h"
#include <kernel/MetalOS.Arch.h>

KSemaphore::KSemaphore(const uint32_t initial, const uint32_t maximum, const std::string& name) :
	KSignalObject(KObjectType::Semaphore),
	m_value(initial),
	m_limit(maximum),
	m_spinlock(),
	m_name(name)
{

}

bool KSemaphore::BlockWait(const size_t count)
{
	//Acquire semaphore
	cpu_flags_t flags = m_spinlock.Acquire();

	bool loop = m_value >= count;
	while (!loop)
	{
		m_spinlock.Release(flags);
		ArchPause();
		flags = m_spinlock.Acquire();

		loop = m_value >= count;
	}

	m_value -= count;
	m_spinlock.Release(flags);

	return true;
}

//Returns post-decrement value
int64_t KSemaphore::Wait(const size_t count)
{
	//Acquire semaphore
	cpu_flags_t flags = m_spinlock.Acquire();

	//Decrement
	m_value -= count;
	m_spinlock.Release(flags);

	return m_value;
}

//Returns pre-increment value
int64_t KSemaphore::Signal(const size_t count)
{
	//Acquire semaphore
	cpu_flags_t flags = m_spinlock.Acquire();

	size_t ret = m_value;

	m_value += count;
	m_spinlock.Release(flags);

	return ret;
}

bool KSemaphore::IsSignalled() const
{
	return m_value > 0;
}
