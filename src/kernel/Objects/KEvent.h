#pragma once

#include "KSignalObject.h"
#include "Assert.h"

class KEvent : public KSignalObject
{
public:
	KEvent(const bool manualReset, const bool initialState) :
		KSignalObject(),
		m_manualReset(manualReset),
		m_state(initialState)
	{

	}

	void Set()
	{
		m_state = true;
	}

	void Reset()
	{
		m_state = false;
	}

	virtual bool IsSignalled() const override
	{
		return m_state;
	}

	virtual void Observed() override
	{
		Assert(IsSignalled());
		if (!m_manualReset)
			m_state = false;
	}

	virtual void Display() const override
	{
		Printf("KEvent\n");
		Printf("    Manual: %d\n", m_manualReset);
		Printf("    State: %d\n", m_state);
	}

private:
	const bool m_manualReset;
	bool m_state;
};
