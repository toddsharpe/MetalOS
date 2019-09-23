#define EXPORT
#include "MetalOS.h"

__forceinline SystemCallResult syscall(SystemCall id, ...);

SYSTEMCALL GetProcessInfo(ProcessInfo* info)
{
	if (info == nullptr)
		return SystemCallResult::Failed;

	return syscall(SystemCall::GetProcessInformation, info);
}

__forceinline SystemCallResult syscall(SystemCall id, ...)
{
	//Registers/stack should be how we want it
	//Use syscall intrinsic

	return SystemCallResult::Success;
}
