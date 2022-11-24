extern "C" void __cdecl _Throw_bad_array_new_length()
{

}

#include "Heap.h"
#include <user/Debug.h>
#include <algorithm>
#include <shared/MetalOS.Types.h>
#include <user/MetalOS.h>

Heap::Heap()
{
	const size_t heapSize = 0x2000000; //32MB
	Initialize(heapSize);
}

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

	HeapBlock* current = this->m_head;
	while (!current->Free || current->GetSize() < alignSize)
	{
		if (current->Next == nullptr)
			return nullptr;

		current = current->Next;
	}

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

void* Heap::Reallocate(void* address, size_t size)
{
	uintptr_t blockAddress = (uintptr_t)address - sizeof(HeapBlock);
	HeapBlock* current = (HeapBlock*)blockAddress;
	
	void* ptr = this->Allocate(size);
	if (address != nullptr)
	{
		const size_t copySize = std::min(size, current->Size);
		memcpy(ptr, address, copySize);
		this->Deallocate(address);
	}
	
	return ptr;
}

void Heap::Deallocate(void* address)
{

}

void Heap::Print()
{
	HeapBlock* current = m_head;
	while (current != nullptr)
	{
		DebugPrintf("D: 0x%016x S: 0x%x F: %d N: 0x%016x\n", current->Data, current->Size, current->Free, current->Next);
		current = current->Next;
	}
}
