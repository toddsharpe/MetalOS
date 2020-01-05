#include "PageTablesPool.h"

PageTablesPool::PageTablesPool(uint64_t physicalAddress, uint32_t pageCount) :
	m_virtualAddress(0), m_physicalAddress(physicalAddress), m_pageCount(pageCount), m_index((bool*)m_virtualAddress)
{
	//TODO
	//Assert(pageCount < PageTablesPoolMax);


}

void PageTablesPool::SetVirtualAddress(uint64_t virtualAddress)
{
	m_virtualAddress = virtualAddress;
	m_index = ((bool*)virtualAddress);
}

bool PageTablesPool::AllocatePage(uint64_t* addressOut)
{
	//First page is index
	//Simple scheme - a page of booleans
	//Yes this should be like bitmasks or maybe ints if i used more info here (like if it was backed to disk)
	//But to stand this up, just booleans

	for (size_t i = 1; i < this->m_pageCount; i++)
	{
		if (m_index[i])
			continue;

		m_index[i] = true;
		*addressOut = m_physicalAddress + (i << PAGE_SHIFT);
		return true;
	}

	return false;
}

bool PageTablesPool::DeallocatePage(uint64_t address)
{
	uint32_t relative = (address - m_physicalAddress);
	if (relative % PAGE_SIZE != 0)
		return false;
	
	size_t index = relative >> PAGE_SHIFT;
	if (index > (m_pageCount - 1))
		return false;
	if (!m_index[index])
		return false;

	m_index[index] = false;
	return true;
}

uint32_t PageTablesPool::AllocatedPageCount()
{
	uint32_t count = 0;
	for (size_t i = 1; i < this->m_pageCount; i++)
	{
		if (!m_index[i])
			continue;

		count++;
	}

	return count;
}

