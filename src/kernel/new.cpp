#include "Kernel.h"

extern Kernel kernel;

void* operator new(const size_t n)
{
	return kernel.Allocate(n);
}

void* operator new[](size_t n)
{
	return kernel.Allocate(n);
}

void operator delete(void* p, size_t n)
{
	kernel.Deallocate(p);
}

void operator delete(void* const p)
{
	kernel.Deallocate(p);
}

void operator delete[](void* const p)
{
	kernel.Deallocate(p);
}

void* __cdecl malloc(size_t size)
{
	return kernel.Allocate(size);
}

void __cdecl free(void* ptr)
{
	kernel.Deallocate(ptr);
}
