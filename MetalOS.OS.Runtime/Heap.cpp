#include "Heap.h"
#include <MetalOS.h>
#include "Debug.h"

void Heap::Initialize(size_t size)
{
	//Allocate
	m_size = size;
	m_address = VirtualAlloc(nullptr, size, MemoryAllocationType::CommitReserve, MemoryProtection::PageReadWrite);

	m_head = static_cast<HeapBlock*>(m_address);
	m_head->Next = nullptr;
	m_head->Size = size - sizeof(HeapBlock);
	m_head->Free = true;
}

void* Heap::Allocate(const size_t size)
{
	const size_t alignSize = HeapAlign(size);
	DebugPrintf("Alloc 0x%016x Size: 0x%016x\n", alignSize, m_size);

	HeapBlock* current = this->m_head;
	DebugPrintf("Head: 0x%016x, Size: 0x%016x\n", current, alignSize);
	while (!current->Free || current->GetSize() < alignSize)
	{
		if (current->Next == nullptr)
			return nullptr;

		current = current->Next;
	}

	DebugPrintf("Current: 0x%016x\n", current);

	//Update block
	current->Free = false;

	uintptr_t address = (uintptr_t)&current->Data;
	if (current->GetSize() - alignSize < Heap::MinBlockSize)
	{
		//Not enough size to split, just return
		return (void*)address;
	}

	//Split the block by inserting after current
	HeapBlock* newBlock = (HeapBlock*)(address + alignSize);
	newBlock->Next = current->Next;
	newBlock->Size = current->GetSize() - alignSize - sizeof(HeapBlock);
	newBlock->Free = 1;

	//Update current block
	current->Next = newBlock;
	current->Size = alignSize;

	return (void*)address;
}

void Heap::Deallocate(void* address)
{

}
