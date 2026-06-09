#pragma once

#include "kernel/Objects/KSignal.h"

class KSemaphore : public KSignal
{
public:
	constexpr KSemaphore(const int initial, const int max) :
		KSignal(KSignalType::Semaphore)
		{
			Semaphore.Value = initial;
			Semaphore.Limit = max;
		}

	void Signal()
	{
		Semaphore.Value++;
	}
};
