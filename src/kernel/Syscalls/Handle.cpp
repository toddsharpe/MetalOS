#include "kernel/Syscalls/Syscalls.h"
#include "kernel/Scheduler.h"
#include "kernel/Objects/KSharedMemory.h"
#include "kernel/Kernel_grant.h"

SyscallResult CloseHandle(const Handle handle)
{
	UProcess& proc = Scheduler::GetUProcess();

	//Type-specific teardown happens in UProcess::CloseObject (shared with KeTerminateProcess)
	return proc.CloseObject((handle_t)handle) ? SyscallResult::Success : SyscallResult::InvalidHandle;
}

//Mints a single-use token that grants another process this handle; only shared memory is grantable today
SyscallResult ShareHandle(const Handle handle, uint64_t* token)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!token || !proc.Space.IsValid(token))
		return SyscallResult::InvalidPointer;

	UObject* const obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;

	switch (obj->Type)
	{
		case UObjectType::SharedMemory:
			*token = KeGrantShare(UObjectType::SharedMemory, obj->Shm, proc.Id);
			return SyscallResult::Success;

		default:
			return SyscallResult::InvalidObject; //not shareable yet
	}
}

//Redeems a grant token as a fresh handle to the same object, left for the type's own call (e.g. MapSharedMemory) to map
SyscallResult ClaimHandle(const uint64_t token, Handle* handle)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!handle || !proc.Space.IsValid(handle))
		return SyscallResult::InvalidPointer;

	UObjectType type;
	void* object = nullptr;
	if (!KeGrantClaim(token, type, object))
		return SyscallResult::InvalidHandle;

	switch (type)
	{
		case UObjectType::SharedMemory:
		{
			//Adopt the grant's reference; this handle owns it until closed.
			UObject* const obj = proc.CreateObject(UObjectType::SharedMemory);
			obj->Shm = static_cast<KSharedMemory*>(object);
			obj->ShmAddress = nullptr;
			*handle = (Handle)obj->Handle;
			return SyscallResult::Success;
		}

		default:
			return SyscallResult::NotImplemented;
	}
}
