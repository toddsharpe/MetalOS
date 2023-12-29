#pragma once

#include "Kernel/MetalOS.Kernel.h"
#include "VirtualAddressSpace.h"
#include "VirtualMemoryManager.h"

//Needs to track every block so condensing works when deallocating, hence free bit
class KernelHeap
{
public:
	KernelHeap(VirtualMemoryManager& virtualMemory, VirtualAddressSpace& addressSpace);

	void* Allocate(const size_t size, void* const caller);
	void Deallocate(void* const address, void* const caller);

	void Display() const;
	void DisplayAllocations() const;

	static const uint16_t Magic = 0xBEEF;

private:
	static constexpr size_t InitialPages = (1 << 12); //4K pages, 16MB heap
	static constexpr size_t HeapAlign = 32;

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
		void* Caller;
		uint8_t Data[];
	};
#pragma warning (pop)
	static_assert(sizeof(HeapBlock) == 32, "Heap block has changed");

	VirtualMemoryManager& m_memoryManager;
	VirtualAddressSpace& m_addressSpace;

	uint64_t m_start;
	uint64_t m_end;
	size_t m_count;
	size_t m_allocated; //Total size of objects allocated, not space being taken up with padding/alignment/overhead
	ListEntry m_blocks;
};

