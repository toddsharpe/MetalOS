#pragma once

#include "kernel/Objects/KSignal.h"

class KPredicate : public KSignal
{
public:
	constexpr KPredicate(const SignalPredicate fn, void* const arg) :
		KSignal(KSignalType::Predicate)
		{
			Predicate.Fn = fn;
			Predicate.Arg = arg;
		}
};
