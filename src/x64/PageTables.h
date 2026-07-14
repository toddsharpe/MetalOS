#pragma once

#include "x64/x64.h"

static constexpr bool IsAligned(const uintptr_t address, const size_t powerOf2)
{
	return (address & (powerOf2 - 1)) == 0;
}

struct PageTablesOps
{
	using resolve_t = void *(*)(const paddr_t address);
	using phy_alloc_t = bool (*)(paddr_t &address);

	resolve_t Resolve;
	phy_alloc_t PhyAlloc;
};

struct PageAttr
{
	bool CanWrite;
	bool CanExecute;
	bool IsGlobal;
};

//TODO(tsharpe): Kernel is mapped at once and therefore needs all permissions
static constexpr PageAttr KernelAll =
{
	.CanWrite = true,
	.CanExecute = true,
	.IsGlobal = true
};

static constexpr PageAttr UserAll =
{
	.CanWrite = true,
	.CanExecute = true,
	.IsGlobal = false
};

/*
static constexpr PageAttr KernelCode =
{
	.CanWrite = false,
	.CanExecute = true,
	.IsGlobal = true
};

static constexpr PageAttr KernelData =
{
	.CanWrite = true,
	.CanExecute = false,
	.IsGlobal = true
};
*/
template <PageTablesOps T_ops>
class PageTabes
{
public:
	static PageTabes Current()
	{
		return PageTabes(__readcr3());
	}

	static PageTabes CreateNew()
	{
		paddr_t root;
		if (!T_ops.PhyAlloc(root))
			return PageTabes(0);
		return PageTabes(root);
	}

	constexpr PageTabes(const paddr_t root = 0) :
		Root(root)
	{
	}

	bool MapPages(const uintptr_t virtualBase, const paddr_t physicalBase, const size_t count, const PageAttr attr) const
	{
		return MapPages(virtualBase, physicalBase, count, attr.CanWrite, attr.CanExecute, attr.IsGlobal);
	}

	bool MapPages(const uintptr_t virtualBase, const paddr_t physicalBase, const size_t count, const bool write, const bool execute, const bool global) const
	{
		using namespace x64;

		static constexpr size_t Pages2M = 512;		 // 2MB in 4KB pages
		static constexpr size_t Pages1G = 512 * 512; // 1GB in 4KB pages

		size_t remaining = count;
		uintptr_t virt = virtualBase;
		paddr_t phys = physicalBase;

		while (remaining > 0)
		{
			const VirtualAddress va =
			{
				.AsUint64 = virt
			};

			// PML4 — always needed
			PML4E *const pml4 = static_cast<PML4E *>(T_ops.Resolve(Root));
			PML4E &pml4e = pml4[va.index4];
			if (!pml4e.Present)
			{
				paddr_t table;
				if (!T_ops.PhyAlloc(table))
					return false;
				pml4e.Value = table;
				pml4e.Present = true;
				pml4e.ReadWrite = true;
				pml4e.UserSupervisor = !global;
			}

			// PDPT
			PDPTE_DIR *const pdpt = static_cast<PDPTE_DIR *>(T_ops.Resolve(pml4e.Value & ~0xFFFull));

			if (remaining >= Pages1G && IsAligned(virt, Pages1G * PageSize) && IsAligned(phys, Pages1G * PageSize))
			{
				// 1GB page — leaf at PDPT level
				PDPTE_PAGE page;
				page.Value = phys; // 1GB-aligned: bits 0-29 are 0, PFN lands at bits 30-47
				page.Present = true;
				page.ReadWrite = write;
				page.ExecuteDisable = !execute;
				page.Global = global;
				page.UserSupervisor = !global;
				page.PageSize = true;
				pdpt[va.index3].Value = page.Value;

				virt += Pages1G * PageSize;
				phys += Pages1G * PageSize;
				remaining -= Pages1G;
				continue;
			}

			PDPTE_DIR &pdpte = pdpt[va.index3];
			if (!pdpte.Present)
			{
				paddr_t table;
				if (!T_ops.PhyAlloc(table))
					return false;
				pdpte.Value = table;
				pdpte.Present = true;
				pdpte.ReadWrite = true;
				pdpte.UserSupervisor = !global;
			}

			// PD
			PDE_DIR *const pd = static_cast<PDE_DIR *>(T_ops.Resolve(pdpte.Value & ~0xFFFull));

			if (remaining >= Pages2M && IsAligned(virt, Pages2M * PageSize) && IsAligned(phys, Pages2M * PageSize))
			{
				// 2MB page — leaf at PD level
				PDE_PAGE page;
				page.Value = phys; // 2MB-aligned: bits 0-20 are 0, PFN lands at bits 21-47
				page.Present = true;
				page.ReadWrite = write;
				page.ExecuteDisable = !execute;
				page.Global = global;
				page.UserSupervisor = !global;
				page.PageSize = true;
				pd[va.index2].Value = page.Value;

				virt += Pages2M * PageSize;
				phys += Pages2M * PageSize;
				remaining -= Pages2M;
				continue;
			}

			PDE_DIR &pde = pd[va.index2];
			if (!pde.Present)
			{
				paddr_t table;
				if (!T_ops.PhyAlloc(table))
					return false;
				pde.Value = table;
				pde.Present = true;
				pde.ReadWrite = true;
				pde.UserSupervisor = !global;
			}

			// PT — 4KB page
			PTE *const pt = static_cast<PTE *>(T_ops.Resolve(pde.Value & ~0xFFFull));
			PTE &pte = pt[va.index1];
			pte.Value = phys; // 4KB-aligned: bits 0-11 are 0, PFN lands at bits 12-47
			pte.Present = true;
			pte.ReadWrite = write;
			pte.ExecuteDisable = !execute;
			pte.Global = global;
			pte.UserSupervisor = !global;

			virt += PageSize;
			phys += PageSize;
			remaining -= 1;
		}

		return true;
	}

