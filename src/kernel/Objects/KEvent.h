#pragma once

#include "kernel/Objects/KSignal.h"
#include "kernel/Objects/KRef.h"

class KEvent : public KSignal
{
public:
	constexpr KEvent(const bool manualReset, const bool initialState) :
		KSignal(KSignalType::Event),
		Refs()
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

	KRef Refs; //A user event's handle plus each thread waiting on it; the event is freed at the last (KeEventUnref)
};
