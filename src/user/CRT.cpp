#pragma once

#include <stdlib.h>
#include "Assert.h"

//Called through a vtable slot whose pure virtual was never overridden, which is always a bug
int __cdecl _purecall(void)
{
	Assert(false);
	return 0;
}

void* operator new(size_t n)
{
	return malloc(n);
}

void* operator new[](size_t n)
{
	return malloc(n);
}

void operator delete(void* p, size_t count)
{
	free(p);
}

void operator delete(void* p)
{
	free(p);
}

void operator delete[](void* p)
{
	free(p);
}

