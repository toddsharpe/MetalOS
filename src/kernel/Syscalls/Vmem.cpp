#include "kernel/Syscalls/Syscalls.h"
#include "kernel/Scheduler.h"
#include "kernel/Arch.h"
#include "kernel/Kernel_vmem.h"

void* VirtualAlloc(const void* address, const size_t size)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!size || size > UserEnd) //Larger can never fit, and would overflow the page count
		return nullptr;

	if (!address)
		return KeVirtualAlloc(proc, size);

	//Fixed addresses must be page aligned (the allocator asserts on it)
	if (((uintptr_t)address & Arch::PageMask) != 0)
		return nullptr;

	return KeVirtualAlloc(proc, address, size);
}

bool VirtualFree(const void* address)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!address)
		return false;

	//Must be the start of a reservation (the allocator asserts otherwise)
	if (proc.Space.GetCount((uintptr_t)address) == 0)
		return false;

	//Only VirtualAlloc'd memory: shared memory and the framebuffer are released through their handles
	if (!proc.Space.OwnsPages((uintptr_t)address))
		return false;

	return KeVirtualFree(proc, address);
}
