#pragma once

#include "Assert.h"

enum class UObjectType
{
	Pipe,
	File,
	Debug,
	Process,
	Semaphore,
	Event
};

class UObject
{
public:
	UObject(const UObjectType type) :
		Type(type)
	{

	}

	virtual bool IsWriteable() const
	{
		return false;
	}

	virtual bool IsReadable() const
	{
		return false;
	}

	virtual void Display() const
	{
		Printf("UObject::Display - 0x%016x\n", this);
		Printf("  Type: %d\n", Type);
		Printf("     W: %d\n", IsWriteable());
		Printf("     R: %d\n", IsReadable());
	}

	const UObjectType Type;
};
