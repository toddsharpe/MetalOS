#pragma once

#include <cstddef>
#include <cstdint>
#include "Kernel/PhysicalMemoryManager.h"

//Needs to track every block so condensing works when deallocating, hence free bit
class KernelHeap
{
public:
	KernelHeap(PhysicalMemoryManager& physicalMemory, void* const heapStart, void* const heapEnd);

	void Initialize();
	bool IsInitialized();
	void* Allocate(const size_t size);
	void Deallocate(void* const address);

	void Display() const;
	void DisplayAllocations() const;

	static const uint16_t Magic = 0xBEEF;

private:
	void Grow(const size_t pages);
	bool CheckHeap();

#pragma warning (push)
#pragma warning(disable: 4200) //Disable zero-sized member warning
	struct HeapBlock
	{
		ListEntry Link;
		uint64_t Free : 1;
		uint64_t Size : 47;
		uint64_t Magic : 16;//Debug only, verify block integrity
		uint8_t Data[];
	};
#pragma warning (pop)
	static_assert(sizeof(HeapBlock) == 24, "Heap block has changed");

	PhysicalMemoryManager& m_physicalMemory;

	bool m_isInitialized;

	//Housekeeping
	ListEntry m_blocks;
	uintptr_t m_start;
	uintptr_t m_watermark;
	uintptr_t m_end;

	//Stats
	size_t m_bytes;
	size_t m_count;
};

