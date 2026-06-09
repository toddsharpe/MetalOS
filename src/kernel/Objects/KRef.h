#pragma once

#include "Assert.h"
#include "kernel/Objects/KSpinLock.h"
extern void PrintStack();

class KRef
{
public:
	constexpr KRef() : m_count(), m_lock()
	{

	}

	void Increment()
	{
		KSpinLockGuard guard(m_lock);
		m_count++;
	}

	void Decrement()
	{
		KSpinLockGuard guard(m_lock);
		Assert(m_count > 0);
		m_count--;
	}

	bool IsEmpty() const
	{
		KSpinLockGuard guard(m_lock);
		return m_count == 0;
	}

	size_t Count() const
	{
		KSpinLockGuard guard(m_lock);
		return m_count;
	}

private:
	size_t m_count;
	mutable KSpinLock m_lock;
};
