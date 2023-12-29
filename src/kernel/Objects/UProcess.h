#pragma once

#include "Kernel/Objects/UObject.h"
#include "UserProcess.h"

#include <memory>

class UProcess : public UObject
{
public:
	UProcess(std::shared_ptr<UserProcess>& proc) :
		UObject(UObjectType::Process),
		Proc(proc)
	{

	}

	virtual void Display() const override
	{
		UObject::Display();
		Printf("  UFile\n");
		Printf("    Name: %s\n", Proc->Name.c_str());
		Printf("    ID: %d\n", Proc->Id);
		Printf("    Console: %d\n", Proc->IsConsole);
	}

	std::shared_ptr<UserProcess> Proc;
};
