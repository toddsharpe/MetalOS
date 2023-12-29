#pragma once

#include "Kernel/Objects/UObject.h"
#include "Kernel/Objects/KFile.h"

class UFile : public UObject
{
public:
	UFile() :
		UObject(UObjectType::File),
		File()
	{

	}

	virtual bool IsWriteable() const override
	{
		return (File.Access & GenericAccess::Write) == GenericAccess::Write;
	}

	virtual bool IsReadable() const override
	{
		return (File.Access & GenericAccess::Read) == GenericAccess::Read;
	}

	virtual void Display() const override
	{
		UObject::Display();
		Printf("  UFile\n");
		Printf("    Position: %d\n", File.Position);
		Printf("    Length: %d\n", File.Length);
	}

	KFile File;
};
