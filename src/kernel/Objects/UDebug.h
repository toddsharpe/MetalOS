#pragma once

#include "Kernel/Objects/UObject.h"


//Simple object to pipe stdout to kernel debug stream
class UDebug : public UObject
{
public:
	UDebug() :
		UObject(UObjectType::Debug)
	{

	}

	virtual bool IsWriteable() const
	{
		return true;
	}
};
