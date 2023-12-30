#pragma once
#include "stdlib.h"

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

