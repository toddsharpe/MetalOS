#include "kernel/Api.h"
#include "kernel/KProcess.h"
#include "kernel/Arch.h"
#include "x64/x64.h"
#include "kernel/Objects/KSharedMemory.h"
#include "Assert.h"

KSharedMemory* KeShmCreate(const size_t size)
{
	Assert(size);
	const size_t pageCount = x64::SizeToPages(size);

	//Acquire contiguous physical pages so the region maps as one range
	const paddr_t physical = KePhysicalAlloc(pageCount);
	Assert(physical);

	//No global registry: regions are reached only through the handles/grants that
	//reference them, and freed by KeShmUnref when the last reference drops.
	KSharedMemory* const shm = KeAlloc<KSharedMemory>(AllocType::Shared, physical, pageCount);
	Assert(shm);
	return shm;
}

void* KeShmMap(KProcess& process, KSharedMemory& shm)
{
	//Reserve virtual space in the process and map the contiguous physical range.
	//Reference counting is per-handle (the SharedMemory UObject), not per-mapping,
	//so mapping does not touch Refs.
	return KeVirtualMap(process, shm.Physical, shm.PageCount << Arch::PageShift);
}

void KeShmUnref(KSharedMemory& shm)
{
	shm.Refs.Decrement();

	//Last handle gone: reclaim the physical pages and drop the object.
	if (shm.Refs.IsEmpty())
	{
		KePhysicalFree(shm.Physical, shm.PageCount);
		KeFree(&shm, AllocType::Shared);
	}
}
