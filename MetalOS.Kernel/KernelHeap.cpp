#include "KernelHeap.h"
#include "LoadingScreen.h"

extern LoadingScreen* loading;

void KernelHeap::Initialize(UINT64 address, UINT32 size)
{
	PHEAP_BLOCK pBlock = (PHEAP_BLOCK)address;
	pBlock->size = size;
	pBlock->next = nullptr;
}

KernelHeap::KernelHeap() : m_opened(false), m_head(nullptr), m_address(0), m_size(0)
{

}

void KernelHeap::OpenHeap(UINT64 address, UINT32 size)
{
	this->m_opened = true;
	this->m_head = (PHEAP_BLOCK)address;
	this->m_address = address;
	this->m_size = size;
}

//TODO: take into account alignment
UINT64 KernelHeap::Allocate(UINT32 size)
{
	loading->WriteLineFormat("Allocate: 0x%x", size);

	PHEAP_BLOCK current = this->m_head;
	while ((current->size < size) && (current->next != nullptr))
	{
		current = current->next;
	}

	if (current->next == nullptr)
		return (UINT64)nullptr;

	//Split current block

	return m_address;
}

void KernelHeap::Deallocate(UINT64 address)
{
	loading->WriteLineFormat("Deallocate 0x%16x", address);
}
