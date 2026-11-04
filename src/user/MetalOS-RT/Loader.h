#pragma once

#include "windows/types.h"
#include "windows/winnt.h"
#include "user/MetalOS.Types.h"
#include "user/MetalOS.h"

namespace Loader
{
	//Loads each imported module and patches the IAT, except for imports from skipModule (may be null)
	void ResolveImports(void* const imageBase, const char* const skipModule);
}

//0x20E, not exported: tells the kernel the name and base of a DLL this loader mapped (the kernel reads its size), so traces through it have symbols
extern "C" SyscallResult RegisterModule(const char* name, const void* imageBase);
