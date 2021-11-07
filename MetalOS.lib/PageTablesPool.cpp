#include "PageTablesPool.h"

#include <crt_string.h>
#include "PageTables.h"

PageTablesPool::PageTablesPool(uint64_t physicalAddress, uint32_t pageCount) : PageTablesPool(physicalAddress, physicalAddress, pageCount)
{
	
}

PageTablesPool::PageTablesPool(uint64_t virtualAddress, uint64_t physicalAddress, uint32_t pageCount) :
	m_virtualAddress(virtualAddress), m_physicalAddress(physicalAddress), m_pageCount(pageCount), m_index((bool*)m_virtualAddress)
{
	memset((void*)m_virtualAddress, 0, PAGE_SIZE);
}

uint64_t PageTablesPool::GetVirtualAddress(uint64_t physicalAddress)
{
	return (physicalAddress - m_physicalAddress) + m_virtualAddress;
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
	uint64_t relative = (address - m_physicalAddress);
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

