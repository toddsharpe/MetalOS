#pragma once

#include "Kernel.h"

class KernelHeap
{
public:
	void static Initialize(UINT64 address, UINT32 size);
	KernelHeap();
	void OpenHeap(UINT64 address, UINT32 size);

	UINT64 Allocate(UINT32 size);
	void Deallocate(UINT64 address);

private:

	//Reserved fields to make this struct alignment friendly.
	//TODO: evaluate: is this too much fluff?
	typedef struct _HEAP_BLOCK
	{
		UINT32 size;
		UINT32 reserved;
		UINT64 reserved2;
		_HEAP_BLOCK* prev;
		_HEAP_BLOCK* next;
		UINT8 block[0];
	} HEAP_BLOCK, *PHEAP_BLOCK;
	static_assert(sizeof(_HEAP_BLOCK) == 32, "Heap Block wrong");

	bool m_opened;
	PHEAP_BLOCK m_head;
	UINT64 m_address;
	UINT32 m_size;
};

