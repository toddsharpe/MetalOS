#include "Heap.h"
#include "user/MetalOS.Types.h"
#include "MetalOS.h"
#include <string.h>

Heap CrtHeap;

extern "C"
{
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
		return CrtHeap.Reallocate(_Block, _Size);
	}

	void* malloc(size_t _Size)
	{
		return CrtHeap.Allocate(_Size);
	}

	void free(void* _Block)
	{
		CrtHeap.Deallocate(_Block);
	}
}
