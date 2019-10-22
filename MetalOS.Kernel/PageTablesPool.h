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
	PageTablesPool(UINT64 physicalAddress, UINT32 pageCount);

	void Activate(PageTables& tables);

	UINT64 AllocatePage();
	void DeallocatePage(UINT64 address);

private:
	bool m_index[PAGE_SIZE];
	UINT64 m_physicalAddress;
	UINT64 m_pageCount;
};

