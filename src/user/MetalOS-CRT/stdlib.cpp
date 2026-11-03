#pragma once

#include "user/stdlib.h"
#include "user/MetalOS.h"

Heap CrtHeap = {};

extern "C"
{
	void exit(int _Code)
	{
		ExitProcess(_Code);
	}

	void* calloc(size_t _Count, size_t _Size)
	{
		//A wrapped-around size would hand back a block smaller than asked for
		if (_Size != 0 && _Count > (size_t)-1 / _Size)
			return nullptr;

		void* ptr = CrtHeap.Allocate(_Count * _Size);
		if (ptr)
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
