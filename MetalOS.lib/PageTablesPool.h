#pragma once
#include "Kernel.h"
#include "MetalOS.h"
#include "PageTables.h"

//First page will be our index
//Gives us 1024 * 8 bits to signify pages
#define PageTablesPoolMax (PAGE_SIZE * 8)

//Class assumes region is Zeroe'd out first
class PageTables;
class PageTablesPool
{
public:
	PageTablesPool(EFI_PHYSICAL_ADDRESS physicalAddress, UINT32 pageCount);
	void SetVirtualAddress(UINT64 virtualAddress);

	void Activate(PageTables& tables);

	bool AllocatePage(UINT64 *addressOut);
	bool DeallocatePage(UINT64 address);

private:
	UINT64 m_virtualAddress;
	EFI_PHYSICAL_ADDRESS m_physicalAddress;
	UINT64 m_pageCount;
	bool* m_index;
};

