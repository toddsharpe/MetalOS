#pragma once
#include <cstdint>

#include "KSpinLock.h"
#include "KObject.h"

class KSemaphore : public KObject
{
public:
	KSemaphore(const uint32_t initial, const uint32_t maximum, const std::string& name);

	bool Wait(size_t count, size_t timeout);
	bool TryWait(size_t count);
	int64_t Signal(size_t count);

	size_t Value()
	{
		return m_value;
	}

	const std::string& GetName() const
	{
		return m_name;
	}

private:
	int64_t m_value;
	size_t m_limit;
	KSpinLock m_spinlock;
	std::string m_name;
};

