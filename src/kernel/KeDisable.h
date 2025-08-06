#pragma once

#include "Assert.h"

int __cdecl _purecall(void)
{
	Assert(false);
	return 0;
}
