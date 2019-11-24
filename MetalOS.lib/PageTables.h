#pragma once

#include "Kernel.h"
#include "MetalOS.h"
#include "PageTablesPool.h"

class PageTablesPool;
class PageTables
{
public:
	PageTables(UINT64 physicalAddress);

	bool MapUserPages(UINT64 virtualAddress, UINT64 physicalAddress, UINT32 count);
	bool MapKernelPages(UINT64 virtualAddress, UINT64 physicalAddress, UINT32 count);

	void SetPool(PageTablesPool* pool) { m_pool = pool; }

	void SetVirtualOffset(UINT64 virtualOffset);

	bool EnableWrite(UINT64 virtualAddress);

	//Is there a use for this besides testing?
	UINT64 ResolveAddress(UINT64 virtualAddress);
	 
private:
	bool MapPage(UINT64 virtualAddress, UINT64 physicalAddress, UINT32 count, bool global);
	bool MapPage(UINT64 virtualAddress, UINT64 physicalAddress, bool global);

	PageTablesPool* m_pool;
	UINT64 m_physicalAddress;
	UINT64 m_virtualOffset;
};

