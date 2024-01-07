#include "PhysicalMemoryManager.h"
#include "Assert.h"

namespace
{
	inline PageState GetPageState(const EFI_MEMORY_DESCRIPTOR& desc)
	{
		switch (desc.Type)
		{
		case EfiConventionalMemory:
			return PageState::Free;

		default:
			return PageState::Reserved;
		}
	}
	
	constexpr size_t BuddySize = 4;

	size_t GetBuddyIndex(const paddr_t address)
	{
		return (address >> PageShift) / BuddySize;
	}
}

PhysicalMemoryManager::PhysicalMemoryManager(void* const address, const size_t count) :
	m_frames(reinterpret_cast<PageFrame*>(address)),
	m_count(count),
	m_freeList(),
	m_buddyMap(count / BuddySize),
	m_nextBuddy()
{

}

void PhysicalMemoryManager::Initialize(const MemoryMap& memoryMap)
{
	//Zero out memory region
	memset(m_frames, 0, sizeof(PageFrame) * m_count);

	//Initialize free list
	ListInitializeHead(&m_freeList);
	m_buddyMap.Initialize();

	//Populate page states
	for (size_t i = 0; i < memoryMap.Length(); i++)
	{
		const EFI_MEMORY_DESCRIPTOR* desc = memoryMap.Get(i);
		Assert(desc);

		const PageState state = GetPageState(*desc);
		const bool regionInUse = state != PageState::Free;
		Assert((desc->PhysicalStart & PageMask) == 0);

		const size_t baseIndex = (desc->PhysicalStart >> PageShift);
		for (size_t j = 0; j < desc->NumberOfPages; j++)
		{
			const size_t absIndex = baseIndex + j;
			
			//Mark page state
			PageFrame& frame = m_frames[absIndex];
			frame.State = state;

			//Update buddy availability by ORing free with current buddy value.
			//Buddy can span regions
			const size_t buddyIndex = absIndex / BuddySize;
			const bool buddyInUse = m_buddyMap.Get(buddyIndex);
			m_buddyMap.Set(buddyIndex, regionInUse || buddyInUse);

			//Conditionally add to free list
			if (!regionInUse)
			{
				ListInsertHead(&m_freeList, &frame.Link);
			}
		}
	}

	//Set buddy to be start of largest conventional region, on BuddySize boundary
	const size_t index = memoryMap.GetLargestConventionalAddress() >> PageShift;
	m_nextBuddy = (index + (BuddySize - 1)) / BuddySize;
}

bool PhysicalMemoryManager::AllocatePage(paddr_t& address)
{
	if (address != 0)
	{
		//Specific address requested
		AssertPrintInt((address & PageMask) == 0, address);

		const size_t index = address >> PageShift;
		AssertOp(index, < , m_count);
		PageFrame& entry = m_frames[index];

		//Check if page is free
		if (entry.State != PageState::Free)
			return false;

		//Set page as active, mark buddy unavailable, remove from free list
		entry.State = PageState::Active;
		m_buddyMap.Set(GetBuddyIndex(address), true);
		ListRemoveEntry(&entry.Link);

		return true;
	}
	else
	{
		//Choose a page, asserting there is one (if not we'd have to free)
		Assert(!ListIsEmpty(&m_freeList));
		ListEntry* popped = ListRemoveHead(&m_freeList);
		PageFrame* entry = LIST_CONTAINING_RECORD(popped, PageFrame, Link);
		address = GetIndex(entry) << PageShift;

		entry->State = PageState::Active;
		m_buddyMap.Set(GetBuddyIndex(address), true);

		return true;
	}
}

void PhysicalMemoryManager::DeallocatePage(const paddr_t address)
{
	NotImplemented();
}

//Allocates contiguous physical pages, returning base address
bool PhysicalMemoryManager::AllocateContiguous(paddr_t& address, const size_t pageCount)
{
	Printf("AllocateContiguous: 0x%016x, 0x%x\n", address, pageCount);

	const size_t length = m_buddyMap.Length;
	const size_t buddyCount = DivRoundUp(pageCount, BuddySize);

	while (m_nextBuddy < (length - buddyCount))
	{
		Printf("Buddy 0x%016x, size 0x%016x, count 0x%016x\n", m_nextBuddy, length, buddyCount);

		//Check to make sure entire buddy group is free
		bool inUse = false;
		for (size_t i = 0; i < buddyCount; i++)
		{
			inUse |= m_buddyMap.Get(m_nextBuddy + i);
			if (inUse)
				break;
		}

		if (inUse)
		{
			m_nextBuddy++;
			continue;
		}

		//Adopt address
		const size_t index = m_nextBuddy * BuddySize;
		address = index << PageShift;

		//Remove these pages from free list
		for (size_t i = 0; i < pageCount; i++)
		{
			ListRemoveEntry(&m_frames[index + i].Link);
		}

		//Mark buddies
		for (size_t i = 0; i < buddyCount; i++)
		{
			m_buddyMap.Set(m_nextBuddy + i, false);
		}

		m_nextBuddy += buddyCount;
		return true;
	}

	return false;
}

size_t PhysicalMemoryManager::GetSize() const
{
	return m_count * sizeof(PageFrame);
}

//Frames is an array, so frame number is index
size_t PhysicalMemoryManager::GetIndex(const PageFrame* entry) const
{
	//Yay for pointer math
	return (entry - m_frames);
}
