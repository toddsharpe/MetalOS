#include "kernel/Syscalls.h"
#include "kernel/Scheduler.h"
#include "kernel/Objects/KPipe.h"
#include "Lib/String.h"
#include "kernel/Kernel_process.h"

namespace
{
	//Resolve a child's standard handle: only the matching end of a pipe can be redirected
	SyscallResult GetStdPipe(UProcess& proc, const HFile handle, const KPipeOp op, KPipe*& pipe)
	{
		UObject* obj = nullptr;
		const SyscallResult status = GetObject(proc, handle, UObjectType::Pipe, obj);
		if (status != SyscallResult::Success)
			return status;
		if (obj->PipeOp != op)
			return SyscallResult::InvalidObject;

		pipe = obj->Pipe;
		return SyscallResult::Success;
	}
}

SyscallResult CreateProcess(const char* commandLine, const CreateProcessArgs* args, CreateProcessResult* result)
{
	UProcess& proc = Scheduler::GetUProcess();
	if ((!proc.Space.IsValidPointer(commandLine)) || (args && !proc.Space.IsValid(args)) || (result && !proc.Space.IsValid(result)))
		return SyscallResult::InvalidPointer;

	//Caller's pipe end for each of the child's standard handles
	struct StdRedirect
	{
		HFile Source;
		KPipeOp Op;
		handle_t Target;
		KPipe* Pipe;
	};
	StdRedirect redirects[] =
	{
		{ args ? args->StdInput : nullptr, KPipeOp::Read, Handles::StdIn, nullptr },
		{ args ? args->StdOutput : nullptr, KPipeOp::Write, Handles::StdOut, nullptr },
		{ args ? args->StdError : nullptr, KPipeOp::Write, Handles::StdErr, nullptr }
	};

	//Resolve standard handles before creating the process, so a bad one can't leave a half-built child
	for (StdRedirect& redirect : redirects)
	{
		if (!redirect.Source)
			continue;

		const SyscallResult status = GetStdPipe(proc, redirect.Source, redirect.Op, redirect.Pipe);
		if (status != SyscallResult::Success)
			return status;
	}

	//Copy string locally to automatically crop it at max bytes, reading only bytes that are mapped
	char copy[128] = {};
	StrCopy(copy, CStr(commandLine, UserStringLength(proc, commandLine, sizeof(copy) - 1)));

	UProcess* created = KeCreateProcess(copy);
	if (!created)
		return SyscallResult::Failed;

	if (result && proc.Space.IsValid(result))
	{
		//Add uobject to calling process
		UObject* obj = proc.CreateObject(UObjectType::Process);
		obj->Process = created;
		created->Refs++; //Released when the handle closes

		result->Process = (HProcess)obj->Handle;
	}

	//If console application, redirect standard handles (without args it keeps the debug output)
	if (created->IsConsole)
	{
		for (const StdRedirect& redirect : redirects)
		{
			if (!redirect.Pipe)
				continue;

			//Replace the default debug output (there is no default stdin, so that close is a no-op)
			created->CloseObject(redirect.Target);

			KRef& ends = (redirect.Op == KPipeOp::Read) ? redirect.Pipe->Readers : redirect.Pipe->Writers;
			ends.Increment();

			UObject* const obj = created->CreateObject(UObjectType::Pipe, redirect.Target);
			obj->Pipe = redirect.Pipe;
			obj->PipeOp = redirect.Op;
		}
	}

	return SyscallResult::Success;
}

SyscallResult TerminateProcess(const HProcess hProc, const uint32_t exitCode)
{
	UProcess& proc = Scheduler::GetUProcess();
	UObject* obj = nullptr;
	const SyscallResult status = GetObject(proc, hProc, UObjectType::Process, obj);
	if (status != SyscallResult::Success)
		return status;

	KeTerminateProcess(*obj->Process, exitCode);
	return SyscallResult::Success;
}

void ExitProcess(const uint32_t exitCode)
{
	UProcess& proc = Scheduler::GetUProcess();
	KeTerminateProcess(proc, exitCode);

	Unreachable();
}

bool IsProcessAlive(uint32_t id)
{
	return KeIsProcessAlive(id);
}

SyscallResult GetProcesses(ProcessListEntry* buffer, const size_t maxCount, size_t* count)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (maxCount > static_cast<size_t>(-1) / sizeof(ProcessListEntry))
		return SyscallResult::InvalidArg; //Byte size would overflow
	if (!proc.Space.IsValidRange(buffer, maxCount * sizeof(ProcessListEntry)) || !proc.Space.IsValid(count))
		return SyscallResult::InvalidPointer;

	*count = KeGetProcessList(buffer, maxCount);
	return SyscallResult::Success;
}
