#include "user/MetalOS-CRT/CRT.h"
#include "user/MetalOS.h"
#include "ToErrno.h"

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
		{
			errno = ENOMEM;
			return nullptr;
		}

		void* ptr = malloc(_Count * _Size);
		if (ptr)
			memset(ptr, 0, _Count * _Size);
		return ptr;
	}

	void* realloc(void* _Block, size_t _Size)
	{
		//Reallocating a block to nothing frees it, and that nullptr isn't a failure
		if (_Block && _Size == 0)
		{
			CrtHeap.Deallocate(_Block);
			return nullptr;
		}

		void* const moved = CrtHeap.Reallocate(_Block, _Size);
		if (!moved)
			errno = ENOMEM;
		return moved;
	}

	void* malloc(size_t _Size)
	{
		void* const ptr = CrtHeap.TryAllocate(_Size);
		if (!ptr)
			errno = ENOMEM;
		return ptr;
	}

	void free(void* _Block)
	{
		CrtHeap.Deallocate(_Block);
	}
}
