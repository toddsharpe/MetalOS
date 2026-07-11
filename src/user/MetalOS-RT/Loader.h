#pragma once

#include "windows/types.h"
#include "windows/winnt.h"
#include "user/MetalOS.Types.h"

namespace Loader
{
	typedef void (*CrtInitializer)();
	void CrtInit(const void* const imageBase);
}
