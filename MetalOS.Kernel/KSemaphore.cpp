#include "KSemaphore.h"
#include "x64_support.h"

KSemaphore::KSemaphore(const uint32_t initial, const uint32_t maximum, const std::string& name) :
	m_value(initial), m_limit(maximum), m_name(name)
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
		x64_pause();
		flags = m_spinlock.Acquire();

		loop = m_value >= count;
	}

	m_value -= count;
	m_spinlock.Release(flags);

	return true;
}

void KSemaphore::Signal(size_t count)
{
	//Acquire semaphore
	cpu_flags_t flags = m_spinlock.Acquire();

	m_value += count;
	m_spinlock.Release(flags);
}
