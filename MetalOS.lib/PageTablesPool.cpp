#include "PageTablesPool.h"
#include "Kernel.h"

PageTablesPool::PageTablesPool(UINT64 physicalAddress, UINT32 pageCount) : m_physicalAddress(physicalAddress), m_pageCount(pageCount), m_index()
{
	//TODO
	//Assert(pageCount < PageTablesPoolMax);



}

bool PageTablesPool::AllocatePage(UINT64* addressOut)
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
		*addressOut = m_physicalAddress + (i << PAGE_SHIFT);
		return true;
	}

	return false;
}

bool PageTablesPool::DeallocatePage(UINT64 address)
{
	UINT32 relative = (address - m_physicalAddress);
	if (relative % PAGE_SIZE == 0)
		return false;
	
	UINT32 index = relative >> PAGE_SHIFT;
	if (index < (m_pageCount - 1))
		return false;
	if (m_index[index])
		return false;

	m_index[index] = false;
}

