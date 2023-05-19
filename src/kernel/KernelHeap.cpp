#include "KernelHeap.h"

#include "Assert.h"
#include <kernel/MetalOS.List.h>

KernelHeap::KernelHeap(VirtualMemoryManager& virtualMemory, VirtualAddressSpace& addressSpace) :
	m_memoryManager(virtualMemory),
	m_addressSpace(addressSpace),
	m_start(addressSpace.Start),
	m_end(m_start),
	m_count(),
	m_allocated(),
	m_blocks()
{
	//Initialize heap
	Grow(InitialPages);

	//Initialize block list
	ListInitializeHead(&m_blocks);

	//Create block for empty region
	HeapBlock* head = (HeapBlock*)m_start;
	head->Free = true;
	head->Size = (m_end - m_start) - sizeof(HeapBlock);
	head->Magic = Magic;
	head->Caller = nullptr;

	//Add to free list
	ListInsertTail(&m_blocks, &head->Link);
}

void KernelHeap::Grow(const size_t pages)
{
	void* const address = m_memoryManager.Allocate((void*)m_end, pages, m_addressSpace);
	AssertEqual((uintptr_t)address, m_end);

	m_end += (pages << PageShift);
	Printf("  NewEnd: 0x%016x\n", m_end);
}

void* KernelHeap::Allocate(const size_t size, void* const caller)
{
	const size_t allocationSize = ByteAlign(size, HeapAlign);

	ListEntry* entry = m_blocks.Flink;
	HeapBlock* block = LIST_CONTAINING_RECORD(entry, HeapBlock, Link);
	while (!block->Free || block->Size < allocationSize)
	{
		AssertEval(block->Magic == Magic, DisplayAllocations());

		entry = entry->Flink;
		block = LIST_CONTAINING_RECORD(entry, HeapBlock, Link);

		//Dont loop around
		if (entry == &m_blocks)
			break;
	}

	//If there isn't a free block, Grow
	if (entry == &m_blocks)
	{
		/*
		Printf("Test\n");
		Printf("Allocate: 0x%x\n", size);
		Display();
		const uintptr_t old_end = m_end;
		Printf("Grow, allocate: 0x%x\n", allocationSize);
		Grow(InitialPages);

		block = (HeapBlock*)old_end;
		ListInsertTail(&m_blocks, &block->Link);
		entry = &block->Link;
		block->Size = (m_end - old_end) - sizeof(HeapBlock);
		block->Magic = Magic;
		block->Caller = nullptr;
		*/
		Assert(false);
	}

	//Mark allocated
	block->Caller = caller;
	block->Free = false;

	//Update statistics
	m_allocated += allocationSize;
	m_count++;

	//Determine if block should be split
	const uintptr_t address = (uintptr_t)&block->Data;
	if (block->Size - allocationSize < (HeapAlign + sizeof(HeapBlock)))
	{
		AssertEval(CheckHeap(), DisplayAllocations());
		memset((void*)address, 0xCC, block->Size);
		return (void*)address;
	}

	//Split the block by inserting after current
	HeapBlock* newBlock = (HeapBlock*)(address + allocationSize);
	ListInsertHead(&block->Link, &newBlock->Link);
	newBlock->Free = true;
	newBlock->Size = block->Size - allocationSize - sizeof(HeapBlock);
	newBlock->Magic = Magic;
	newBlock->Caller = nullptr;

	block->Size = allocationSize;
	AssertEval(CheckHeap(), DisplayAllocations());
	memset((void*)address, 0xCC, block->Size);
	return (void*)address;
}

void KernelHeap::Deallocate(void* const address, void* const caller)
{
	AssertOp((uintptr_t)address, >=, m_start + sizeof(HeapBlock));
	AssertOp((uintptr_t)address, <, m_end);

	HeapBlock* block = (HeapBlock*)((uintptr_t)address - sizeof(HeapBlock));
	AssertEqual(block->Magic, Magic);
	Assert(!block->Free);
	Assert(&block->Data == address);
	
	//Mark free
	block->Free = true;
	block->Caller = caller;

	//Update statistics
	m_allocated -= block->Size;
	m_count--;

	//Fill with 0xCC
	memset(address, 0xBB, block->Size);

	/*
	//Condense with previous block
	HeapBlock* prev = LIST_CONTAINING_RECORD(block->Link.Blink, HeapBlock, Link);
	if (prev->Free)
	{
		prev->Size += sizeof(HeapBlock) + block->Size;
		ListRemoveEntry(&block->Link);
		block = prev;
	}

	//Condense with next block
	HeapBlock* next = LIST_CONTAINING_RECORD(block->Link.Flink, HeapBlock, Link);
	if (next->Free)
	{
		block->Size += sizeof(HeapBlock) + next->Size;
		ListRemoveEntry(&next->Link);
	}
	*/
	AssertEval(CheckHeap(), DisplayAllocations());
}

void KernelHeap::Display() const
{
	Printf("PrintHeap\n");
	Printf("Head: 0x%016x\n", this->m_start);
	Printf("Start: 0x%016x\n", this->m_start);
	Printf("End: 0x%016x\n", this->m_end);
	Printf("Allocated: 0x%016x\n", this->m_allocated);
	Printf("Count: 0x%016x\n", this->m_count);

	HeapBlock* block = LIST_CONTAINING_RECORD(m_blocks.Blink, HeapBlock, Link);
	Assert(block->Magic == Magic);
	Printf("Back Size: 0x%016x\n", block->Size);
	Printf("Back addr: 0x%016x\n", block);
}

void KernelHeap::DisplayAllocations() const
{
	ListEntry* entry = m_blocks.Flink;

	while (entry != &m_blocks)
	{
		HeapBlock* block = LIST_CONTAINING_RECORD(entry, HeapBlock, Link);
		Assert(block->Magic == Magic);

		Printf("  Addr: 0x%016x, Size: 0x%016x, Free: %d, Caller: 0x%016x\n", &block->Data, block->Size, block->Free, block->Caller);
		entry = entry->Flink;
	}
}

bool KernelHeap::CheckHeap()
{
	//Ensure all addresses are monotonically increasing.
	ListEntry* current = m_blocks.Flink;
	HeapBlock* block = LIST_CONTAINING_RECORD(current, HeapBlock, Link);
	uintptr_t lastAddr = 0;
	while (current != &m_blocks)
	{
		if ((uintptr_t)block->Data < lastAddr)
			return false;

		lastAddr = (uintptr_t)block->Data;
		current = current->Flink;
		block = LIST_CONTAINING_RECORD(current, HeapBlock, Link);
	}

	return true;
}
