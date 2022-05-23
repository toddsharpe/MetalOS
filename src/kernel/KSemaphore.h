#pragma once


#include <cstdint>
#include "KSpinLock.h"
#include "KSignalObject.h"

//Count decremented each time thread completes a wait
//Incremented each time thread releases
//When coult is 0, threads block
class KSemaphore : public KSignalObject
{
public:
	KSemaphore(const uint32_t initial, const uint32_t maximum, const std::string& name);

	bool BlockWait(const size_t count);
	int64_t Wait(const size_t count);
	int64_t Signal(const size_t count);

	const std::string& GetName() const
	{
		return m_name;
	}

	virtual bool IsSignalled() const override;

private:
	int64_t m_value;
	size_t m_limit;
	KSpinLock m_spinlock;
	std::string m_name;
};

