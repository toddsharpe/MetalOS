#include "Assert.h"

#include "PhysicalMemoryManager.h"
#include <shared/MetalOS.System.h>

PhysicalMemoryManager::PhysicalMemoryManager(MemoryMap& memoryMap) :
	m_memoryMap(memoryMap),
	m_length(memoryMap.GetPhysicalAddressSize() >> PageShift),
	m_frames((PAGE_FRAME*)KernelPfnDbStart),
	m_freeList(),
	m_buddyMap(m_length / BuddySize),
	m_nextBuddy()
{
	Assert((memoryMap.GetPhysicalAddressSize() & PageMask) == 0);
	
	//Zero out frame database
	memset(m_frames, 0, sizeof(PAGE_FRAME) * m_length);

	//Initialize free list
	ListInitializeHead(&m_freeList);

	//Populate page states
	for (size_t i = 0; i < memoryMap.Length(); i++)
	{
		const EFI_MEMORY_DESCRIPTOR* desc = memoryMap.Get(i);
		Assert(desc != nullptr);

		const PageState state = GetPageState(*desc);
		Assert((desc->PhysicalStart & PageMask) == 0);
		const size_t baseIndex = desc->PhysicalStart >> PageShift;
		for (size_t j = 0; j < desc->NumberOfPages; j++)
		{
			//Mark page state
			PAGE_FRAME* frame = &m_frames[baseIndex + j];
			frame->State = state;

			//Update buddy availability by ORing free with current buddy value.
			const size_t buddyIndex = (baseIndex + j) / BuddySize;
			const bool notFree = state != PageState::Free;
			m_buddyMap.Set(buddyIndex, m_buddyMap.Get(buddyIndex) || notFree);

			//Conditionally add to free list
			if (state == PageState::Free)
			{
				ListInsertHead(&m_freeList, &frame->Link);
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
		Assert((address & PageMask) == 0);

		const size_t index = address >> PageShift;
		AssertOp(index, < , m_length);
		PAGE_FRAME* entry = &m_frames[index];

		//Check if page is free
		if (entry->State != PageState::Free)
			return false;

		//Set page as active, mark buddy unavailable, remove from free list
		entry->State = PageState::Active;
		m_buddyMap.Set(GetBuddyIndex(address), true);
		ListRemoveEntry(&entry->Link);

		return true;
	}

	//Choose a page, asserting there is one (if not we'd have to free)
	Assert(!ListIsEmpty(&m_freeList));
	ListEntry* popped = ListRemoveHead(&m_freeList);
	PAGE_FRAME* entry = LIST_CONTAINING_RECORD(popped, PAGE_FRAME, Link);
	address = GetIndex(entry) << PageShift;

	entry->State = PageState::Active;
	m_buddyMap.Set(GetBuddyIndex(address), true);

	return true;

}

//Allocates contiguous physical pages, returning base address
bool PhysicalMemoryManager::AllocateContiguous(paddr_t& address, const size_t pageCount)
{
	Printf("AllocateContiguous: 0x%016x, 0x%x\n", address, pageCount);

	const size_t length = m_buddyMap.Length;
	const size_t buddyCount = (pageCount + (BuddySize - 1)) / BuddySize;;

	while (m_nextBuddy < length - buddyCount)
	{
		Printf("Buddy 0x%016x, size 0x%016x, count 0x%016x\n", m_nextBuddy, length, buddyCount);
		//Check to make sure entire buddy group is free
		bool notFree = false;
		for (size_t i = 0; i < buddyCount; i++)
		{
			notFree |= m_buddyMap.Get(m_nextBuddy + i);
			if (notFree)
				break;
		}

		if (notFree)
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
	return sizeof(PAGE_FRAME) * m_length;
}

constexpr size_t PhysicalMemoryManager::GetBuddyIndex(const paddr_t address)
{
	return (address >> PageShift) / BuddySize;
}

PageState PhysicalMemoryManager::GetPageState(const EFI_MEMORY_DESCRIPTOR& desc)
{
	switch (desc.Type)
	{
	case EfiConventionalMemory:
		return PageState::Free;

	default:
		return PageState::Platform;
	}
}

//Frames is an array, so frame number is index
size_t PhysicalMemoryManager::GetIndex(const PAGE_FRAME* entry) const
{
	//Yay for pointer math
	return (entry - m_frames);
}

