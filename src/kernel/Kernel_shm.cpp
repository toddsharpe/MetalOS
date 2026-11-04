#include "kernel/Api.h"
#include "kernel/KProcess.h"
#include "kernel/Arch.h"
#include "x64/x64.h"
#include "kernel/Objects/KSharedMemory.h"
#include "kernel/UObject.h"
#include "Assert.h"

//Kernel globals (defined in Kernel.cpp).
extern KProcess m_process;

KSharedMemory* KeShmCreate(const size_t size)
{
	Assert(size);
	const size_t pageCount = x64::SizeToPages(size);

	//Acquire contiguous physical pages so the region maps as one range (a process picks the size, so this can fail)
	paddr_t physical = 0;
	if (!KePhysicalTryAlloc(physical, pageCount))
		return nullptr;

	//No global registry: regions are reached only through the handles/grants that
	//reference them, and freed by KeShmUnref when the last reference drops.
	KSharedMemory* const shm = KeAlloc<KSharedMemory>(AllocType::Kernel, physical, pageCount);
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
		KeFree(&shm, AllocType::Kernel);
	}
}

void* KePublishRegion(const char* const endpoint, const size_t size, void (*const initialize)(void* region))
{
	KSharedMemory* const shm = KeShmCreate(size);
	Assert(shm);

	//The kernel keeps its mapping for the life of the system, so it holds a reference like a handle would
	shm->Refs.Increment();
	void* const region = KeShmMap(m_process, *shm);
	Assert(region);
	initialize(region);

	//Published only once initialized; the grant holds its own reference until a server claims it
	Assert(KeEndpointRegister(endpoint, KeGrantShare(UObjectType::SharedMemory, shm, KernelGrantOwner), KernelEndpointOwner));
	return region;
}
