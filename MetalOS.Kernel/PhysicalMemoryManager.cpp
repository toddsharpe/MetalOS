#include "PhysicalMemoryManager.h"

#include "Main.h"

//TODO: consider loader building this structure?
//Chicken and egg with memorymap not being finalized?
//Could reserve the memory and build in kernel?
PhysicalMemoryManager::PhysicalMemoryManager(MemoryMap& memoryMap) :
	m_zeroed(),
	m_free(),
	m_memoryMap(memoryMap),
	m_db()
{
	//Allocate space
	m_length = memoryMap.GetPhysicalAddressSize() / PAGE_SIZE;
	m_db = (PFN_ENTRY*)KernelPfnDbStart;
	memset(m_db, 0, sizeof(PFN_ENTRY) * m_length);

	//Populate page states
	for (size_t i = 0; i < memoryMap.Length(); i++)
	{
		const EFI_MEMORY_DESCRIPTOR* desc = memoryMap.Get(i);
		const PhysicalPageState state = GetPageState(desc);

		for (size_t j = 0; j < desc->NumberOfPages; j++)
		{
			//Mark page in db
			const size_t base = desc->PhysicalStart >> PAGE_SHIFT;
			m_db[base + j].State = state;

			if (state == PhysicalPageState::Free)
			{
				//Push on free stack
				//Top of stack has null next ptr
				m_db[base + j].Prev = m_free;
				m_free = &m_db[base + j];
				if (m_free->Prev != nullptr)
					m_free->Prev->Next = m_free;
			}
		}
	}
}

bool PhysicalMemoryManager::AllocatePage(paddr_t& address, PhysicalPageState& state)
{
	if (address != 0)
	{
		//Specific address request
		Assert((address && PAGE_MASK) == 0);

		PFN_ENTRY& entry = m_db[address >> PAGE_SHIFT];
		if (entry.State != PhysicalPageState::Zeroed && entry.State != PhysicalPageState::Free)
			return false;

		state = entry.State;
		entry.State = PhysicalPageState::Active;
		return true;
	}

	//Choose a page
	if (m_zeroed != nullptr)
	{
		address = GetPFN(m_zeroed) << PAGE_SHIFT;
		m_zeroed = m_zeroed->Prev;
		state = PhysicalPageState::Zeroed;
		return true;
	}
	else if (m_free != nullptr)
	{
		address = GetPFN(m_free) << PAGE_SHIFT;
		m_free = m_free->Prev;
		state = PhysicalPageState::Free;
		return true;
	}
	else
	{
		Assert(false);//This is where we need to free physical frames
		return false;
	}
}

const PhysicalPageState PhysicalMemoryManager::GetPageState(const EFI_MEMORY_DESCRIPTOR* desc)
{
	switch (desc->Type)
	{
	case EfiConventionalMemory:
		return PhysicalPageState::Free;

	case EfiLoaderData:
		return PhysicalPageState::Active;

	default:
		return PhysicalPageState::Platform;
	}
}

paddr_t PhysicalMemoryManager::GetPFN(PPFN_ENTRY entry)
{
	return ((paddr_t)entry - (paddr_t)(m_db)) / sizeof(PFN_ENTRY);
}

