#include "kernel/Api.h"
#include "kernel/KProcess.h"
#include "kernel/Arch.h"
#include "x64/x64.h"
#include "x64/PageTables.h"
#include "MetalOS.Space.h"
#include "Assert.h"

//Kernel globals (defined in Kernel.cpp).
extern KProcess m_process;

//Reserves space in a process, acquires physical pages, maps into its page tables
namespace _VMEM
{
	void* Allocate(KProcess& process, const size_t count)
	{
		CPrintf(process.Debug, "Allocate: Any, Count: 0x%x Global: %d\n", count, process.IsGlobal);
		Assert(count);

		//Reserve region, returning nullptr if it isn't free
		const uintptr_t reserved = process.Space.Reserve(count);
		if (!reserved)
			return nullptr;

		//Map into the process's page tables
		const PageAttr attributes = process.IsGlobal ? KernelAll : UserAll;
		for (size_t i = 0; i < count; i++)
		{
			const paddr_t paddr = KePhysicalAlloc();
			Assert(paddr)
			const uintptr_t virt = reserved + (i << Arch::PageShift);
			Assert(process.Tables.MapPages(virt, paddr, 1, attributes));
		}

		return (void*)reserved;
	}

	void* Allocate(KProcess& process, const void* address, const size_t count)
	{
		CPrintf(process.Debug, "Allocate: 0x%016x, Count: 0x%x Global: %d\n", address, count, process.IsGlobal);
		Assert(address);
		Assert(count);

		//Reserve region, returning nullptr if it isn't free
		const uintptr_t reserved = reinterpret_cast<uintptr_t>(address);
		if (!process.Space.Reserve(reserved, count))
			return nullptr;

		//Map into the process's page tables
		//TODO(tsharpe): Support read/execute only
		const PageAttr attributes = process.IsGlobal ? KernelAll : UserAll;
		for (size_t i = 0; i < count; i++)
		{
			const paddr_t paddr = KePhysicalAlloc();
			Assert(paddr)
			const uintptr_t virt = reserved + (i << Arch::PageShift);
			Assert(process.Tables.MapPages(virt, paddr, 1, attributes));
		}

		return (void*)reserved;
	}

	void* VirtualMap(KProcess& process, const paddr_t* addresses, const size_t count)
	{
		CPrintf(process.Debug, "VirtualMap: Any, Count: 0x%x\n", count);
		Assert(count != 0);

		//Reserve region, returning nullptr if it isn't free
		const uintptr_t reserved = process.Space.Reserve(count);
		if (!reserved)
			return nullptr;

		//Map into the process's page tables
		//TODO(tsharpe): Support read/execute only
		const PageAttr attributes = process.IsGlobal ? KernelAll : UserAll;
		for (size_t i = 0; i < count; i++)
		{
			const paddr_t paddr = addresses[i];
			Assert(paddr)
			const uintptr_t virt = reserved + (i << Arch::PageShift);
			Assert(process.Tables.MapPages(virt, paddr, 1, attributes));
		}

		return (void*)reserved;
	}

	bool Free(KProcess& process, const void* address)
	{
		const uintptr_t virt = reinterpret_cast<uintptr_t>(address);
		const size_t count = process.Space.GetCount(virt);
		CPrintf(process.Debug, "Free: 0x%016x Count: 0x%x\n", virt, count);
		Assert(count);

		Assert(process.Tables.UnmapPages(virt, count));
		return process.Space.Free(virt, count);
	}
}

/*
 * Virtual memory.
 *
 * All sizes are in bytes. Each routine has an m_process (kernel) form and a
 * form taking an explicit KProcess. KeVirtualAlloc backs the range with fresh
 * physical pages; KeVirtualMap maps caller-supplied physical addresses.
 */

//Allocate anywhere
void* KeVirtualAlloc(const size_t size)
{
	return KeVirtualAlloc(m_process, size);
}

void* KeVirtualAlloc(KProcess& process, const size_t size)
{
	return _VMEM::Allocate(process, x64::SizeToPages(size));
}

//Allocate at a fixed virtual address
void* KeVirtualAlloc(const void* address, const size_t size)
{
	return KeVirtualAlloc(m_process, address, size);
}

void* KeVirtualAlloc(KProcess& process, const void* address, const size_t size)
{
	return _VMEM::Allocate(process, address, x64::SizeToPages(size));
}

//Free
bool KeVirtualFree(const void* const address)
{
	return _VMEM::Free(m_process, address);
}

