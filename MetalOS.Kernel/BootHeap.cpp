#include "BootHeap.h"
#include "Main.h"

BootHeap::BootHeap(const void* address, const size_t length) :
	m_address((uintptr_t)address),
	m_length(length),
	m_watermark(m_address)
{

}

void* BootHeap::Allocate(size_t size)
{
	Assert(m_watermark + size <= m_address + m_length);

	const uintptr_t level = m_watermark;

	m_watermark += HEAP_ALIGN(size);
	return (void*)level;
}

//Don't deallocate
void BootHeap::Deallocate(void* address)
{
	Assert(false);
}
