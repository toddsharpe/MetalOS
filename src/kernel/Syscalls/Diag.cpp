#include "kernel/Syscalls/Syscalls.h"
#include "kernel/Core/Scheduler.h"
#include "kernel/Arch.h"
#include "kernel/Diag.h"

SyscallResult DebugPrint(const char* s)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(s))
		return SyscallResult::InvalidPointer;

	//User text is data, never a format string
	Printf("%.*s", (int)UserStringLength(proc, s, MaxDebugPrint), s);
	return SyscallResult::Success;
}

SyscallResult DebugPrintBytes(const void* s, const size_t length)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!length)
		return SyscallResult::InvalidArg;

	//The whole range is read
	if (!proc.Space.IsValidRange(s, length))
		return SyscallResult::InvalidPointer;

	PrintBytes(s, length);
	return SyscallResult::Success;
}

//TODO(tsharpe): Untested
SyscallResult DebugPrintStack(const uint64_t rip)
{
	UThread& thread = Scheduler::GetUThread();
	thread.Process.Display();

	//Stack is the raw user RSP from syscall entry
	if (!thread.Process.Space.IsValidRange(thread.Stack, sizeof(uintptr_t)))
		return SyscallResult::InvalidPointer;

	const Arch::Context context = ArchMakeContext(rip, (uintptr_t)thread.Stack, *(uintptr_t*)thread.Stack);
	PrintStack(&context, thread.Process);

	return SyscallResult::Success;
}
