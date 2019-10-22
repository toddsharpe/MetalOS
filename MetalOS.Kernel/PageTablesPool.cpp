#include "PageTablesPool.h"
#include "Main.h"
#include "Kernel.h"

PageTablesPool::PageTablesPool(UINT64 physicalAddress, UINT32 pageCount) : m_physicalAddress(physicalAddress), m_pageCount(pageCount), m_index()
{
	Assert(pageCount < PageTablesPoolMax);


}

UINT64 PageTablesPool::AllocatePage()
{
	//First page is index
	//Simple scheme - a page of booleans
	//Yes this should be like bitmasks or maybe ints if i used more info here (like if it was backed to disk)
	//But to stand this up, just booleans

	for (UINT32 i = 1; i < PAGE_SIZE; i++)
	{
		if (m_index[i])
			continue;

		m_index[i] = true;
		return m_physicalAddress + i << PAGE_SHIFT;
	}
}

void PageTablesPool::DeallocatePage(UINT64 address)
{
	UINT32 relative = (address - m_physicalAddress);
	Assert(relative % PAGE_SIZE == 0);
	
	UINT32 index = relative >> PAGE_SHIFT;
	Assert(index < (m_pageCount - 1));
	Assert(m_index[index]);

	m_index[index] = false;
}

