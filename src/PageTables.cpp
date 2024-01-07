#include "PageTables.h"

#include "PageTablesPool.h"
#include "Assert.h"
#include <intrin.h>
#include <string>

PageTablesPool* PageTables::Pool = nullptr;
bool PageTables::Debug = false;

PageTables::PageTables() :
	m_root()
{
	
}

PageTables::PageTables(const paddr_t root) : m_root(root)
{

}

void PageTables::OpenCurrent()
{
	AssertOp(m_root, ==, 0);
	m_root = __readcr3();
}

void PageTables::CreateNew()
{
	Assert(Pool->AllocatePage(m_root));
}

paddr_t PageTables::GetRoot() const
{
	return m_root;
}

bool PageTables::IsActive() const
{
	return __readcr3() == m_root;
}

//TODO(tsharpe): Rewrite to use larger page blocks (2MB, 1GB)
bool PageTables::MapPages(const uintptr_t virtualBase, const paddr_t physicalBase, const size_t count, const bool global) const
{
	CPrintf(Debug, "V: 0x%016x P: 0x%016x C: 0x%x G: %d\r\n", virtualBase, physicalBase, count, global);

	Assert(Pool);
	Assert(m_root);

	for (size_t i = 0; i < count; i++)
	{
		const uintptr_t offset = (i << PageShift);
		if (!MapPage(virtualBase + offset, physicalBase + offset, global))
			return false;
	}

	return true;
}

paddr_t PageTables::ResolveAddress(const uintptr_t virtualAddress) const
{
	//loading->WriteLineFormat("Resolving: 0x%16x", virtualAddress);

	VirtualAddress addr;
	addr.AsUint64 = virtualAddress;

	const PPML4E map4 = (PPML4E)Pool->GetVirtualAddress(m_root);
	const PML4E level4 = map4[addr.index4];
	CPrintf(Debug, "L4: 0x%016x, P: %d, RW: %d S: %d -\r\n", level4.Value, level4.Present, level4.ReadWrite, level4.UserSupervisor);

	const PPDPTE_DIR map3 = (PPDPTE_DIR)Pool->GetVirtualAddress(level4.Value & ~0xFFF);
	const PDPTE_DIR level3 = map3[addr.index3];
	CPrintf(Debug, "L3: 0x%016x, P: %d, RW: %d S: %d -\r\n", level3.Value, level3.Present, level3.ReadWrite, level3.UserSupervisor);

	const PPDE_DIR map2 = (PPDE_DIR)Pool->GetVirtualAddress(level3.Value & ~0xFFF);
	const PDE_DIR level2 = map2[addr.index2];
	CPrintf(Debug, "L2: 0x%016x, P: %d, RW: %d S: %d -\r\n", level2.Value, level2.Present, level2.ReadWrite, level2.UserSupervisor);
	if (level2.PageSize == 1)
	{
		//L2 maps a 2MB page
		return (level2.Value & ~0x1FFFFF) | (virtualAddress & 0x1FFFFF);
	}

	//L2 maps a 4k page
	const PPTE map1 = (PPTE)Pool->GetVirtualAddress(level2.Value & ~0xFFF);
	const PTE level1 = map1[addr.index1];
	CPrintf(Debug, "L1: 0x%016x, P: %d, RW: %d S: %d -\r\n", level1.Value, level1.Present, level1.ReadWrite, level1.UserSupervisor);

	return (level1.Value & ~0xFFF) + addr.offset;
}

void PageTables::ClearKernelEntries() const
{
	PPML4E currentL4 = (PPML4E)Pool->GetVirtualAddress(m_root);

	const size_t kernelCount = (1 << 8);//high bit gets extended to full 64 bit width with 4 level paging
	const size_t count = (1 << 9);
	for (size_t i4 = kernelCount; i4 < count; i4++)
	{
		currentL4[i4].Value = 0;
	}
}

void PageTables::LoadKernelMappings() const
{
	uintptr_t* const root = (uintptr_t*)Pool->GetVirtualAddress(m_root);

	PageTables current;
	current.OpenCurrent();
	uintptr_t* const currentRoot = (uintptr_t*)Pool->GetVirtualAddress(current.GetRoot());

	//Kernel addresses start at high bit set, occupy half of root page
	const size_t kernelStart = (1 << 8);
	static_assert(kernelStart * sizeof(uintptr_t) == PageSize / 2, "Invalid region");

	//copy all mappings from current PT's root to this PT's root
	memcpy(&root[kernelStart], &currentRoot[kernelStart], PageSize / 2);
}

