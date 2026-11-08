#include "kernel/Syscalls/Syscalls.h"
#include "kernel/Scheduler.h"
#include "kernel/System.h"

milli_t GetTickCount()
{
	return KeGetTicks();
}

SyscallResult GetSystemTime(SystemTime* time)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValid(time))
		return SyscallResult::InvalidPointer;

	KSystemTime kTime = {};
	KeGetSystemTime(kTime);

	//Convert to user time
	time->Year = kTime.Year;
	time->Month = kTime.Month;
	time->Day = kTime.Day;
	time->Hour = kTime.Hour;
	time->Minute = kTime.Minute;
	time->Second = kTime.Second;
	time->Milliseconds = kTime.Milliseconds;

	return SyscallResult::Success;
}
