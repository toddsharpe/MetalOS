#pragma once

#include "Kernel/Objects/UObject.h"
#include "Kernel/Objects/KEvent.h"

#include <memory>

class UEvent : public UObject
{
public:
	UEvent(const bool manualReset, const bool initialState) :
		UObject(UObjectType::Event),
		Event(manualReset, initialState)
	{

	}

	virtual void Display() const override
	{
		UObject::Display();
		Printf("  UEvent\n");
		Printf("    Signalled: %d\n", Event.IsSignalled());
	}

	KEvent Event;
};
