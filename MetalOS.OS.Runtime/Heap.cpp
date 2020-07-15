#include "Heap.h"
#include <MetalOS.h>
#include "Debug.h"

void Heap::Initialize(size_t size)
{
	//Allocate
	m_size = size;
	m_address = VirtualAlloc(nullptr, size, MemoryAllocationType::CommitReserve, MemoryProtection::PageReadWrite);
}

void* Heap::Allocate(const size_t size)
{
	return nullptr;
}

void Heap::Deallocate(void* address)
{
}
