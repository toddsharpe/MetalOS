#pragma once

#include "KSignalObject.h"

typedef bool(*SignalPredicate)(void* arg);
class KPredicate : public KSignalObject
{
public:
	KPredicate(SignalPredicate predicate, void* arg);

	virtual bool IsSignalled() const override;
private:
	SignalPredicate m_predicate;
	void* m_arg;
};

