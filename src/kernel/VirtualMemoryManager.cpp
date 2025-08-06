#include "kernel/VirtualMemoryManager.h"
#include "kernel/Api.h"
#include "kernel/Arch.h"
#include "Assert.h"

//Reserves space in calling process, acquires physical pages, maps into page tables
namespace VMM
{
	void* Allocate(KProcess& process, const size_t count)
	{
		CPrintf(process.Debug, "Allocate: Any, Count: 0x%x Global: %d\n", count, process.IsGlobal);
		Assert(count);

		//Reserve region, returning nullptr if it isn't free
		const uintptr_t reserved = process.Reserve(count);
		if (!reserved)
			return nullptr;

		//Map into current page tables
		PageTables current;
		current.OpenCurrent();
		for (size_t i = 0; i < count; i++)
		{
			paddr_t paddr = KePhysicalAlloc();
			Assert(paddr)
			Assert(current.MapPages(reserved + (i << PageShift), paddr, 1, process.IsGlobal));
		}
		
		//Zero region
		void* const baseAddress = (void*)reserved;
		memset(baseAddress, 0, count * PageSize);
		return baseAddress;
	}
	
	void* Allocate(KProcess& process, const void* address, const size_t count)
	{
		CPrintf(process.Debug, "Allocate: 0x%016x, Count: 0x%x Global: %d\n", address, count, process.IsGlobal);
		Assert(address);
		Assert(count);
		
		//Reserve region, returning nullptr if it isn't free
		const uintptr_t reserved = (uintptr_t)address;
		if (!process.Reserve(reserved, count))
			return nullptr;

		//Map into current page tables
		PageTables current;
		current.OpenCurrent();
		for (size_t i = 0; i < count; i++)
		{
			paddr_t paddr = KePhysicalAlloc();
			Assert(paddr)
			Assert(current.MapPages(reserved + (i << PageShift), paddr, 1, process.IsGlobal));
		}
		
		//Zero region
		void* const baseAddress = (void*)reserved;
		memset(baseAddress, 0, count * PageSize);
		return baseAddress;
	}

	bool MapContiguous(KProcess& process, const void* const virtualAddr, const paddr_t physicaAddr, const size_t count)
	{
		CPrintf(process.Debug, "MapContiguous: VA: 0x%016x, PA: 0x%016x, Count: 0x%x, Global: %d\n", virtualAddr, physicaAddr, count, process.IsGlobal);
		Assert(virtualAddr);
		Assert(physicaAddr);
		Assert(count);

		//Reserve region, returning false if it isn't free
		const uintptr_t reserved = (uintptr_t)virtualAddr;
		if (!process.Reserve(reserved, count))
			return false;

		//Map into current page tables
		PageTables current;
		current.OpenCurrent();
		Assert(current.MapPages(reserved, physicaAddr, count, process.IsGlobal));

		return true;
	}

	void* VirtualMap(KProcess& process, const paddr_t* addresses, const size_t count)
	{
		CPrintf(process.Debug, "VirtualMap: Any, Count: 0x%x\n", count);
		Assert(count != 0);

		//Reserve region, returning nullptr if it isn't free
		const uintptr_t reserved = process.Reserve(count);
		if (!reserved)
			return nullptr;

		//Map into current page tables
		PageTables current;
		current.OpenCurrent();
		for (size_t i = 0; i < count; i++)
		{
			const paddr_t paddr = addresses[i];
			Assert(paddr)
			Assert(current.MapPages(reserved + (i << PageShift), paddr, 1, process.IsGlobal));
		}
		
		//Zero region
		void* const baseAddress = (void*)reserved;
		memset(baseAddress, 0, count * PageSize);
		return baseAddress;
	}
}
