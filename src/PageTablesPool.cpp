#include "PageTablesPool.h"

#include "Assert.h"
#include <string.h>
#include <intrin.h>

namespace
{
	//One page worth of bools
	constexpr size_t PageCountMax = PageSize / sizeof(bool);
}

PageTablesPool::PageTablesPool(void* const virtualBase, const paddr_t physicalBase, const size_t count) :
	m_virtualBase(reinterpret_cast<uintptr_t>(virtualBase)),
	m_physicalBase(physicalBase),
	m_count(count),
	m_index(reinterpret_cast<bool*>(virtualBase))
{

}

void PageTablesPool::Initialize()
{
	Assert(m_count < PageCountMax);
	
	//Zero index page
	memset(m_index, 0, PageSize);
}

bool PageTablesPool::AllocatePage(paddr_t& address)
{
	//First page is array of bools as index
	for (size_t i = 1; i < this->m_count; i++)
	{
		//True indicates allocated
		if (m_index[i])
			continue;

		//Allocate page
		m_index[i] = true;
		address = (m_physicalBase + (i << PageShift));
		return true;
	}

	return false;
}

bool PageTablesPool::DeallocatePage(const paddr_t address)
{
	Assert(address % PageSize == 0);

	const paddr_t relative = (address - m_physicalBase);
	const size_t index = relative >> PageShift;
	Assert(index < m_count);
	
	//Assert page was allocated, then mark it deallocated
	Assert(m_index[index]);
	m_index[index] = false;
	return true;
}

//Existing pages require identity shortcut
void* PageTablesPool::GetVirtualAddress(const paddr_t address) const
{
	//Assert(address > m_physicalBase);
	//Assert(address < m_physicalBase + PageSize * m_count);

	if (address < m_physicalBase)
		return (void*)address;

	return (void*)((address - m_physicalBase) + m_virtualBase);
}
