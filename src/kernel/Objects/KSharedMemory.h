#pragma once

#include "kernel/Types.h"
#include "kernel/Objects/KRef.h"

// A block of contiguous physical pages shared between processes. Reached only via
// the handles/grants that reference it (no global id); freed when the last handle
// reference drops (see KeShmUnref).
class KSharedMemory
{
public:
	constexpr KSharedMemory(const paddr_t physical, const size_t pageCount) :
		Physical(physical),
		PageCount(pageCount),
		Refs()
	{
	}

	const paddr_t Physical;
	const size_t PageCount;
	KRef Refs;
};
