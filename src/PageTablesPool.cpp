#include "PageTablesPool.h"

#include "Assert.h"
#include <string.h>
#include <intrin.h>

PageTablesPool::PageTablesPool(void* const baseAddress, const paddr_t physicalAddress, const size_t pageCount) :
	m_baseAddress(baseAddress),
	m_physicalAddress(physicalAddress),
	m_pageCount(pageCount),
	m_index((bool*)baseAddress)
{
	memset((void*)m_baseAddress, 0, PageSize);
}

uint64_t PageTablesPool::GetVirtualAddress(uint64_t physicalAddress)
{
	//Printf("Pin 0x%016x, Pbase: 0x%016x, V: 0x%016x\r\n", physicalAddress, m_physicalAddress, m_baseAddress);
	//Assert(physicalAddress > m_physicalAddress);

	if (physicalAddress < m_physicalAddress)
		return physicalAddress;

	return (physicalAddress - m_physicalAddress) + (uint64_t)m_baseAddress;
}

bool PageTablesPool::AllocatePage(uint64_t * addressOut)
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
		*addressOut = m_physicalAddress + (i << PageShift);
		return true;
	}

	return false;
}

bool PageTablesPool::DeallocatePage(uint64_t address)
{
	uint64_t relative = (address - m_physicalAddress);
	if (relative % PageSize != 0)
		return false;

	size_t index = relative >> PageShift;
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
