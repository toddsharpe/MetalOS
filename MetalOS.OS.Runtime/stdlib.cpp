extern "C"
{
	#include "stdlib.h"
}
#include <MetalOS.h>
#include <string.h>
#include "Heap.h"

extern Heap CrtHeap;

void exit(int _Code)
{
	ExitProcess(_Code);
}

void* calloc(size_t _Count, size_t _Size)
{
	void* ptr = CrtHeap.Allocate(_Count * _Size);
	memset(ptr, 0, _Count * _Size);
	return ptr;
}

void* realloc(void* _Block, size_t _Size)
{
	void* ptr = CrtHeap.Allocate(_Size);
	memcpy(ptr, _Block, _Size);
	free(_Block);
	return ptr;
}

extern "C" void* malloc(size_t _Size)
{
	return CrtHeap.Allocate(_Size);
}

extern "C" void free(void* _Block)
{
	CrtHeap.Deallocate(_Block);
}

