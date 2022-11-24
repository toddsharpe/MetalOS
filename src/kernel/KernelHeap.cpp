#include "Kernel.h"
#include "Assert.h"

#include "KernelHeap.h"

KernelHeap::KernelHeap(VirtualMemoryManager& virtualMemory, VirtualAddressSpace& addressSpace) :
	m_memoryManager(virtualMemory),
	m_addressSpace(addressSpace),
	m_address(addressSpace.GetStart()),
	m_end(m_address),
	m_allocated(),
	m_head()
{
	//Initialize heap
	const size_t initPages = (1 << 10);//1KB of pages = 4MB heap
	Grow(initPages);

	//Initialize by creating a head free block
	this->m_head = (HeapBlock*)m_address;
	this->m_head->Next = nullptr;
	this->m_head->Prev = nullptr;
	this->m_head->Free = true;
	this->m_head->Magic = Magic;
}

void KernelHeap::Grow(size_t pages)
{
	PrintHeapHeaders();
	kernel.Printf("Heap Grow - 0x%x\n", pages);
	void* address = m_memoryManager.Allocate(m_end, pages, MemoryProtection::PageReadWrite, m_addressSpace);
	Assert(address);

	m_end += (pages << PAGE_SHIFT);
	kernel.Printf("  NewEnd: 0x%016x, Addr: 0x%016x\n", m_end, address);
}

void* KernelHeap::Allocate(const size_t size, const uintptr_t callerAddress)
{
	const size_t allocationSize = HEAP_ALIGN(size);
	HeapBlock* current = this->m_head;
	while (!current->Free || current->GetLength() < allocationSize)
	{
		Assert(current->Magic == Magic);
		//If this is our last block it needs to be extended
		if (current->Next == nullptr)
			break;

		current = current->Next;
	}

	//Extend last block if needed
	if (current->Next == nullptr && (((size_t)this->m_end - (size_t)current->Data) < allocationSize))
	{
		//End of the list, allocate
		const size_t pages = SIZE_TO_PAGES(allocationSize);
		this->Grow(pages);

		//Grow can allocate, so run block pointer to the end
		while (current->Next != nullptr)
			current = current->Next;

		//Assume the last block is free, if not do more work TBD
		Assert(current->Free);
	}

	//Update block
	current->Free = false;
	current->Magic = KernelHeap::Magic;
	current->CallerAddress = callerAddress;

	//Update statistics
	this->m_allocated += allocationSize;

	uintptr_t address = (uintptr_t)&current->Data;
	if (current->GetLength() - allocationSize < KernelHeap::MinBlockSize + sizeof(HeapBlock))
	{
		//Not enough size to split, just return
		return (void*)address;
	}
	
	//Split the block by inserting after current
	HeapBlock* newBlock = (HeapBlock*)(address + allocationSize);
	newBlock->Next = current->Next;
	newBlock->Prev = current;
	newBlock->Flags = 0;
	newBlock->Free = 1;
	newBlock->Magic = Magic;

	//Update current block
	current->Next = newBlock;

	return (void*)address;
}

void KernelHeap::Deallocate(const void* address)
{
	AssertOp((uintptr_t)address, >=, m_address + sizeof(HeapBlock));
	AssertOp((uintptr_t)address, <, m_end);

	HeapBlock* block = (HeapBlock*)((uintptr_t)address - sizeof(HeapBlock));
	AssertEqual(block->Magic, Magic);
	
	block->Free = true;

	//Condense with previous block
	if (block->Prev->Free)
	{
		block->Prev->Next = block->Next;
		if (block->Next != nullptr)
			block->Next->Prev = block->Prev;
		block = block->Prev;
	}

	//Condense with next block
	if ((block->Next != nullptr) && (block->Next->Free))
	{
		block->Next->Prev = block->Prev;
		block->Prev->Next = block->Next;
	}
}

void KernelHeap::PrintHeap() const
{
	PrintHeapHeaders();
	HeapBlock* current = this->m_head;
	while (current != nullptr)
	{
		kernel.Printf("  L: 0x%8x F: 0x%8x P: 0x%16x, N: 0x%016x IP: 0x%16x\n", current->GetLength(), current->Flags, current->Prev, current->Next, current->CallerAddress);
		current = current->Next;
	}
}

void KernelHeap::PrintHeapHeaders() const
{
	kernel.Printf("PrintHeap\n");
	kernel.Printf("Head: 0x%016x\n", this->m_head);
	kernel.Printf("Address: 0x%016x\n", this->m_address);
	kernel.Printf("End: 0x%016x\n", this->m_end);
	kernel.Printf("Allocated: 0x%016x\n", this->m_allocated);
}