void PageTables::Display()
{
	const size_t count = PageSize / sizeof(uintptr_t);

	Printf("Root: 0x%016x\r\n", m_root);

	const PPML4E l4 = (PPML4E)Pool->GetVirtualAddress(m_root);
	for (size_t i4 = 0; i4 < count; i4++)
	{
		if (l4[i4].Value == 0)
			continue;

		const uint64_t virtualAddress = BuildAddress(i4, 0, 0, 0, 0);
		Printf("  - 0x%016x (0x%016x) P: %d, RW: %d S: %d\r\n", l4[i4].Value, virtualAddress, l4[i4].Present, l4[i4].ReadWrite, l4[i4].UserSupervisor);

		const PPDPTE_DIR l3 = (PPDPTE_DIR)(Pool->GetVirtualAddress(l4[i4].Value & ~0xFFF));
		if (l3->PageSize)
		{
			const PPDPTE_PAGE l3Page = (PPDPTE_PAGE)l3;
			Printf("PageFrame: 0x%016x\r\n", l3Page->PageFrameNumber);
		}
		else
		{
			for (size_t i3 = 0; i3 < count; i3++)
			{
				if (l3[i3].Value == 0)
					continue;

				const uint64_t virtualAddress = BuildAddress(i4, i3, 0, 0, 0);
				Printf("    - 0x%016x (0x%016x) P: %d, RW: %d S: %d\r\n", l3[i3].Value, virtualAddress, l3[i3].Present, l3[i3].ReadWrite, l3[i3].UserSupervisor);

				const PPDE_DIR l2 = (PPDE_DIR)(Pool->GetVirtualAddress(l3[i3].Value & ~0xFFF));
				for (size_t i2 = 0; i2 < count; i2++)
				{
					if (l2[i2].Value == 0)
						continue;

					const uint64_t virtualAddress = BuildAddress(i4, i3, i2, 0, 0);
					Printf("      - 0x%016x (0x%016x) P: %d, RW: %d S: %d\r\n", l2[i2].Value, virtualAddress, l2[i2].Present, l2[i2].ReadWrite, l2[i2].UserSupervisor);

					const PPTE l1 = (PPTE)(Pool->GetVirtualAddress(l2[i2].Value & ~0xFFF));
					for (size_t i1 = 0; i1 < count; i1++)
					{
						if (l1[i1].Value == 0)
							continue;

						const uint64_t virtualAddress = BuildAddress(i4, i3, i2, i1, 0);
						Printf("        - 0x%016x (0x%016x) P: %d, RW: %d S: %d\r\n", l1[i1].Value, virtualAddress, l1[i1].Present, l1[i1].ReadWrite, l1[i1].UserSupervisor);
					}
				}
			}
		}
	}
}

void PageTables::DisplayRoot() const
{
	Printf("DisplayRoot\r\n");
	Printf("    Root: 0x%016x\r\n", m_root);

	const size_t count = PageSize / sizeof(uintptr_t);
	const PPML4E l4 = (PPML4E)Pool->GetVirtualAddress(m_root);
	for (size_t i4 = 0; i4 < count; i4++)
	{
		if (l4[i4].Value == 0)
			continue;

		const uint64_t virtualAddress = BuildAddress(i4, 0, 0, 0, 0);
		Printf("0x%x - 0x%016x (0x%016x) P: %d, RW: %d S: %d\r\n", i4, l4[i4].Value, virtualAddress, l4[i4].Present, l4[i4].ReadWrite, l4[i4].UserSupervisor);
	}
}

bool PageTables::MapPage(const uintptr_t virtualAddress, const paddr_t physicalAddress, const bool global) const
{
	CPrintf(Debug, "MapPage-V: 0x%016x P: 0x%016x G: %d\r\n", virtualAddress, physicalAddress, global);
	
	Assert(Pool != nullptr);
	Assert(m_root);

	VirtualAddress addr;
	addr.AsUint64 = virtualAddress;

	PPML4E map4 = (PPML4E)Pool->GetVirtualAddress(m_root);
	PML4E& level4 = map4[addr.index4];
	CPrintf(Debug, "L4: 0x%016x, Index: %d, V: 0x%016x\r\n", map4, addr.index4, level4.Value);
	if (level4.Value == 0)
	{
		paddr_t allocated;
		Assert(Pool->AllocatePage(allocated));
		CPrintf(Debug, "  page: 0x%016x\r\n", allocated);

		level4.Value = allocated;
		level4.Present = true;
		level4.ReadWrite = true;
		level4.UserSupervisor = !global;
		level4.Accessed = true;
		CPrintf(Debug, "  0x%016x:\r\n", level4.Value);
	}

	PPDPTE_DIR map3 = (PPDPTE_DIR)(Pool->GetVirtualAddress(level4.Value & ~0xFFF));
	PDPTE_DIR& level3 = map3[addr.index3];
	CPrintf(Debug, "L3: 0x%016x, Index: %d, V: 0x%016x\r\n", map3, addr.index3, level3.Value);
	if (level3.Value == 0)
	{
		paddr_t allocated;
		Assert(Pool->AllocatePage(allocated));
		CPrintf(Debug, "  page: 0x%016x\r\n", allocated);

		level3.Value = allocated;
		level3.Present = true;
		level3.ReadWrite = true;
		level3.UserSupervisor = !global;
		level3.Accessed = true;
		CPrintf(Debug, "  0x%016x:\r\n", level4.Value);
	}

	PPDE_DIR map2 = (PPDE_DIR)(Pool->GetVirtualAddress(level3.Value & ~0xFFF));
	PDE_DIR& level2 = map2[addr.index2];
	CPrintf(Debug, "L2: 0x%016x, Index: %d, V: 0x%016x\r\n", map2, addr.index2, level2.Value);
	if (level2.Value == 0)
	{
		paddr_t allocated;
		Assert(Pool->AllocatePage(allocated));
		CPrintf(Debug, "  page: 0x%016x\r\n", allocated);

		level2.Value = allocated;
		level2.Present = true;
		level2.ReadWrite = true;
		level2.UserSupervisor = !global;
		CPrintf(Debug, "  0x%016x:\r\n", level4.Value);
	}

	PPTE map1 = (PPTE)(Pool->GetVirtualAddress(level2.Value & ~0xFFF));
	PTE& level1 = map1[addr.index1];
	CPrintf(Debug, "L1: 0x%016x, Index: %d, V: 0x%016x\r\n", map1, addr.index1, level1.Value);
	level1.Value = physicalAddress;
	level1.Present = true;
	level1.ReadWrite = true;
	level1.UserSupervisor = !global;
	level1.Global = global;

	return true;
}

uintptr_t PageTables::BuildAddress(const size_t i4, const size_t i3, const size_t i2, const size_t i1, const size_t offset) const
{
	VirtualAddress addr = { offset, i1, i2, i3, i4, 0 };
	if (i4 & (1 << 8)) //Sign extend
		addr.upper = 0xFFFF;

	return addr.AsUint64;
}
