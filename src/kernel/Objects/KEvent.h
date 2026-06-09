#pragma once

#include "kernel/Objects/KSignal.h"

class KEvent : public KSignal
{
public:
	constexpr KEvent(const bool manualReset, const bool initialState) :
		KSignal(KSignalType::Event)
		{
			Event.ManualReset = manualReset;
			Event.State = initialState;
		}

	void Set()
	{
		Event.State = true;
	}

	void Reset()
	{
		Event.State = false;
	}
};
