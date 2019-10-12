#define EXPORT
#include "MetalOS.h"

extern "C" SystemCallResult __syscall(SystemCall id, ...);

SYSTEMCALL GetProcessInfo(ProcessInfo* info)
{
	if (info == nullptr)
		return SystemCallResult::Failed;

	return __syscall(SystemCall::GetProcessInformation, info);
}
