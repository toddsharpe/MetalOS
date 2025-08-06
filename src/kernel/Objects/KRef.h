#pragma once

#include "Assert.h"
extern void PrintStack();

class KRef
{
public:
	constexpr KRef() : m_count()
	{

	}

	void Increment()
	{
		m_count++;
	}

	void Decrement()
	{
		Assert(m_count > 0);
		m_count--;
	}

	bool IsEmpty() const
	{
		return m_count == 0;
	}

	size_t Count() const
	{
		return m_count;
	}

private:
	size_t m_count;
};
