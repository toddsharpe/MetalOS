#include "PageTables.h"
#include "Kernel.h"

#define PhysicalToVirtual(x) ((x) + m_virtualOffset)

//typedef uint64_t   UINTN;
//typedef UINTN           EFI_STATUS;
//extern UINTN
//Print(
//	const CHAR16* fmt,
//	...
//);

#define Print(x)
#define Keywait(x)

//extern EFI_STATUS Keywait(const CHAR16* String);

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
	Print(L"V: 0x%q P: 0x%q C: 0x%q G: %d VO: 0x%q\r\n", virtualAddress, physicalAddress, count, global, m_virtualOffset);
	
	//TODO: error code?
	//Assert(m_pool != nullptr);

	for (UINT32 i = 0; i < count; i++)
	{
		if (!MapPage(virtualAddress + ((UINT64)i << PAGE_SHIFT), physicalAddress + ((UINT64)i << PAGE_SHIFT), global))
			return false;
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
	UINT32 l4Index = (virtualAddress >> 39) & 0x1FF;
	Print(L"L4: 0x%q, Index: %d\r\n", l4, l4Index);

	if (l4[l4Index].Value == 0)
	{
		if (!m_pool->AllocatePage(&newPageAddress))
			return false;
		Print(L"page: 0x%q - r: 0x%q c: 0x%q\r\n", newPageAddress, &(l4[l4Index]), l4 + l4Index);
		Keywait(L"asd\r\n");

		l4[l4Index].Value = newPageAddress;
		l4[l4Index].Present = true;
		l4[l4Index].ReadWrite = true;
		l4[l4Index].UserSupervisor = true;//TODO: make this false
		l4[l4Index].Accessed = true;
		Print(L"  0x%q:", l4[l4Index].Value);
	}
	PPDPTE l3 = (PPDPTE)(PhysicalToVirtual(l4[l4Index].Value & ~0xFFF));
	UINT32 l3Index = (virtualAddress >> 30) & 0x1FF;
	Print(L"L3: 0x%q, Index: %d\r\n", l3, l3Index);
	Keywait(L"asd2\r\n");

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
		Print(L"  0x%q:", l3[l3Index].Value);
	}
	PPDE l2 = (PPDE)(PhysicalToVirtual(l3[l3Index].Value & ~0xFFF));
	Keywait(L"asd\r\n");
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
	Print(L"L4: 0x%q, P: %d, RW: %d S: %d -\r\n", l4[l4Index].Value, l4[l4Index].Present, l4[l4Index].ReadWrite, l4[l4Index].UserSupervisor);
	PPDPTE l3 = (PPDPTE)PhysicalToVirtual(l4[l4Index].Value & ~0xFFF);
	Print(L"L3: 0x%q, P: %d, RW: %d S: %d -\r\n", l3[l3Index].Value, l3[l3Index].Present, l3[l3Index].ReadWrite, l3[l3Index].UserSupervisor);
	PPDE l2 = (PPDE)PhysicalToVirtual(l3[l3Index].Value & ~0xFFF);
	Print(L"L2: 0x%q, P: %d, RW: %d S: %d -\r\n", l2[l2Index].Value, l2[l2Index].Present, l2[l2Index].ReadWrite, l2[l2Index].UserSupervisor);

	if (l2[l2Index].PageSize == 1)
	{
		//L2 maps a 2MB page
		return (l2[l2Index].Value & ~0x1FFFFF) | (virtualAddress & 0x1FFFFF);
	}

	//L2 maps a 4k page

	PPTE l1 = (PPTE)PhysicalToVirtual(l2[l2Index].Value & ~0xFFF);
	Print(L"L1: 0x%q, P: %d, RW: %d S: %d -\r\n", l1[l1Index].Value, l1[l1Index].Present, l1[l1Index].ReadWrite, l1[l1Index].UserSupervisor);
	return (l1[l1Index].Value & ~0xFFF) + offset;
}

bool PageTables::EnableWrite(UINT64 virtualAddress)
{
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
		l2[l2Index].ReadWrite = true;
	}
	else
	{
		PPTE l1 = (PPTE)PhysicalToVirtual(l2[l2Index].Value & ~0xFFF);
		l1[l1Index].ReadWrite = true;
	}

	return true;
}