	bool UnmapPages(const uintptr_t virtualBase, const size_t count) const
	{
		using namespace x64;

		for (size_t i = 0; i < count; i++)
		{
			const uintptr_t virt = virtualBase + (i << PageShift);
			const VirtualAddress va = { .AsUint64 = virt };

			PML4E* const pml4 = static_cast<PML4E*>(T_ops.Resolve(Root));
			if (!pml4[va.index4].Present)
				continue;

			PDPTE_DIR* const pdpt = static_cast<PDPTE_DIR*>(T_ops.Resolve(pml4[va.index4].Value & ~0xFFFull));
			if (!pdpt[va.index3].Present)
				continue;
			if (pdpt[va.index3].PageSize)
			{
				pdpt[va.index3].Value = 0;
				__invlpg(reinterpret_cast<void*>(virt));
				continue;
			}

			PDE_DIR* const pd = static_cast<PDE_DIR*>(T_ops.Resolve(pdpt[va.index3].Value & ~0xFFFull));
			if (!pd[va.index2].Present)
				continue;
			if (pd[va.index2].PageSize)
			{
				pd[va.index2].Value = 0;
				__invlpg(reinterpret_cast<void*>(virt));
				continue;
			}

			PTE* const pt = static_cast<PTE*>(T_ops.Resolve(pd[va.index2].Value & ~0xFFFull));
			pt[va.index1].Value = 0;
			__invlpg(reinterpret_cast<void*>(virt));
		}

		return true;
	}

	void ClearKernelEntries() const
	{
		using namespace x64;
		
		PML4E *const pml4 = static_cast<PML4E *>(T_ops.Resolve(Root));

		const size_t kernelCount = (1 << 8); // high bit gets extended to full 64 bit width with 4 level paging
		const size_t count = (1 << 9);
		for (size_t i4 = kernelCount; i4 < count; i4++)
		{
			pml4[i4].Value = 0;
		}
	}

	void LoadKernelMappings() const
	{
		using namespace x64;
		
		uintptr_t *const root = (uintptr_t *)T_ops.Resolve(Root);

		PageTabes<T_ops> current = PageTabes<T_ops>::Current();
		uintptr_t *const currentRoot = (uintptr_t *)T_ops.Resolve(current.Root);

		// Kernel addresses start at high bit set, occupy half of root page
		constexpr size_t kernelStart = (1 << 8);
		static_assert(kernelStart * sizeof(uintptr_t) == PageSize / 2, "Invalid region");

		// copy all mappings from current PT's root to this PT's root
		memcpy(&root[kernelStart], &currentRoot[kernelStart], PageSize / 2);
	}

	bool IsValid(const void* virtAddr) const
	{
		using namespace x64;

		const VirtualAddress va = { .AsUint64 = reinterpret_cast<uint64_t>(virtAddr) };

		PML4E* const pml4 = static_cast<PML4E*>(T_ops.Resolve(Root));
		if (!pml4[va.index4].Present)
			return false;

		PDPTE_DIR* const pdpt = static_cast<PDPTE_DIR*>(T_ops.Resolve(pml4[va.index4].Value & ~0xFFFull));
		if (!pdpt[va.index3].Present)
			return false;
		if (pdpt[va.index3].PageSize) // 1GB page
			return true;

		PDE_DIR* const pd = static_cast<PDE_DIR*>(T_ops.Resolve(pdpt[va.index3].Value & ~0xFFFull));
		if (!pd[va.index2].Present)
			return false;
		if (pd[va.index2].PageSize) // 2MB page
			return true;

		PTE* const pt = static_cast<PTE*>(T_ops.Resolve(pd[va.index2].Value & ~0xFFFull));
		return pt[va.index1].Present;
	}

	bool IsValid() const
	{
		return Root != 0;
	}

	//Translate a virtual address to its physical address in these tables (0 if unmapped).
	//Includes the in-page offset; handles 4KB/2MB/1GB leaf pages.
	paddr_t ResolvePhysical(const void* const virtAddr) const
	{
		using namespace x64;

		const uint64_t addr = reinterpret_cast<uint64_t>(virtAddr);
		const VirtualAddress va = { .AsUint64 = addr };

		PML4E* const pml4 = static_cast<PML4E*>(T_ops.Resolve(Root));
		if (!pml4[va.index4].Present)
			return 0;

		PDPTE_DIR* const pdpt = static_cast<PDPTE_DIR*>(T_ops.Resolve(pml4[va.index4].Value & ~0xFFFull));
		if (!pdpt[va.index3].Present)
			return 0;
		if (pdpt[va.index3].PageSize) // 1GB leaf
			return (pdpt[va.index3].Value & 0x000FFFFFC0000000ull) | (addr & 0x3FFFFFFFull);

		PDE_DIR* const pd = static_cast<PDE_DIR*>(T_ops.Resolve(pdpt[va.index3].Value & ~0xFFFull));
		if (!pd[va.index2].Present)
			return 0;
		if (pd[va.index2].PageSize) // 2MB leaf
			return (pd[va.index2].Value & 0x000FFFFFFFE00000ull) | (addr & 0x1FFFFFull);

		PTE* const pt = static_cast<PTE*>(T_ops.Resolve(pd[va.index2].Value & ~0xFFFull));
		if (!pt[va.index1].Present)
			return 0;
		return (pt[va.index1].Value & 0x000FFFFFFFFFF000ull) | (addr & 0xFFFull); // 4KB leaf
	}

	paddr_t Root;
};
