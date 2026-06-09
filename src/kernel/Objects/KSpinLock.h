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
		while (_InterlockedCompareExchange64(&m_value, Locked, Unlocked) == Locked)
			_pause();
		return flags;
	}

	void Release(const cpu_flags_t flags)
	{
		_InterlockedExchange64(&m_value, Unlocked);
		ArchRestoreFlags(flags);
	}

private:
	static const int Locked = 1;
	static const int Unlocked = 0;

	volatile int64_t m_value;
};

class KSpinLockGuard
{
public:
	KSpinLockGuard(KSpinLock& lock) :
		m_lock(lock),
		m_flags(m_lock.Acquire())
	{}

	~KSpinLockGuard()
	{
		m_lock.Release(m_flags);
	}

	KSpinLockGuard(const KSpinLockGuard&) = delete;
	KSpinLockGuard& operator=(const KSpinLockGuard&) = delete;

private:
	KSpinLock& m_lock;
	cpu_flags_t m_flags;
};
