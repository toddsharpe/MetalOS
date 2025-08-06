#pragma once

#include "kernel/Types.h"
#include "Assert.h"

template <typename TIrq, size_t N>
class InterruptTable
{
public:
	InterruptTable() :
		m_table()
	{

	}

	void Add(const TIrq irq, const ActionContext& context)
	{
		//TODO(tsharpe): Assert IRQ slot is empty?
		const size_t i = static_cast<size_t>(irq);
		m_table[i] = context;
	}

	bool Contains(const TIrq irq)
	{
		const size_t i = static_cast<size_t>(irq);
		return !m_table[i].IsEmpty();
	}

	ActionContext operator[](const TIrq index) const
	{
		const size_t i = static_cast<size_t>(index);
		Assert(i < N);
		return m_table[i];
	}

private:
	ActionContext m_table[N];
};
