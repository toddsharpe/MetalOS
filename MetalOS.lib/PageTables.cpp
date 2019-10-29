#include "PageTables.h"
#include "Kernel.h"

#define PhysicalToVirtual(x) ((x) + m_virtualOffset)

PageTables::PageTables(UINT64 physicalAddress) : m_physicalAddress(physicalAddress), m_pool(nullptr), m_virtualOffset(0)
{
	
}

bool PageTables::MapUserPages(UINT64 virtualAddress, UINT64 physicalAddress, UINT32 count)
{
	return MapPage(virtualAddress, physicalAddress, count, false);
}

bool PageTables::MapKernelPages(UINT64 virtualAddress, UINT64 physicalAddress, UINT32 count)
{
	return MapPage(virtualAddress, physicalAddress, count, true);
}

void PageTables::SetVirtualOffset(UINT64 virtualOffset)
{
	m_virtualOffset = virtualOffset;
}

//This should be rewritten to map blocks instead of pages like this, but TODO etc
bool PageTables::MapPage(UINT64 virtualAddress, UINT64 physicalAddress, UINT32 count, bool global)
{
	//loading->WriteLineFormat("V: 0x%016x P: 0x%016x C: 0x%x G: %d", virtualAddress, physicalAddress, count, global);
	
	//TODO: error code?
	//Assert(m_pool != nullptr);

	for (UINT32 i = 0; i < count; i++)
	{
		if (!MapPage(virtualAddress + (i << PAGE_SHIFT), physicalAddress + (i << PAGE_SHIFT), global))
			return false;

		//TEST:
		return true;
	}

	return true;
}

bool PageTables::MapPage(UINT64 virtualAddress, UINT64 physicalAddress, bool global)
{
	UINT64 newPageAddress;
	UINT32 offset = virtualAddress & 0xFFF;

	//TODO: error code?
	//Assert(m_pool != nullptr);

	PPML4E l4 = (PPML4E)PhysicalToVirtual(m_physicalAddress);
	//loading->WriteLineFormat("L4: 0x%16x", l4);

	UINT32 l4Index = (virtualAddress >> 39) & 0x1FF;
	if (l4[l4Index].Value == 0)
	{
		//loading->WriteLineFormat("Making L3:");
		//Add table to L4 lookup
		
		if (!m_pool->AllocatePage(&newPageAddress))
			return false;

		l4[l4Index].Value = newPageAddress;
		l4[l4Index].Present = true;
		l4[l4Index].ReadWrite = true;
		l4[l4Index].UserSupervisor = true;//TODO: make this false
		l4[l4Index].Accessed = true;
		//loading->WriteLineFormat("  0x%16x:", l4[l4Index].Value);
	}
	PPDPTE l3 = (PPDPTE)(PhysicalToVirtual(l4[l4Index].Value & ~0xFFF));

	UINT32 l3Index = (virtualAddress >> 30) & 0x1FF;
	if (l3[l3Index].Value == 0)
	{
		if (!m_pool->AllocatePage(&newPageAddress))
			return false;
		
		//Add table to L3 lookup
		l3[l3Index].Value = newPageAddress;
		l3[l3Index].Present = true;
		l3[l3Index].ReadWrite = true;
		l3[l3Index].UserSupervisor = true;//TODO: make this false
		l3[l3Index].Accessed = true;
	}
	PPDE l2 = (PPDE)(PhysicalToVirtual(l3[l3Index].Value & ~0xFFF));

	UINT32 l2Index = (virtualAddress >> 21) & 0x1FF;
	if (l2[l2Index].Value == 0)
	{
		if (!m_pool->AllocatePage(&newPageAddress))
			return false;
		
		//Add table to L2 lookup
		l2[l2Index].Value = newPageAddress;
		l2[l2Index].Present = true;
		l2[l2Index].ReadWrite = true;
		l2[l2Index].UserSupervisor = true;//TODO: make this false
	}
	PPTE l1 = (PPTE)(PhysicalToVirtual(l2[l2Index].Value & ~0xFFF));

	UINT32 l1Index = (virtualAddress >> 12) & 0x1FF;
	//Assert(l1[l1Index].Value == 0);

	//Make the connection
	l1[l1Index].Value = physicalAddress;
	l1[l1Index].Present = true;
	l1[l1Index].ReadWrite = true;
	l1[l1Index].UserSupervisor = true;//TODO: make this false
	l1[l1Index].Global = true;
}

UINT64 PageTables::ResolveAddress(UINT64 virtualAddress)
{
	//loading->WriteLineFormat("Resolving: 0x%16x", virtualAddress);
	UINT32 offset = virtualAddress & 0xFFF;
	UINT32 l1Index = (virtualAddress >> 12) & 0x1FF;
	UINT32 l2Index = (virtualAddress >> 21) & 0x1FF;
	UINT32 l3Index = (virtualAddress >> 30) & 0x1FF;
	UINT32 l4Index = (virtualAddress >> 39) & 0x1FF;

	PPML4E l4 = (PPML4E)PhysicalToVirtual(m_physicalAddress);
	PPDPTE l3 = (PPDPTE)PhysicalToVirtual(l4[l4Index].Value & ~0xFFF);
	PPDE l2 = (PPDE)PhysicalToVirtual(l3[l3Index].Value & ~0xFFF);

	if (l2[l2Index].PageSize == 1)
	{
		//L2 maps a 2MB page
		return (l2[l2Index].Value & ~0x1FFFFF) | (virtualAddress & 0x1FFFFF);
	}

	//L2 maps a 4k page

	PPTE l1 = (PPTE)PhysicalToVirtual(l2[l2Index].Value & ~0xFFF);
	return (l1[l1Index].Value & ~0xFFF) + offset;
}
