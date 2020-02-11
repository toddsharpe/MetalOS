#pragma once

#include "MetalOS.Kernel.h"

class KernelHeap
{
public:
	KernelHeap(uintptr_t address, uint32_t size);

	uintptr_t Allocate(uint32_t size);
	void Deallocate(uintptr_t address);

	void PrintHeap();

	static const uint32_t MinBlockSize = 4;
	static constexpr uint16_t Magic = 0xBEEF;

private:
	typedef struct _HEAP_BLOCK
	{
		uint32_t Size;
		union
		{
			struct
			{
				uint16_t Free : 1;
				uint16_t Magic;
			};
			uint32_t Flags;
		};
		_HEAP_BLOCK* Next;
		uint8_t Block[0];
	} HEAP_BLOCK, *PHEAP_BLOCK;
	static_assert(sizeof(HEAP_BLOCK) == 16, "Heap block has changed");

	uint64_t m_address;
	uint32_t m_size;
	uint32_t m_allocated;
	PHEAP_BLOCK m_head;
};