bool KeVirtualFree(KProcess& process, const void* const address)
{
	return _VMEM::Free(process, address);
}

//Map a fixed virtual address to a contiguous physical range
bool KeVirtualMap(const void* const virtualAddr, const paddr_t physicalAddr, const size_t size)
{
	return KeVirtualMap(m_process, virtualAddr, physicalAddr, size);
}

bool KeVirtualMap(KProcess& process, const void* const virtualAddr, const paddr_t physicalAddr, const size_t size)
{
	CPrintf(process.Debug, "Map: VA: 0x%016x, PA: 0x%016x, Size: 0x%x, Global: %d\n", virtualAddr, physicalAddr, size, process.IsGlobal);
	Assert(virtualAddr);
	Assert(size);

	const size_t count = x64::SizeToPages(size);

	//Reserve region, returning false if it isn't free
	const uintptr_t reserved = (uintptr_t)virtualAddr;
	if (!process.Space.Reserve(reserved, count))
		return false;

	//Map into the process's page tables
	//TODO(tsharpe): Support read/execute only
	const PageAttr attributes = process.IsGlobal ? KernelAll : UserAll;
	Assert(process.Tables.MapPages(reserved, physicalAddr, count, attributes));

	return true;
}

//Map an array of physical pages, reserving the virtual range automatically
void* KeVirtualMap(const paddr_t* addresses, const size_t size)
{
	return KeVirtualMap(m_process, addresses, size);
}

void* KeVirtualMap(KProcess& process, const paddr_t* addresses, const size_t size)
{
	return _VMEM::VirtualMap(process, addresses, x64::SizeToPages(size));
}

//Map a contiguous physical range, reserving the virtual range automatically
void* KeVirtualMap(const paddr_t address, const size_t size)
{
	return KeVirtualMap(m_process, address, size);
}

void* KeVirtualMap(KProcess& process, const paddr_t address, const size_t size)
{
	CPrintf(process.Debug, "Map: PA: 0x%016x, Size: 0x%x, Global: %d\n", address, size, process.IsGlobal);
	Assert(size);

	const size_t count = x64::SizeToPages(size);

	//Reserve region, returning nullptr if it isn't free
	const uintptr_t reserved = process.Space.Reserve(count);
	if (!reserved)
		return nullptr;

	//Map the contiguous physical range
	const PageAttr attributes = process.IsGlobal ? KernelAll : UserAll;
	Assert(process.Tables.MapPages(reserved, address, count, attributes));

	return (void*)reserved;
}

//Write memory into the address space of another process without switching CR3.
void KeWriteProcessMemory(KProcess& process, void* const destVirtual, const void* const src, const size_t size)
{
	Assert(&process != &m_process);
	
	const uint8_t* source = static_cast<const uint8_t*>(src);
	uintptr_t dest = reinterpret_cast<uintptr_t>(destVirtual);
	size_t remaining = size;

	while (remaining > 0)
	{
		const size_t pageOffset = dest & Arch::PageMask;
		const size_t pageRemaining = Arch::PageSize - pageOffset;
		const size_t chunk = remaining < pageRemaining ? remaining : pageRemaining;

		const paddr_t phys = process.Tables.ResolvePhysical(reinterpret_cast<void*>(dest));
		Assert(phys);

		memcpy(reinterpret_cast<void*>(KernelPhysicalStart + phys), source, chunk);

		dest += chunk;
		source += chunk;
		remaining -= chunk;
	}
}

//Alias a kernel-resident region [source, source+size) into `process` at the fixed address `dest`
void KeAliasInto(KProcess& process, const void* const source, const void* const dest, const size_t size)
{
	const size_t count = x64::SizeToPages(size);
	const uintptr_t destVA = reinterpret_cast<uintptr_t>(dest);

	//Reserve the destination range in the process (a fixed address = the image's ImageBase).
	Assert(process.Space.Reserve(destVA, count));

	//Map the source's (kernel-resident) physical frames into the process, no new physical.
	const PageAttr attributes = process.IsGlobal ? KernelAll : UserAll;
	for (size_t i = 0; i < count; i++)
	{
		const uintptr_t srcPage = reinterpret_cast<uintptr_t>(source) + (i << Arch::PageShift);
		const paddr_t phys = m_process.Tables.ResolvePhysical(reinterpret_cast<void*>(srcPage));
		Assert(phys);
		Assert(process.Tables.MapPages(destVA + (i << Arch::PageShift), phys & ~0xFFFull, 1, attributes));
	}
}
