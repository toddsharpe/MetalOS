#include "kernel/Syscalls/Syscalls.h"
#include "kernel/Scheduler.h"
#include "kernel/Endpoint.h"

namespace
{
	//Copies a NUL-terminated user endpoint name; one that doesn't fit is rejected, not truncated
	bool CopyEndpointName(const UProcess& proc, const char* const name, char (&copy)[MaxEndpointName])
	{
		const size_t length = UserStringLength(proc, name, MaxEndpointName);
		if (length == 0 || length == MaxEndpointName)
			return false;
		if (!proc.Space.IsValidRange(name + length, 1) || name[length] != '\0')
			return false;

		memcpy(copy, name, length);
		copy[length] = '\0';
		return true;
	}
}

SyscallResult RegisterEndpoint(const char* name, const uint64_t id)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!name || !proc.Space.IsValidPointer(name))
		return SyscallResult::InvalidPointer;
	char copy[MaxEndpointName] = {};
	if (!CopyEndpointName(proc, name, copy))
		return SyscallResult::InvalidArg;

	return KeEndpointRegister(copy, id, proc.Id) ? SyscallResult::Success : SyscallResult::Failed;
}

SyscallResult LookupEndpoint(const char* name, uint64_t* id)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!name || !proc.Space.IsValidPointer(name))
		return SyscallResult::InvalidPointer;
	if (!id || !proc.Space.IsValid(id))
		return SyscallResult::InvalidPointer;
	char copy[MaxEndpointName] = {};
	if (!CopyEndpointName(proc, name, copy))
		return SyscallResult::InvalidArg;

	return KeEndpointLookup(copy, *id) ? SyscallResult::Success : SyscallResult::Failed;
}

SyscallResult PostEndpoint(const char* name, const uint64_t value)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!name || !proc.Space.IsValidPointer(name))
		return SyscallResult::InvalidPointer;
	char copy[MaxEndpointName] = {};
	if (!CopyEndpointName(proc, name, copy))
		return SyscallResult::InvalidArg;

	return KeEndpointPost(copy, value) ? SyscallResult::Success : SyscallResult::Failed;
}

SyscallResult PollEndpoint(const char* name, uint64_t* value)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!name || !proc.Space.IsValidPointer(name))
		return SyscallResult::InvalidPointer;
	if (!value || !proc.Space.IsValid(value))
		return SyscallResult::InvalidPointer;
	char copy[MaxEndpointName] = {};
	if (!CopyEndpointName(proc, name, copy))
		return SyscallResult::InvalidArg;

	return KeEndpointPoll(copy, proc.Id, *value) ? SyscallResult::Success : SyscallResult::Failed;
}
