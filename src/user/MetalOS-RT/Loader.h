#pragma once

#include "windows/types.h"
#include "windows/winnt.h"
#include "user/MetalOS.Types.h"

namespace Loader
{
	//Loads each imported module and patches the IAT, except for imports from skipModule (may be null)
	void ResolveImports(void* const imageBase, const char* const skipModule);
}
