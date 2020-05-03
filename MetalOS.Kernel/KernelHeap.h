#pragma once

#include "MetalOS.Kernel.h"
#include "VirtualAddressSpace.h"
#include "VirtualMemoryManager.h"

class KernelHeap
{
public:
	KernelHeap(VirtualMemoryManager& virtualMemory, VirtualAddressSpace& addressSpace);

	void* Allocate(size_t size);
	void Deallocate(void* address);

	void PrintHeap();
	uint32_t GetAllocated() const
	{
		return m_allocated;
	}

	static const uint32_t MinBlockSize = 4;
	static constexpr uint16_t Magic = 0xBEEF;

private:
	typedef struct _HEAP_BLOCK
	{
		uint32_t Size;//not including the header
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

	void Grow(size_t pages);

	VirtualMemoryManager& m_memoryManager;
	VirtualAddressSpace& m_addressSpace;

	uint64_t m_address;
	uint64_t m_end;
	uint32_t m_allocated; //Total size of objects allocated, not space being taken up with padding/alignment/overhead
	PHEAP_BLOCK m_head;
};

