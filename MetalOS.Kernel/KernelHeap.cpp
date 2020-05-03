#include "KernelHeap.h"

#include "Main.h"

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
	this->m_head = (PHEAP_BLOCK)m_address;
	this->m_head->Size = m_end - m_address;
	this->m_head->Flags = 0;
	this->m_head->Free = true;
	this->m_head->Next = nullptr;
	this->m_head->Magic = Magic;
}

void KernelHeap::Grow(size_t pages)
{
	void* address = m_memoryManager.Allocate(m_end, pages, MemoryProtection(true, true, false), m_addressSpace);
	Assert(address);

	m_end += (pages << PAGE_SHIFT);
}

//TODO: take into account alignment
void* KernelHeap::Allocate(size_t size)
{
	PHEAP_BLOCK pBlock = this->m_head;
	while (!pBlock->Free || pBlock->Size < size + sizeof(HEAP_BLOCK))
	{
		if (pBlock->Next == nullptr)
		{
			//End of the list, allocate
			const size_t pages = SIZE_TO_PAGES(size);
			this->Grow(pages);

			//Grow can allocate, so run block pointer to the end
			while (pBlock->Next != nullptr)
				pBlock = pBlock->Next;

			//Assume the last block is free, if not do more work TBD
			Assert(pBlock->Free);

			pBlock->Size += (pages << PAGE_SHIFT);
			break;
		}

		pBlock = pBlock->Next;
	}

	//Update block
	pBlock->Free = false;

	//Update statistics
	this->m_allocated += size;

	uintptr_t address = (uintptr_t)&pBlock->Block;
	if (pBlock->Size - (size + sizeof(HEAP_BLOCK)) < KernelHeap::MinBlockSize + sizeof(HEAP_BLOCK))
	{
		//Not enough size to split, just return
		return (void*)address;
	}
	
	//Split the block
	PHEAP_BLOCK newBlock = (PHEAP_BLOCK)(address + size);
	newBlock->Size = pBlock->Size - size - sizeof(HEAP_BLOCK);
	newBlock->Flags = 0;
	newBlock->Free = 1;
	newBlock->Magic = Magic;
	newBlock->Next = pBlock->Next;

	//Update current block
	pBlock->Size = size;
	pBlock->Next = newBlock;

	return (void*)address;
}

void KernelHeap::Deallocate(void* address)
{
	Assert((uintptr_t)address >= m_address + sizeof(HEAP_BLOCK));
	Assert((uintptr_t)address < m_end);

	PHEAP_BLOCK pBlock = (PHEAP_BLOCK)((uintptr_t)address - sizeof(HEAP_BLOCK));
	Assert(pBlock->Magic == Magic);
	
	pBlock->Free = true;

	//TODO: check flags?
	//TODO: condense with block before? change to double pointer blocks?
	if (pBlock->Next->Free)
	{
		//Combine this and next blocks
		pBlock->Size += pBlock->Next->Size + sizeof(HEAP_BLOCK);
		pBlock->Next = pBlock->Next->Next;
	}
}

void KernelHeap::PrintHeap()
{
	PHEAP_BLOCK current = this->m_head;
	while (current != nullptr)
	{
		Print("  P 0x%16x S: 0x%8x F: 0x%8x N: 0x%16x\n", current, current->Size, current->Flags, current->Next);
		current = current->Next;
	}
}
