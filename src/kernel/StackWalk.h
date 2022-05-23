#pragma once

#include <cstdint>
#include "MetalOS.Kernel.h"

//https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64?view=vs-2019
//http://www.nynaeve.net/Code/StackWalk64.cpp
class StackWalk
{
public:
	//ImageBase should probably be retired once kernel knows IP->Module lookups
	StackWalk(PCONTEXT context, PKNONVOLATILE_CONTEXT_POINTERS contextPointers = nullptr);

	bool HasNext();
	PCONTEXT Next(const uintptr_t imageBase);

private:
	PCONTEXT m_context;
	PKNONVOLATILE_CONTEXT_POINTERS m_contextPointers;
};

