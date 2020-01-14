#include "PageFrameAllocator.h"
#include <Kernel.h>
#include "Main.h"
#include "LoadingScreen.h"

extern LoadingScreen* loading;

PageFrameAllocator::PageFrameAllocator(MemoryMap& memoryMap) :
	m_nextIndex((memoryMap.GetLargestConventionalAddress() / PAGE_SIZE) - 1),
	m_bitmap(memoryMap.GetPhysicalAddressSize() / PAGE_SIZE),
	m_memoryMap(memoryMap)
{

}

uintptr_t PageFrameAllocator::AllocatePage()
{
	//m_nextIndex points to the last allocated page
	m_nextIndex++;
	
	uintptr_t start = m_nextIndex;
	while (m_bitmap.Get(m_nextIndex) || !m_memoryMap.IsConventional(m_nextIndex * PAGE_SIZE))
	{
		m_nextIndex = (m_nextIndex + 1) % m_bitmap.Size();

		//Assert if physical memory is full?
		Assert(start != m_nextIndex);
	}

	return m_nextIndex * PAGE_SIZE;
}

//TODO
void* PageFrameAllocator::AllocatePages(size_t count)
{
	return nullptr;
}

void PageFrameAllocator::DeallocatePage(uintptr_t address)
{
	//Assert address is on page boundary
	Assert((address & ~PAGE_MASK) == 0);

	size_t index = address >> PAGE_SHIFT;
	Assert(m_bitmap.Get(index));//Assert page is allocated

	m_bitmap.Clear(index);
}

//TODO
void PageFrameAllocator::DeallocatePages(uintptr_t address, size_t count)
{
}
