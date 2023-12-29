#pragma once

#include "Kernel/Objects/UObject.h"
#include "Kernel/Objects/KSemaphore.h"

class USemaphore : public UObject
{
public:
	USemaphore(const int initial, const int maximum, const std::string& name) :
		UObject(UObjectType::Semaphore),
		Sem(initial, maximum, name)
	{

	}

	virtual void Display() const override
	{
		UObject::Display();
		Printf("  USemaphore\n");
		Printf("    Name: %s\n", Sem.Name.c_str());
		Printf("    Signalled: %d\n", Sem.IsSignalled());
	}

	KSemaphore Sem;
};
