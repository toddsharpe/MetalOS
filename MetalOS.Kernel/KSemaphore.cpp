#include "KSemaphore.h"
#include "Main.h"
#include <MetalOS.Arch.h>

KSemaphore::KSemaphore(const uint32_t initial, const uint32_t maximum, const std::string& name) :
	KObject(),
	m_value(initial),
	m_limit(maximum),
	m_spinlock(),
	m_name(name)
{

}

bool KSemaphore::Wait(size_t count, size_t timeout)
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

bool KSemaphore::TryWait(size_t count)
{
	//Acquire semaphore
	cpu_flags_t flags = m_spinlock.Acquire();

	//Decrement
	m_value -= count;

	if (m_value < 0)
	{
		m_spinlock.Release(flags);
		return false;
	}
	else
	{
		m_spinlock.Release(flags);
		return true;
	}
}

int64_t KSemaphore::Signal(size_t count)
{
	//Acquire semaphore
	cpu_flags_t flags = m_spinlock.Acquire();

	size_t ret = m_value;

	m_value += count;
	m_spinlock.Release(flags);

	return ret;
}
