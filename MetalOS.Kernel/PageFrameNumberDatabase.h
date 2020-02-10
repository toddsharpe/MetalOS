#pragma once

#include "msvc.h"
#include <array>
#include <vector>


class PageFrameNumberDatabase
{
public:
	PageFrameNumberDatabase(size_t count);

	uintptr_t AllocatePage();

private:
	//Windows Internals, Part 2, Page 297
	enum PageLocation : uint8_t
	{
		Zeroed,
		Free,
		Standby,
		Modified,
		ModifiedNoWrite,
		Bad,
		Active,
		Transition,
	};

	//Windows Internals, Part 2, Page 316
	typedef struct _PFN_ENTRY
	{
		_PFN_ENTRY* Prev;
		_PFN_ENTRY* Next;

		uintptr_t PTEAddress;
		uint8_t ShareCount;
		PageLocation PageLocation;
		
		struct
		{
			uint8_t WriteInProgress : 1;
			uint8_t Modified : 1;
			uint8_t ReadInProgress : 1;
		} Flags;
		uint8_t Priority;//Unused

		uintptr_t PteFrame;

	} PFN_ENTRY, *PPFN_ENTRY;

	uintptr_t GetPFN(PPFN_ENTRY entry);

	std::vector<PFN_ENTRY> m_db;

	//Lists for quick access at runtime
	PPFN_ENTRY m_zeroed;
	PPFN_ENTRY m_free;
	PPFN_ENTRY m_standby;
	PPFN_ENTRY m_modifed;
};

