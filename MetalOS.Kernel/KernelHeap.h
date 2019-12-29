#pragma once

#include "Kernel.h"
#include "Main.h"

class KernelHeap
{
public:
	KernelHeap(UINT64 address, UINT32 size);

	UINT64 Allocate(UINT32 size);
	void Deallocate(UINT64 address);

	void PrintHeap();

	static const INT32 MinBlockSize = 4;
	static constexpr UINT16 Magic = 0xBEEF;

private:
	typedef struct _HEAP_BLOCK
	{
		UINT32 Size;
		union
		{
			struct
			{
				UINT16 Free : 1;
				UINT16 Magic;
			};
			UINT32 Flags;
		};
		_HEAP_BLOCK* Next;
		UINT8 Block[0];
	} HEAP_BLOCK, *PHEAP_BLOCK;
	static_assert(sizeof(HEAP_BLOCK) == 16, "Heap block has changed");

	UINT64 m_address;
	UINT32 m_size;
	UINT32 m_allocated;
	PHEAP_BLOCK m_head;
};

