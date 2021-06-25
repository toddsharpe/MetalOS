#include "PhysicalMemoryManager.h"

#include "Main.h"

//TODO: consider loader building this structure?
//Chicken and egg with memorymap not being finalized?
//Could reserve the memory and build in kernel?
PhysicalMemoryManager::PhysicalMemoryManager(MemoryMap& memoryMap) :
	m_zeroed(),
	m_free(),
	m_buddyMap(memoryMap.GetPhysicalAddressSize() / (PAGE_SIZE * BuddySize)),
	m_nextBuddy(),
	m_memoryMap(memoryMap),
	m_db()
{
	//Connect to PFN DB reserved by Boot
	m_length = memoryMap.GetPhysicalAddressSize() / PAGE_SIZE;
	m_db = (PFN_ENTRY*)KernelPfnDbStart;
	memset(m_db, 0, sizeof(PFN_ENTRY) * m_length);

	//Populate page states
	for (size_t i = 0; i < memoryMap.Length(); i++)
	{
		const EFI_MEMORY_DESCRIPTOR* desc = memoryMap.Get(i);
		const PageState state = GetPageState(desc);

		for (size_t j = 0; j < desc->NumberOfPages; j++)
		{
			//Mark page in db
			const size_t base = desc->PhysicalStart >> PAGE_SHIFT;
			m_db[base + j].State = state;

			//Buddy cells can span memory types. To ensure buddy is only free if all states are free,
			//use false for Free, and OR the results
			const size_t buddyIndex = (base + j) / BuddySize;
			const bool flag = state != PageState::Free;
			m_buddyMap.Set(buddyIndex, m_buddyMap.Get(buddyIndex) || flag);

			if (state == PageState::Free)
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

	//Set buddy to be start of largect conventional, on BuddySize boundary
	const size_t index = memoryMap.GetLargestConventionalAddress() >> PAGE_SHIFT;
	m_nextBuddy = index / BuddySize + ((index % BuddySize != 0) ? 1 : 0);
}

bool PhysicalMemoryManager::AllocatePage(paddr_t& address, PageState& state)
{
	if (address != 0)
	{
		//Specific address request
		Assert((address & PAGE_MASK) == 0);

		PFN_ENTRY& entry = m_db[address >> PAGE_SHIFT];
		if (entry.State != PageState::Zeroed && entry.State != PageState::Free)
			return false;

		state = entry.State;
		entry.State = PageState::Active;
		m_buddyMap.Set(GetBuddyIndex(address), true);
		return true;
	}

	//Choose a page
	if (m_zeroed != nullptr)
	{
		address = GetPFN(m_zeroed) << PAGE_SHIFT;
		m_zeroed = m_zeroed->Prev;
		state = PageState::Zeroed;
		m_buddyMap.Set(GetBuddyIndex(address), true);
		return true;
	}
	else if (m_free != nullptr)
	{
		address = GetPFN(m_free) << PAGE_SHIFT;
		m_free = m_free->Prev;
		state = PageState::Free;
		m_buddyMap.Set(GetBuddyIndex(address), true);
		return true;
	}
	else
	{
		Assert(false);//This is where we need to free physical frames
		return false;
	}
}

bool PhysicalMemoryManager::AllocateContiguous(paddr_t& address, const size_t pageCount)
{
	Print("AllocateContiguous: 0x%x\n", pageCount);
	const size_t size = m_buddyMap.Size();
	const size_t count = pageCount / BuddySize + ((pageCount % BuddySize != 0) ? 1 : 0);
	while (m_nextBuddy < size - count)
	{
		bool found = true;
		for (size_t i = 0; i < count; i++)
		{
			if (m_buddyMap.Get(m_nextBuddy + i))
			{
				found = false;
				break;
			}
		}

		if (found)
		{
			address = (m_nextBuddy * BuddySize) << PAGE_SHIFT;

			//Remove these pages from free lists (can't be on zero'd list yet)
			PPFN_ENTRY entry = &m_db[address >> PAGE_SHIFT];
			for (size_t i = 0; i < pageCount; i++)
			{
				if (m_free == entry)
				{
					//Top of list has null next pointer
					m_free = m_free->Prev;
				}
				else
				{
					if (entry->Next != nullptr)
						entry->Next->Prev = entry->Prev;

					if (entry->Prev != nullptr)
						entry->Prev->Next = entry->Next;

					entry->Next = nullptr;
					entry->Prev = nullptr;
				}
				entry++;
			}

			m_buddyMap.Set(GetBuddyIndex(address), true);
			m_nextBuddy += count;
			return true;
		}

		m_nextBuddy++;
	}

	return false;
}

const PageState PhysicalMemoryManager::GetPageState(const EFI_MEMORY_DESCRIPTOR* desc)
{
	switch (desc->Type)
	{
	case EfiConventionalMemory:
		return PageState::Free;

	case EfiLoaderData:
		return PageState::Active;

	default:
		return PageState::Platform;
	}
}

paddr_t PhysicalMemoryManager::GetPFN(PPFN_ENTRY entry)
{
	return ((paddr_t)entry - (paddr_t)(m_db)) / sizeof(PFN_ENTRY);
}

const size_t PhysicalMemoryManager::GetBuddyIndex(paddr_t address)
{
	return (address >> PAGE_SHIFT) / BuddySize;
}

