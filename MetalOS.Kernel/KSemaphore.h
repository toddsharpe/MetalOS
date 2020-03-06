#pragma once
#include <cstdint>
#include "KSpinLock.h"

class KSemaphore
{
public:
	KSemaphore(const uint32_t initial, const uint32_t maximum, const std::string& name);

	bool Wait(size_t count, size_t timeout);
	void Signal(size_t count);

private:
	size_t m_value;
	size_t m_limit;
	KSpinLock m_spinlock;
	std::string m_name;
};

