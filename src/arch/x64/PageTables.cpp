#include "PageTables.h"

#include <kernel/MetalOS.Internal.h>
#include "PageTablesPool.h"
#include <intrin.h>
#include <kernel/MetalOS.Internal.h>

#define Keywait(x)
//extern EFI_STATUS Keywait(const CHAR16* String);

#define LibPrint(fmt, ...) \
{ \
	if (BootLibrary.DebugPrint != NULL) \
	{ \
		BootLibrary.DebugPrint(fmt, ##__VA_ARGS__); \
	} \
} \

#define AssertPrint(fmt, ...) \
{ \
	if (BootLibrary.AssertPrint != NULL) \
	{ \
		BootLibrary.AssertPrint(fmt, ##__VA_ARGS__); \
	} \
} \

#define Assert(x) if (!(x)) { AssertPrint("Assert Failed: %s", #x); }

extern MetalOSLibrary BootLibrary;

PageTablesPool* PageTables::Pool = nullptr;

PageTables::PageTables() :
	m_physicalAddress()
{
	Assert(Pool->AllocatePage(&m_physicalAddress));
}

PageTables::PageTables(paddr_t physicalAddress) : m_physicalAddress(physicalAddress)
{

}

void PageTables::ClearKernelEntries()
{
	PPML4E currentL4 = (PPML4E)Pool->GetVirtualAddress(m_physicalAddress);

	const size_t kernelCount = (1 << 8);//high bit gets extended to full 64 bit width with 4 level paging
	const size_t count = (1 << 9);
	for (size_t i4 = kernelCount; i4 < count; i4++)
	{
		currentL4[i4].Value = 0;
	}
}

constexpr uintptr_t PageTables::BuildAddress(size_t i4, size_t i3, size_t i2, size_t i1, size_t index)
{
	uint64_t virtualAddress = (i4 << 39) + (i3 << 30) + (i2 << 21) + (i1 << 12) + index;
	if (i4 & (1 << 8)) //Sign extend
		virtualAddress |= 0xFFFFull << 48;

	return virtualAddress;
}

uintptr_t PageTables::GetCr3() const
{
	return m_physicalAddress;
}

void PageTables::Display()
{
	const size_t count = (1 << 9);

	LibPrint("CR3: 0x%016x\r\n", m_physicalAddress);

	PPML4E l4 = (PPML4E)Pool->GetVirtualAddress(m_physicalAddress);
	for (size_t i4 = 0; i4 < count; i4++)
	{
		if (l4[i4].Value == 0)
			continue;

		uint64_t virtualAddress = BuildAddress(i4, 0, 0, 0);
		LibPrint("  - 0x%016x (0x%016x) P: %d, RW: %d S: %d\r\n", l4[i4].Value, virtualAddress, l4[i4].Present, l4[i4].ReadWrite, l4[i4].UserSupervisor);

		PPDPTE_DIR l3 = (PPDPTE_DIR)(Pool->GetVirtualAddress(l4[i4].Value & ~0xFFF));
		if (l3->PageSize)
		{
			PPDPTE_PAGE l3Page = (PPDPTE_PAGE)l3;
			LibPrint("PageFrame: 0x%016x\r\n", l3Page->PageFrameNumber);
		}
		else
		{
			for (size_t i3 = 0; i3 < count; i3++)
			{
				if (l3[i3].Value == 0)
					continue;

				uint64_t virtualAddress = BuildAddress(i4, i3, 0, 0);
				LibPrint("    - 0x%016x (0x%016x) P: %d, RW: %d S: %d\r\n", l3[i3].Value, virtualAddress, l3[i3].Present, l3[i3].ReadWrite, l3[i3].UserSupervisor);

				PPDE_DIR l2 = (PPDE_DIR)(Pool->GetVirtualAddress(l3[i3].Value & ~0xFFF));
				for (size_t i2 = 0; i2 < count; i2++)
				{
					if (l2[i2].Value == 0)
						continue;

					uint64_t virtualAddress = BuildAddress(i4, i3, i2, 0);
					LibPrint("      - 0x%016x (0x%016x) P: %d, RW: %d S: %d\r\n", l2[i2].Value, virtualAddress, l2[i2].Present, l2[i2].ReadWrite, l2[i2].UserSupervisor);

					PPTE l1 = (PPTE)(Pool->GetVirtualAddress(l2[i2].Value & ~0xFFF));
					for (size_t i1 = 0; i1 < count; i1++)
					{
						if (l1[i1].Value == 0)
							continue;

						uint64_t virtualAddress = BuildAddress(i4, i3, i2, i1);
						LibPrint("        - 0x%016x (0x%016x) P: %d, RW: %d S: %d\r\n", l1[i1].Value, virtualAddress, l1[i1].Present, l1[i1].ReadWrite, l1[i1].UserSupervisor);
					}
				}
			}
		}
	}
}

void PageTables::DisplayCr3()
{
	const size_t count = (1 << 9);
	PPML4E l4 = (PPML4E)Pool->GetVirtualAddress(m_physicalAddress);
	LibPrint("CR3: 0x%016x\n", m_physicalAddress);
	for (size_t i4 = 0; i4 < count; i4++)
	{
		if (l4[i4].Value == 0)
			continue;

		const uint64_t virtualAddress = BuildAddress(i4, 0, 0, 0);
		LibPrint("0x%x - 0x%016x (0x%016x) P: %d, RW: %d S: %d\n", i4, l4[i4].Value, virtualAddress, l4[i4].Present, l4[i4].ReadWrite, l4[i4].UserSupervisor);
	}
}

void PageTables::LoadKernelMappings(PageTables* copyPt)
{
	PPML4E currentL4 = (PPML4E)Pool->GetVirtualAddress(m_physicalAddress);
	PPML4E copyL4 = (PPML4E)copyPt->Pool->GetVirtualAddress(copyPt->m_physicalAddress);

	const size_t kernelCount = (1 << 8);//high bit gets extended to full 64 bit width with 4 level paging
	const size_t count = (1 << 9);
	for (size_t i4 = kernelCount; i4 < count; i4++)
	{
		if (copyL4[i4].Value == 0)
			continue;

		currentL4[i4].Value = copyL4[i4].Value;
	}
}

bool PageTables::MapUserPages(uintptr_t virtualAddress, uintptr_t physicalAddress, size_t count)
{
	return MapPage(virtualAddress, physicalAddress, count, false);
}

bool PageTables::MapKernelPages(uintptr_t virtualAddress, uintptr_t physicalAddress, size_t count)
{
	return MapPage(virtualAddress, physicalAddress, count, true);
}

//This should be rewritten to map blocks instead of pages like this, but TODO etc
//TODO: page attributes!
bool PageTables::MapPage(uintptr_t virtualAddress, uintptr_t physicalAddress, size_t count, bool global)
{
	LibPrint("V: 0x%016x P: 0x%016x C: 0x%x G: %d\r\n", virtualAddress, physicalAddress, count, global);

	//TODO: error code?
	//Assert(Pool != nullptr);

	for (size_t i = 0; i < count; i++)
	{
		if (!MapPage(virtualAddress + ((uintptr_t)i << PAGE_SHIFT), physicalAddress + ((uintptr_t)i << PAGE_SHIFT), global))
			return false;
	}

	return true;
}

bool PageTables::MapPage(uintptr_t virtualAddress, uintptr_t physicalAddress, bool global)
{
	uintptr_t newPageAddress;
	uint32_t offset = virtualAddress & 0xFFF;

	LibPrint("V: 0x%016x P: 0x%016x\r\n", virtualAddress, physicalAddress);

	//TODO: error code?
	//Assert(Pool != nullptr);

	PPML4E l4 = (PPML4E)Pool->GetVirtualAddress(m_physicalAddress);
	uint32_t l4Index = (virtualAddress >> 39) & 0x1FF;
	LibPrint("L4: 0x%016x, Index: %d, V: 0x%016x\r\n", l4, l4Index, l4[l4Index].Value);

	if (l4[l4Index].Value == 0)
	{
		if (!Pool->AllocatePage(&newPageAddress))
			return false;
		//LibPrint("page: 0x%016x - r: 0x%016x c: 0x%016x\r\n", newPageAddress, &(l4[l4Index]), l4 + l4Index);
		Keywait(L"asd\r\n");

		l4[l4Index].Value = newPageAddress;
		l4[l4Index].Present = true;
		l4[l4Index].ReadWrite = true;
		l4[l4Index].UserSupervisor = !global;
		l4[l4Index].Accessed = true;
		//LibPrint("  0x%016x:", l4[l4Index].Value);
	}
	PPDPTE_DIR l3 = (PPDPTE_DIR)(Pool->GetVirtualAddress(l4[l4Index].Value & ~0xFFF));
	uint32_t l3Index = (virtualAddress >> 30) & 0x1FF;
	//LibPrint("L3: 0x%016x, Index: %d, V: 0x%016x\r\n", l3, l3Index, l3[l3Index].Value);
	Keywait(L"asd2\r\n");

	if (l3[l3Index].Value == 0)
	{
		if (!Pool->AllocatePage(&newPageAddress))
			return false;
		//LibPrint("page: 0x%016x - r: 0x%016x c: 0x%016x\r\n", newPageAddress, &(l3[l3Index]), l3 + l3Index);

		//Add table to L3 lookup
		l3[l3Index].Value = newPageAddress;
		l3[l3Index].Present = true;
		l3[l3Index].ReadWrite = true;
		l3[l3Index].UserSupervisor = !global;
		l3[l3Index].Accessed = true;
		//LibPrint("  0x%016x:", l3[l3Index].Value);
	}
	PPDE_DIR l2 = (PPDE_DIR)(Pool->GetVirtualAddress(l3[l3Index].Value & ~0xFFF));
	Keywait(L"asd\r\n");
	uint32_t l2Index = (virtualAddress >> 21) & 0x1FF;
	//LibPrint("L2: 0x%016x, Index: %d, V: 0x%016x\r\n", l2, l2Index, l2[l2Index].Value);

	if (l2[l2Index].Value == 0)
	{
		if (!Pool->AllocatePage(&newPageAddress))
			return false;

		//Add table to L2 lookup
		l2[l2Index].Value = newPageAddress;
		l2[l2Index].Present = true;
		l2[l2Index].ReadWrite = true;
		l2[l2Index].UserSupervisor = !global;
		//LibPrint("  0x%016x:", l2[l2Index].Value);
	}
	PPTE l1 = (PPTE)(Pool->GetVirtualAddress(l2[l2Index].Value & ~0xFFF));

	uint32_t l1Index = (virtualAddress >> 12) & 0x1FF;
	//LibPrint("L1: 0x%016x, Index: %d, V: 0x%016x\r\n", l1, l1Index, l1[l1Index].Value);
	//Assert(l1[l1Index].Value == 0);

	//Make the connection
	l1[l1Index].Value = physicalAddress;
	l1[l1Index].Present = true;
	l1[l1Index].ReadWrite = true;
	l1[l1Index].UserSupervisor = !global;//TODO: make this false
	l1[l1Index].Global = global;

	return true;
}

uintptr_t PageTables::ResolveAddress(uintptr_t virtualAddress)
{
	//loading->WriteLineFormat("Resolving: 0x%16x", virtualAddress);
	uint32_t offset = virtualAddress & 0xFFF;
	uint32_t l1Index = (virtualAddress >> 12) & 0x1FF;
	uint32_t l2Index = (virtualAddress >> 21) & 0x1FF;
	uint32_t l3Index = (virtualAddress >> 30) & 0x1FF;
	uint32_t l4Index = (virtualAddress >> 39) & 0x1FF;

	PPML4E l4 = (PPML4E)Pool->GetVirtualAddress(m_physicalAddress);
	LibPrint("L4: 0x%016x, P: %d, RW: %d S: %d -\r\n", l4[l4Index].Value, l4[l4Index].Present, l4[l4Index].ReadWrite, l4[l4Index].UserSupervisor);
	PPDPTE_DIR l3 = (PPDPTE_DIR)Pool->GetVirtualAddress(l4[l4Index].Value & ~0xFFF);
	LibPrint("L3: 0x%016x, P: %d, RW: %d S: %d -\r\n", l3[l3Index].Value, l3[l3Index].Present, l3[l3Index].ReadWrite, l3[l3Index].UserSupervisor);
	PPDE_DIR l2 = (PPDE_DIR)Pool->GetVirtualAddress(l3[l3Index].Value & ~0xFFF);
	LibPrint("L2: 0x%016x, P: %d, RW: %d S: %d -\r\n", l2[l2Index].Value, l2[l2Index].Present, l2[l2Index].ReadWrite, l2[l2Index].UserSupervisor);

	if (l2[l2Index].PageSize == 1)
	{
		//L2 maps a 2MB page
		return (l2[l2Index].Value & ~0x1FFFFF) | (virtualAddress & 0x1FFFFF);
	}

	//L2 maps a 4k page

	PPTE l1 = (PPTE)Pool->GetVirtualAddress(l2[l2Index].Value & ~0xFFF);
	LibPrint("L1: 0x%016x, P: %d, RW: %d S: %d -\r\n", l1[l1Index].Value, l1[l1Index].Present, l1[l1Index].ReadWrite, l1[l1Index].UserSupervisor);

	return (l1[l1Index].Value & ~0xFFF) + offset;
}

bool PageTables::EnableWrite(uintptr_t virtualAddress)
{
	uint32_t offset = virtualAddress & 0xFFF;
	uint32_t l1Index = (virtualAddress >> 12) & 0x1FF;
	uint32_t l2Index = (virtualAddress >> 21) & 0x1FF;
	uint32_t l3Index = (virtualAddress >> 30) & 0x1FF;
	uint32_t l4Index = (virtualAddress >> 39) & 0x1FF;

	PPML4E l4 = (PPML4E)Pool->GetVirtualAddress(m_physicalAddress);
	PPDPTE_DIR l3 = (PPDPTE_DIR)Pool->GetVirtualAddress(l4[l4Index].Value & ~0xFFF);
	PPDE_DIR l2 = (PPDE_DIR)Pool->GetVirtualAddress(l3[l3Index].Value & ~0xFFF);

	if (l2[l2Index].PageSize == 1)
	{
		l2[l2Index].ReadWrite = true;
	}
	else
	{
		PPTE l1 = (PPTE)Pool->GetVirtualAddress(l2[l2Index].Value & ~0xFFF);
		l1[l1Index].ReadWrite = true;
	}

	return true;
}
