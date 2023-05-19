#pragma once

#include "MetalOS.Kernel.h"

#include <array>
#include <vector>
#include "MemoryMap.h"
#include <kernel/LoaderParams.h>
#include "Bitvector.h"
#include <kernel/MetalOS.List.h>

//return original page state and then marks active
//Page state should be internal, then just return a bool if its zeroed
class PhysicalMemoryManager
{
public:
	PhysicalMemoryManager(MemoryMap& memoryMap);

	bool AllocatePage(paddr_t& address);
	void DeallocatePage(const paddr_t address);

	bool AllocateContiguous(paddr_t& address, const size_t pageCount);

	size_t GetSize() const;

private:
	//Construct bitvector where everybit represents group of BuddySize pages.
	static constexpr size_t BuddySize = 4;
	static constexpr size_t GetBuddyIndex(const paddr_t address);

	static PageState GetPageState(const EFI_MEMORY_DESCRIPTOR& desc);
	size_t GetIndex(const PAGE_FRAME* entry) const;

	MemoryMap& m_memoryMap;
	const size_t m_length;
	PAGE_FRAME* const m_frames;

	//Lists for quick access at runtime
	ListEntry m_freeList;

	//Bitvector for contiguous allocations, where False is Free.
	Bitvector m_buddyMap;
	size_t m_nextBuddy;
};

