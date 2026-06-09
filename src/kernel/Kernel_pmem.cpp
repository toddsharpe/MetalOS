#include "kernel/Api.h"
#include "kernel/Arch.h"
#include "MetalOS.Loader.h"
#include "Lib/Bitset.h"
#include "Lib/IntrusiveList.h"
#include "Lib/Math.h"
#include "Assert.h"

extern LoaderParams BootParams;

//Allocates physical pages from the page frame database
namespace _PMEM
{
	//Support 8GB max
	constexpr size_t MaxPages = Gigabytes(8) / Arch::PageSize;
	constexpr size_t BuddySize = 4;

	//Never give out page 0
	constexpr size_t StartBuddy = 1;

	PageFrame* const m_frames = (PageFrame*)KernelPageFrameDBStart;
	const size_t m_count = Clamp(BootParams.PageFrameCount, size_t{}, MaxPages) / BuddySize * BuddySize;
	IntrusiveList<PageFrame> m_freeList;

	//Bitvector for contiguous allocations, where False is Free.
	StaticBitset<MaxPages / BuddySize> m_buddyMap;
	size_t m_nextBuddy = StartBuddy;

	constexpr size_t GetIndex(const PageFrame* entry)
	{
		//Pointer math
		return (entry - m_frames);
	}

	/**
	 * Allocates a single page.
	 * - Note: Page might be on free list, but buddy block might already be in use
	 */
	bool Alloc(paddr_t& address)
	{
		if (m_freeList.IsEmpty())
			return false;

		PageFrame* entry = m_freeList.Pop();
		Assert(entry->State == PageState::Free);
		entry->State = PageState::Active;

		const size_t idx = GetIndex(entry);
		const size_t buddyIdx = idx / BuddySize;

		//Mark buddy as in use
		m_buddyMap.Set(buddyIdx, true);

		address = idx << Arch::PageShift;
		return true;
	}

	/**
	 * Allocates contiguous region.
	 */
	bool Alloc(paddr_t& address, const size_t count)
	{
		const size_t length = m_buddyMap.Length;
		const size_t buddyCount = DivRoundUp(count, BuddySize);

		size_t foundIdx = 0;
		if (!m_buddyMap.FirstContiguous(buddyCount, foundIdx, false, m_nextBuddy))
		{
			//Reset buddy index
			m_nextBuddy = StartBuddy;
			if (!m_buddyMap.FirstContiguous(buddyCount, foundIdx, false, m_nextBuddy))
				return false;
		}
		m_nextBuddy = foundIdx + buddyCount;
		//Printf("Buddy 0x%016x, size 0x%016x, count 0x%016x\n", m_nextBuddy, length, buddyCount);

		//Mark buddies as in use
		for (size_t i = 0; i < buddyCount; i++)
		{
			Assert(!m_buddyMap.Get(foundIdx + i));
			m_buddyMap.Set(foundIdx + i, true);
		}

		//Remove page from free list
		const paddr_t pageIdx = foundIdx * BuddySize;
		for (size_t i = 0; i < count; i++)
		{
			m_frames[pageIdx + i].State = PageState::Active;
			m_freeList.Remove(m_frames[pageIdx + i]);
		}

		address = pageIdx << Arch::PageShift;
		//Printf("Allocate: 0x%016x, 0x%x\n", address, count);
		return true;
	}

	void Free(const paddr_t address)
	{
		const size_t idx = address >> Arch::PageShift;
		Assert(idx < m_count);

		PageFrame& frame = m_frames[idx];
		Assert(frame.State == PageState::Active);
		frame.State = PageState::Free;
		m_freeList.InsertHead(frame);

		// If all pages in the buddy are now free, mark it available for contiguous allocation.
		// Reserved pages permanently hold a buddy in-use, so mixed buddies stay marked.
		const size_t buddyIdx = idx / BuddySize;
		const size_t buddyStart = buddyIdx * BuddySize;

		bool allFree = true;
		for (size_t i = 0; i < BuddySize; i++)
		{
			const size_t pageIdx = buddyStart + i;
			if (m_frames[pageIdx].State != PageState::Free)
			{
				allFree = false;
				break;
			}
		}

		if (allFree)
		{
			m_buddyMap.Set(buddyIdx, false);
			if (buddyIdx < m_nextBuddy)
				m_nextBuddy = buddyIdx;
		}
	}

	void Free(const paddr_t address, const size_t count)
	{
		for (size_t i = 0; i < count; i++)
		{
			const paddr_t page = address + (i << Arch::PageShift);
			Free(page);
		}
	}
}

/*
 * Physical memory.
 */
void KePhysicalInitialize()
{
	//Zero out memory region
	memset(_PMEM::m_frames, 0, sizeof(PageFrame) * _PMEM::m_count);

	//Initialize free list and buddy map
	_PMEM::m_freeList.Initialize();
	_PMEM::m_buddyMap.Initialize();

	//Default everything to reserved with all buddies in use. The bootloader's
	//FreeRanges then carve out the pages the allocator may hand out.
	for (size_t i = 0; i < _PMEM::m_count; i++)
		_PMEM::m_frames[i].State = PageState::Reserved;
	for (size_t i = 0; i < _PMEM::m_buddyMap.Length; i++)
		_PMEM::m_buddyMap.Set(i, true);

	//Mark free pages from the bootloader-provided ranges (sentinel-terminated)
	for (size_t i = 0; i < MaxMemoryRanges; i++)
	{
		const MemoryRange& range = BootParams.FreeRanges[i];
		if (range.PageCount == 0)
			break;
		Assert((range.Start & Arch::PageMask) == 0);

		const size_t startIdx = (range.Start >> Arch::PageShift);
		for (size_t j = 0; j < range.PageCount; j++)
		{
			const size_t pageIdx = startIdx + j;
			if (pageIdx >= _PMEM::m_count)
				break;

			PageFrame& frame = _PMEM::m_frames[pageIdx];
			frame.State = PageState::Free;
			_PMEM::m_freeList.InsertHead(frame);
		}
	}

	//A buddy is available for contiguous allocation only if all of its pages are free
	const size_t validBuddies = _PMEM::m_count / _PMEM::BuddySize;
	for (size_t b = 0; b < validBuddies; b++)
	{
		bool allFree = true;
		for (size_t k = 0; k < _PMEM::BuddySize; k++)
		{
			if (_PMEM::m_frames[b * _PMEM::BuddySize + k].State != PageState::Free)
			{
				allFree = false;
				break;
			}
		}
		_PMEM::m_buddyMap.Set(b, !allFree);
	}
}

paddr_t KePhysicalAlloc()
{
	paddr_t paddr = 0;
	Assert(_PMEM::Alloc(paddr));

	//Zero out the page
	memset(reinterpret_cast<void*>(KernelPhysicalStart + paddr), 0, Arch::PageSize);

	//Printf("KePhysicalAlloc: Addr: %p\n", paddr);
	return paddr;
}

paddr_t KePhysicalAlloc(const size_t count)
{
	paddr_t address = 0;
	Assert(_PMEM::Alloc(address, count));

	//Zero out the page
	for (size_t i = 0; i < count; i++)
	{
		const paddr_t paddr = address + (i << Arch::PageShift);
		memset(reinterpret_cast<void*>(KernelPhysicalStart + paddr), 0, Arch::PageSize);
	}

	//Printf("KePhysicalAlloc: Addr: %p, Count: %d\n", address, count);
	return address;
}

void KePhysicalFree(const paddr_t address)
{
	_PMEM::Free(address);
}

void KePhysicalFree(const paddr_t address, const size_t count)
{
	_PMEM::Free(address, count);
}
