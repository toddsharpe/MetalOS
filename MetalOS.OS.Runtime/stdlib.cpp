extern "C"
{
	#include "stdlib.h"
}
#include "Heap.h"

extern Heap CrtHeap;

extern "C" void* malloc(size_t _Size)
{
	return CrtHeap.Allocate(_Size);
}

extern "C" void free(void* _Block)
{
	CrtHeap.Deallocate(_Block);
}

