#pragma once


#include "KSignalObject.h"

#include "Assert.h"
#include <cstdint>
#include <string>

//Count decremented each time thread completes a wait
//Incremented each time thread releases
//When coult is 0, threads block
class KSemaphore : public KSignalObject
{
public:
	KSemaphore(const int initial, const int maximum, const std::string& name) :
		KSignalObject(),
		Name(name),
		m_value(initial),
		m_limit(maximum)
	{

	}

	int Value() const
	{
		return m_value;
	}

	void Signal()
	{
		m_value++;
	}

	virtual bool IsSignalled() const override
	{
		return m_value > 0;
	}

	virtual void Observed() override
	{
		Assert(IsSignalled());
		m_value--;
	}

	virtual void Display() const override
	{
		Printf("KSemaphore\n");
		Printf("    Value: %d\n", m_value);
		Printf("    Limit: %d\n", m_limit);
	}

	const std::string Name;

private:
	int m_value;
	int m_limit;
};

