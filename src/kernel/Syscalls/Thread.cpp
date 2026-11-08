#include "kernel/Syscalls/Syscalls.h"
#include "kernel/Core/Scheduler.h"
#include "kernel/Thread.h"
#include "kernel/Process.h"

HThread GetCurrentThread()
{
	return (HThread)&Scheduler::GetUThread();
}

HThread CreateThread(size_t stackSize, ThreadStart startAddress, void* arg)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(startAddress))
		return nullptr;

	UThread* created = KeCreateUThread(proc, stackSize, startAddress, arg);
	if (!created)
		return nullptr;

	//Add uobject to calling process
	UObject* obj = proc.CreateObject(UObjectType::Thread);
	obj->Thread = created;
	created->Refs++; //Released when the handle closes

	return (HThread)obj->Handle;
}

uint32_t GetThreadId(HThread thread)
{
	UProcess& proc = Scheduler::GetUProcess();
	UObject* obj = nullptr;
	if (GetObject(proc, thread, UObjectType::Thread, obj) != SyscallResult::Success)
		return 0;

	return obj->Thread->Id;
}

void Sleep(const milli_t time)
{
	if (!time)
		return;

	const nano_t nano = ToNano(time);
	KeSleepThread(nano);
}

void SwitchToThread()
{
	KeYield();
}

SyscallResult TerminateThread(const HThread thread, const uint32_t exitCode)
{
	UProcess& proc = Scheduler::GetUProcess();
	UObject* obj = nullptr;
	const SyscallResult status = GetObject(proc, thread, UObjectType::Thread, obj);
	if (status != SyscallResult::Success)
		return status;

	KeExitUThread(*obj->Thread, exitCode);
	return SyscallResult::Success;
}

void ExitThread(const uint32_t exitCode)
{
	KeExitUThread(Scheduler::GetUThread(), exitCode);
	Unreachable();
}
