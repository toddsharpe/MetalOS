#include "KernelHeap.h"

#include "Assert.h"
#include "MetalOS.List.h"
#include <PageTables.h>
#include <intrin.h>

namespace
{
	static constexpr size_t InitialPages = (1 << 12); //4K pages, 16MB heap
	static constexpr size_t HeapAlign = 32;
}

KernelHeap::KernelHeap(PhysicalMemoryManager& physicalMemory, void* const heapStart, void* const heapEnd) :
	m_physicalMemory(physicalMemory),
	m_isInitialized(),
	m_blocks(),
	m_start(reinterpret_cast<uintptr_t>(heapStart)),
	m_watermark(reinterpret_cast<uintptr_t>(heapStart)),
	m_end(reinterpret_cast<uintptr_t>(heapEnd)),
	m_bytes(),
	m_count()
{

}

void KernelHeap::Initialize()
{
	//Initialize block list
	ListInitializeHead(&m_blocks);

	//Initialize heap
	Grow(InitialPages);

	//Create block for empty region, adding to block list
	HeapBlock* head = (HeapBlock*)m_start;
	ListInsertTail(&m_blocks, &head->Link);
	head->Free = true;
	head->Size = (m_end - m_start) - sizeof(HeapBlock);
	head->Magic = Magic;

	m_isInitialized = true;
}

bool KernelHeap::IsInitialized()
{
	return m_isInitialized;
}

void* KernelHeap::Allocate(const size_t size)
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
	block->Free = false;

	//Update statistics
	m_bytes += allocationSize;
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

	block->Size = allocationSize;
	AssertEval(CheckHeap(), DisplayAllocations());
	memset((void*)address, 0xCC, block->Size);
	return (void*)address;
}

void KernelHeap::Deallocate(void* const address)
{
	AssertOp((uintptr_t)address, >=, m_start + sizeof(HeapBlock));
	AssertOp((uintptr_t)address, <, m_end);

	HeapBlock* block = (HeapBlock*)((uintptr_t)address - sizeof(HeapBlock));
	AssertEqual(block->Magic, Magic);
	Assert(!block->Free);
	Assert(&block->Data == address);
	
	//Mark free
	block->Free = true;

	//Update statistics
	m_bytes -= block->Size;
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
	Printf("    Start: 0x%016x\n", m_start);
	Printf("    Watermark: 0x%016x\n", m_watermark);
	Printf("    End: 0x%016x\n", m_end);
	Printf("    Bytes: 0x%016x\n", m_bytes);
	Printf("    Count: 0x%016x\n", m_count);

	//Display last block
	const HeapBlock* block = LIST_CONTAINING_RECORD(m_blocks.Blink, HeapBlock, Link);
	Assert(block->Magic == Magic);
	Printf("    Back Size: 0x%016x\n", block->Size);
	Printf("    Back addr: 0x%016x\n", block);
}

void KernelHeap::DisplayAllocations() const
{
	ListEntry* entry = m_blocks.Flink;
	while (entry != &m_blocks)
	{
		HeapBlock* block = LIST_CONTAINING_RECORD(entry, HeapBlock, Link);
		Assert(block->Magic == Magic);

		Printf("    Addr: 0x%016x, Size: 0x%016x, Free: %d\n", &block->Data, block->Size, block->Free);
		entry = entry->Flink;
	}
}

void KernelHeap::Grow(const size_t pages)
{
	Printf("KernelHeap::Grow\n");
	Printf("    Watermark: 0x%016X\n", m_watermark);

	PageTables tables;
	tables.OpenCurrent();
	for (size_t i = 0; i < pages; i++)
	{
		paddr_t page = 0;
		Assert(m_physicalMemory.AllocatePage(page));
		Assert(tables.MapPages(m_watermark, page, 1, true));
		m_watermark += PageSize;
	}
	Printf("    Watermark: 0x%016X\n", m_watermark);
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
