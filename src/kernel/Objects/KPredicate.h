#pragma once

#include "Kernel/Objects/KSignalObject.h"

typedef bool(*SignalPredicate)(void* const arg);
class KPredicate : public KSignalObject
{
public:
	KPredicate(const SignalPredicate predicate, void* const arg) :
		m_predicate(predicate),
		m_arg(arg)
	{ }

	virtual bool IsSignalled() const override
	{
		return (*m_predicate)(m_arg);
	}

private:
	const SignalPredicate m_predicate;
	void* const m_arg;
};
