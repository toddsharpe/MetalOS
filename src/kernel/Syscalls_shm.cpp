#include "kernel/Syscalls.h"
#include "kernel/Scheduler.h"
#include "kernel/Objects/KSharedMemory.h"
#include "kernel/Kernel_shm.h"

SyscallResult CreateSharedMemory(const size_t size, HSharedMem* handle, void** address)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!handle || !proc.Space.IsValid(handle))
		return SyscallResult::InvalidPointer;
	if (!address || !proc.Space.IsValid(address))
		return SyscallResult::InvalidPointer;
	if (!size || size > UserEnd)
		return SyscallResult::InvalidArg;

	KSharedMemory* const shm = KeShmCreate(size);
	if (!shm)
		return SyscallResult::Failed;
	shm->Refs.Increment(); //this handle's reference

	void* const va = KeShmMap(proc, *shm);
	if (!va)
	{
		KeShmUnref(*shm);
		return SyscallResult::Failed;
	}

	//Wrap the mapping in a handle; closing it (or process exit) releases the region.
	UObject* const obj = proc.CreateObject(UObjectType::SharedMemory);
	obj->Shm = shm;
	obj->ShmAddress = va;

	*handle = (HSharedMem)obj->Handle;
	*address = va;
	return SyscallResult::Success;
}

//Maps a shared-memory handle the caller owns into its address space, returning the existing mapping if there is one
SyscallResult MapSharedMemory(const HSharedMem handle, void** address)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!address || !proc.Space.IsValid(address))
		return SyscallResult::InvalidPointer;

	UObject* obj = nullptr;
	const SyscallResult status = GetObject(proc, handle, UObjectType::SharedMemory, obj);
	if (status != SyscallResult::Success)
		return status;

	if (!obj->ShmAddress)
	{
		void* const va = KeShmMap(proc, *obj->Shm);
		if (!va)
			return SyscallResult::Failed;
		obj->ShmAddress = va;
	}

	*address = obj->ShmAddress;
	return SyscallResult::Success;
}

//Unmaps the caller's view and drops this handle's ref; the pages are freed with the last ref in any process
SyscallResult DeleteSharedMemory(const HSharedMem handle)
{
	UProcess& proc = Scheduler::GetUProcess();

	UObject* obj = nullptr;
	const SyscallResult status = GetObject(proc, handle, UObjectType::SharedMemory, obj);
	if (status != SyscallResult::Success)
		return status;

	return proc.CloseObject((handle_t)handle) ? SyscallResult::Success : SyscallResult::Failed;
}
