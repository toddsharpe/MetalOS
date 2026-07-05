#pragma once

#include "MetalOS.Syscalls.h"
#include "kernel/Scheduler.h"
#include "kernel/Arch.h"
#include "user/MetalOS.h"
#include "Lib/String.h"
#include "kernel/Api.h"
#include "kernel/Objects/KPredicate.h"
#include "kernel/Objects/KPipe.h"
#include "kernel/Objects/UPipe.h"
#include "kernel/Objects/KSharedMemory.h"
#include "kernel/Net/KSocket.h"
#include "Lib/ByteSwap.h"
#include "Assert.h"

//TODO(tsharpe): Syscalls should be going through Kernel Api, but theres no kernel internal use for windows?

namespace
{
	constexpr KFileAccess ToAccess(const FileAccess access)
	{
		switch (access)
		{
			case FileAccess::Read:
				return KFileAccess::Read;

			case FileAccess::Write:
				return KFileAccess::Write;

			case FileAccess::ReadWrite:
				return KFileAccess::ReadWrite;

			default:
				return KFileAccess::None;
		}
	}

	constexpr WaitStatus FromWaitResult(const KWaitResult result)
	{
		switch (result)
		{
			case KWaitResult::Signaled:
				return WaitStatus::Signaled;

			case KWaitResult::Timeout:
				return WaitStatus::Timeout;

			default:
				return WaitStatus::None;
		}
	}
}

//Syscalls that the kernel adds params
SyscallResult DebugPrintStack(const uint64_t rip);

//Packed form of SocketRecvFrom (the userland wrapper packs the >4 args, see Sockets.cpp)
SyscallResult SocketRecvFromSys(HSocket sock, SocketRecvParams* params);

uint64_t Dispatch(const Arch::SyscallFrame& frame)
{
	Syscall call = static_cast<Syscall>(frame.SystemCall);
	switch (call)
	{
		//0x100
		case Syscall::GetTickCount:
			return (uint64_t)GetTickCount();

		case Syscall::GetSystemTime:
			return (uint64_t)GetSystemTime((SystemTime*)frame.Arg0);

		//0x200
		case Syscall::GetCurrentThread:
			return (uint64_t)GetCurrentThread();

		case Syscall::CreateProcess:
			return (uint64_t)CreateProcess((char*)frame.Arg0, (CreateProcessArgs*)frame.Arg1, (CreateProcessResult*)frame.Arg2);

		case Syscall::CreateThread:
			return (uint64_t)CreateThread((size_t)frame.Arg0, (ThreadStart)frame.Arg1, (void*)frame.Arg2);

		case Syscall::GetThreadId:
			return (uint64_t)GetThreadId((HThread)frame.Arg0);

		case Syscall::Sleep:
			Sleep((milli_t)frame.Arg0);
			return 0;

		case Syscall::SwitchToThread:
			SwitchToThread();
			return 0;

		case Syscall::SuspendThread:
			return (uint64_t)SuspendThread((HThread)frame.Arg0);

		case Syscall::ResumeThread:
			return (uint64_t)ResumeThread((HThread)frame.Arg0);

		case Syscall::TerminateProcess:
			return (uint64_t)TerminateProcess((HProcess)frame.Arg0, (uint32_t)frame.Arg1);

		case Syscall::ExitProcess:
			ExitProcess((uint32_t)frame.Arg0);
			return 0;

		case Syscall::TerminateThread:
			return (uint64_t)TerminateThread((HThread)frame.Arg0, (uint32_t)frame.Arg1);

		case Syscall::ExitThread:
			ExitThread((uint32_t)frame.Arg0);
			return 0;

		case Syscall::IsProcessAlive:
			return (uint64_t)IsProcessAlive((uint32_t)frame.Arg0);

		//0x300 windowing moved to the usermode WM (User.dll <-> wm.exe)

		//0x400
		case Syscall::CreateFile:
			return (uint64_t)CreateFile((char*)frame.Arg0, (FileAccess)frame.Arg1);

		case Syscall::ReadFile:
			return (uintptr_t)ReadFile((HFile)frame.Arg0, (void*)frame.Arg1, (size_t)frame.Arg2, (size_t*)frame.Arg3);

		case Syscall::WriteFile:
			return (uintptr_t)WriteFile((HFile)frame.Arg0, (void*)frame.Arg1, (size_t)frame.Arg2, (size_t*)frame.Arg3);

		case Syscall::SetFilePointer:
			return (uintptr_t)SetFilePointer((HFile)frame.Arg0, (ssize_t)frame.Arg1, (Seek)frame.Arg2, (size_t*)frame.Arg3);

		case Syscall::MoveFile:
			return (uint64_t)MoveFile((char*)frame.Arg0, (char*)frame.Arg1);

		case Syscall::DeleteFile:
			return (uint64_t)DeleteFile((char*)frame.Arg0);

		case Syscall::CreateDirectory:
			return (uint64_t)CreateDirectory((char*)frame.Arg0);

		case Syscall::CreatePipe:
			return (uint64_t)CreatePipe((HFile*)frame.Arg0, (HFile*)frame.Arg1);

		case Syscall::PeekNamedPipe:
			return (uint64_t)PeekNamedPipe((HFile)frame.Arg0, (size_t*)frame.Arg1);

		case Syscall::CloseHandle:
			return (uint64_t)CloseHandle((Handle)frame.Arg0);

		//0x500
		case Syscall::WaitForSingleObject:
			return (uint64_t)WaitForSingleObject((Handle)frame.Arg0, (milli_t)frame.Arg1, (WaitStatus*)frame.Arg2);

		case Syscall::CreateEvent:
			return (uint64_t)CreateEvent((HEvent*)frame.Arg0, (bool)frame.Arg1, (bool)frame.Arg2);

		case Syscall::SetEvent:
			return (uint64_t)SetEvent((HEvent)frame.Arg0);

		case Syscall::ResetEvent:
			return (uint64_t)ResetEvent((HEvent)frame.Arg0);

		//0x600
		case Syscall::VirtualAlloc:
			return (uintptr_t)VirtualAlloc((void*)frame.Arg0, (size_t)frame.Arg1);

		case Syscall::CreateSharedMemory:
			return (uint64_t)CreateSharedMemory((size_t)frame.Arg0, (HSharedMem*)frame.Arg1, (void**)frame.Arg2);

		case Syscall::MapSharedMemory:
			return (uint64_t)MapSharedMemory((HSharedMem)frame.Arg0, (void**)frame.Arg1);

		case Syscall::MapFramebuffer:
			return (uint64_t)MapFramebuffer((GraphicsDevice*)frame.Arg0);

		//0x700
		case Syscall::DebugPrint:
			return (uint64_t)DebugPrint((char*)frame.Arg0);

		case Syscall::DebugPrintBytes:
			return (uint64_t)DebugPrintBytes((char*)frame.Arg0, (size_t)frame.Arg1);

		case Syscall::DebugPrintStack:
			return (uint64_t)DebugPrintStack(frame.UserIP);

		//0x800
		case Syscall::SocketCreate:
			return (uint64_t)SocketCreate((int)frame.Arg0, (int)frame.Arg1, (int)frame.Arg2);

		case Syscall::SocketBind:
			return (uint64_t)SocketBind((HSocket)frame.Arg0, (const sockaddr_in*)frame.Arg1);

		case Syscall::SocketConnect:
			return (uint64_t)SocketConnect((HSocket)frame.Arg0, (const sockaddr_in*)frame.Arg1);

		case Syscall::SocketSendTo:
			return (uint64_t)SocketSendTo((HSocket)frame.Arg0, (const void*)frame.Arg1, (size_t)frame.Arg2, (const sockaddr_in*)frame.Arg3);

		case Syscall::SocketRecvFrom:
			return (uint64_t)SocketRecvFromSys((HSocket)frame.Arg0, (SocketRecvParams*)frame.Arg1);

		case Syscall::SocketSend:
			return (uint64_t)SocketSend((HSocket)frame.Arg0, (const void*)frame.Arg1, (size_t)frame.Arg2);

		case Syscall::SocketClose:
			return (uint64_t)SocketClose((HSocket)frame.Arg0);

		case Syscall::GetInterfaces:
			return (uint64_t)GetInterfaces((InterfaceInfo*)frame.Arg0, (size_t)frame.Arg1, (size_t*)frame.Arg2);

		case Syscall::GetInterfaceIp:
			return (uint64_t)GetInterfaceIp((uint32_t)frame.Arg0, (in_addr*)frame.Arg1, (in_addr*)frame.Arg2);

		case Syscall::SetInterfaceIp:
			return (uint64_t)SetInterfaceIp((uint32_t)frame.Arg0, (const in_addr*)frame.Arg1, (const in_addr*)frame.Arg2);

		case Syscall::SetGateway:
			return (uint64_t)SetGateway((uint32_t)frame.Arg0, (const in_addr*)frame.Arg1);

		//0x900
		case Syscall::RegisterEndpoint:
			return (uint64_t)RegisterEndpoint((const char*)frame.Arg0, (uint64_t)frame.Arg1);

		case Syscall::LookupEndpoint:
			return (uint64_t)LookupEndpoint((const char*)frame.Arg0, (uint64_t*)frame.Arg1);

		case Syscall::PostEndpoint:
			return (uint64_t)PostEndpoint((const char*)frame.Arg0, (uint64_t)frame.Arg1);

		case Syscall::PollEndpoint:
			return (uint64_t)PollEndpoint((const char*)frame.Arg0, (uint64_t*)frame.Arg1);

		//0xA00
		case Syscall::ShareHandle:
			return (uint64_t)ShareHandle((Handle)frame.Arg0, (uint64_t*)frame.Arg1);

		case Syscall::ClaimHandle:
			return (uint64_t)ClaimHandle((uint64_t)frame.Arg0, (Handle*)frame.Arg1);

		default:
			//Syscall not found
			Printf("Syscall not implemented: 0x%x\n", frame.SystemCall);
			Printf("  IP: 0x%016x\n", frame.UserIP);
			return (uint64_t)SyscallResult::NotImplemented;
	}
}

extern "C" uint64_t KeSyscall(const Arch::SyscallFrame& frame)
{
	//Printf("Syscall: 0x%x\n", frame.SystemCall);
	const uint64_t ret = Dispatch(frame);
	//Printf("    Result: 0x%016x\n", ret);
	return ret;
}

/*
 * 0x100.
 */
milli_t GetTickCount()
{
	return KeGetTicks();
}

SyscallResult GetSystemTime(SystemTime* time)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(time))
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

/*
 * 0x200.
 */
HThread GetCurrentThread()
{
	return (HThread)&Scheduler::GetUThread();
}

SyscallResult CreateProcess(const char* commandLine, const CreateProcessArgs* args, CreateProcessResult* result)
{
	UProcess& proc = Scheduler::GetUProcess();
	if ((!proc.Space.IsValidPointer(commandLine)) || (args && !proc.Space.IsValidPointer(args)) || (result && !proc.Space.IsValidPointer(result)))
		return SyscallResult::InvalidPointer;

	//Copy string locally to automatically crop it at max bytes
	StaticString<128> copy = {};
	copy.AppendClip(commandLine);

	UProcess* created = KeCreateProcess(copy.c_str());
	if (!created)
		return SyscallResult::Failed;

	if (result && proc.Space.IsValidPointer(result))
	{
		//Add uobject to calling process
		UObject* obj = proc.CreateObject(UObjectType::Process);
		obj->Process = created;

		result->Process = (HProcess)obj->Handle;
	}

	//If console application, set standard handles
	if (created->IsConsole)
	{
		Assert(args);

		if (args->StdInput)
		{
			UObject* obj = proc.GetObject((handle_t)args->StdInput);
			if (!obj)
				return SyscallResult::InvalidHandle;
			if (!obj->CanRead())
				return SyscallResult::InvalidObject;

			KPipe& backing = obj->Pipe->KPipe;
			backing.Readers.Increment();

			//Set stdin to new process
			UObject* stdin = KeAlloc<UObject>(AllocType::User, UObjectType::Pipe, Handles::StdIn);
			stdin->Pipe = KeAlloc<UPipe>(AllocType::User, backing, KPipeOp::Read);
			Assert(created->Objects.Add(stdin));
		}

		if (args->StdOutput)
		{
			UObject* obj = proc.GetObject((handle_t)args->StdOutput);
			if (!obj)
				return SyscallResult::InvalidHandle;
			if (!obj->CanWrite())
				return SyscallResult::InvalidObject;

			//Delete existing stdout
			created->CloseObject((handle_t)Handles::StdOut);

			KPipe& backing = obj->Pipe->KPipe;
			backing.Writers.Increment();

			//Set stdout to new process
			UObject* stdout = KeAlloc<UObject>(AllocType::User, UObjectType::Pipe, Handles::StdOut);
			stdout->Pipe = KeAlloc<UPipe>(AllocType::User, backing, KPipeOp::Write);
			Assert(created->Objects.Add(stdout));
		}

		if (args->StdError)
		{
			UObject* obj = proc.GetObject((handle_t)args->StdError);
			if (!obj)
				return SyscallResult::InvalidHandle;
			if (!obj->CanWrite())
				return SyscallResult::InvalidObject;

			//Delete existing stdout
			created->CloseObject((handle_t)Handles::StdErr);

			KPipe& backing = obj->Pipe->KPipe;
			backing.Writers.Increment();

			//Set stderr to new process
			UObject* stderr = KeAlloc<UObject>(AllocType::User, UObjectType::Pipe, Handles::StdErr);
			stderr->Pipe = KeAlloc<UPipe>(AllocType::User, backing, KPipeOp::Write);
			Assert(created->Objects.Add(stderr));
		}
	}

	return SyscallResult::Success;
}

HThread CreateThread(size_t stackSize, ThreadStart startAddress, void* arg)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(startAddress))
		return nullptr;

	UThread* created = KeCreateUThread(proc, stackSize, startAddress, arg);

	//Add uobject to calling process
	UObject* obj = proc.CreateObject(UObjectType::Thread);
	obj->Thread = created;

	return (HThread)obj->Handle;
}

bool IsProcessAlive(uint32_t id)
{
	return KeIsProcessAlive(id);
}

uint32_t GetThreadId(HThread thread)
{
	UProcess& proc = Scheduler::GetUProcess();
	UObject* obj = proc.GetObject((handle_t)thread);
	if (!obj)
		return 0;

	if (obj->Type != UObjectType::Thread)
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

SyscallResult SuspendThread(const HThread file)
{
	return SyscallResult::NotImplemented;
}

SyscallResult ResumeThread(const HThread file)
{
	return SyscallResult::NotImplemented;
}

SyscallResult TerminateProcess(const HProcess hProc, const uint32_t exitCode)
{
	UProcess& proc = Scheduler::GetUProcess();
	UObject* obj = proc.GetObject((handle_t)hProc);
	if (!obj)
		return SyscallResult::InvalidHandle;

	if (obj->Type != UObjectType::Process)
		return SyscallResult::InvalidObject;

	KeTerminateProcess(*obj->Process, exitCode);
	return SyscallResult::Success;
}

void ExitProcess(const uint32_t exitCode)
{
	UProcess& proc = Scheduler::GetUProcess();

	//Free all windows
	KeTerminateProcess(proc, exitCode);

	Unreachable();
}

SyscallResult TerminateThread(const HThread thread, const uint32_t exitCode)
{
	UNUSED(exitCode);
	
	UProcess& proc = Scheduler::GetUProcess();
	UObject* obj = proc.GetObject((handle_t)thread);
	if (!obj)
		return SyscallResult::InvalidHandle;

	if (obj->Type != UObjectType::Thread)
		return SyscallResult::InvalidObject;

	KeExitThread(obj->Thread->Thread);
	return SyscallResult::Success;
}

void ExitThread(const uint32_t exitCode)
{
	UNUSED(exitCode);
	
	KeExitThread();
}

/*
* 0x400.
*/
HFile CreateFile(const char* path, const FileAccess access)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(path))
		return nullptr;

	UObject* uFile = proc.CreateObject(UObjectType::File);
	if (!uFile)
		return nullptr;

	StaticString<128> copy;
	copy.AppendClip(path);
	copy.ToLower();

	const bool result = KeCreateFile(uFile->File, copy.c_str(), ToAccess(access));
	if (!result)
		return nullptr;
	return (HFile)uFile->Handle;
}

SyscallResult ReadFile(const HFile handle, void* buffer, const size_t bufferSize, size_t* bytesRead)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(buffer) || (bytesRead && !proc.Space.IsValidPointer(bytesRead)))
		return SyscallResult::InvalidPointer;

	if (!bufferSize)
		return SyscallResult::InvalidArg;

	UObject* obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;

	if (!obj->CanRead())
		return SyscallResult::InvalidObject;

	switch (obj->Type)
	{
		case UObjectType::File:
		{
			const bool result = KeReadFile(obj->File, buffer, bufferSize, bytesRead);
			return result ? SyscallResult::Success : SyscallResult::Failed;
		}
		break;

		case UObjectType::Pipe:
		{
			if (obj->Pipe->Op != KPipeOp::Read)
				return SyscallResult::InvalidObject;
			
			//Create read wait
			KPipeSignal signal(obj->Pipe->KPipe, KPipeOp::Read, bufferSize);
			KPredicate wait(&KPipe::EventSignal, &signal);

			//Wait for pipe signal
			KWaitResult result = KeWait(wait);
			Assert(result == KWaitResult::Signaled);

			//Read
			size_t bytes = 0;
			Assert(obj->Pipe->KPipe.Read(buffer, bufferSize, bytes));
			if (bytesRead)
				*bytesRead = bytes;

			return obj->Pipe->KPipe.IsBroken() ? SyscallResult::BrokenPipe : SyscallResult::Success;
		}
		break;

		default:
			Trace();
			return SyscallResult::Failed;
	}
}

SyscallResult WriteFile(const HFile handle, const void* buffer, const size_t bufferSize, size_t* bytesWritten)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(buffer))
		return SyscallResult::InvalidPointer;
	if (!bufferSize)
		return SyscallResult::InvalidArg;
	if (bytesWritten && !proc.Space.IsValidPointer(bytesWritten))
		return SyscallResult::InvalidPointer;

	UObject* obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;
	
	if (!obj->CanWrite())
		return SyscallResult::InvalidObject;

	switch (obj->Type)
	{
		case UObjectType::Pipe:
		{
			if (obj->Pipe->Op != KPipeOp::Write)
				return SyscallResult::InvalidObject;

			//Create read wait
			KPipeSignal signal(obj->Pipe->KPipe, KPipeOp::Write, bufferSize);
			KPredicate wait(&KPipe::EventSignal, &signal);

			//Wait for pipe signal
			KWaitResult result = KeWait(wait);
			Assert(result == KWaitResult::Signaled);

			//Write
			size_t bytes = 0;
			Assert(obj->Pipe->KPipe.Write(buffer, bufferSize, bytes));
			return SyscallResult::Success;
		}
		break;

		case UObjectType::Debug:
		{
			const char* const s = (char*)buffer;
			Printf(s);
			return SyscallResult::Success;
		}
		break;

		default:
			Assert(false);
			return SyscallResult::Failed;
	}
}

SyscallResult SetFilePointer(const HFile handle, const ssize_t position, const Seek seek, size_t* newPosition)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (newPosition && !proc.Space.IsValidPointer(newPosition))
		return SyscallResult::InvalidPointer;

	UObject* uFile = proc.GetObject((handle_t)handle);
	if (!uFile)
		return SyscallResult::InvalidHandle;

	size_t newPos = 0;
	switch (seek)
	{
		case Seek::Begin:
			newPos = position;
			break;

		case Seek::Current:
			newPos = uFile->File.Position + position;
			break;

		case Seek::End:
			newPos = uFile->File.Position + position;
			break;

		default:
			return SyscallResult::InvalidArg;
	}

	const bool result = KeSetFilePosition(uFile->File, newPos);
	if (!result)
		return SyscallResult::Failed;

	if (newPosition)
		*newPosition = newPos;

	return SyscallResult::Success;
}

SyscallResult MoveFile(const char* existingFileName, const char* newFileName)
{
	Assert(false);
	return SyscallResult::Failed;
}

SyscallResult DeleteFile(const char* fileName)
{
	Assert(false);
	return SyscallResult::Failed;
}

SyscallResult CreateDirectory(const char* path)
{
	Assert(false);
	return SyscallResult::Failed;
}

SyscallResult CreatePipe(HFile* readHandle, HFile* writeHandle)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!readHandle || !proc.Space.IsValidPointer(readHandle))
		return SyscallResult::InvalidPointer;
	if (!writeHandle || !proc.Space.IsValidPointer(writeHandle))
		return SyscallResult::InvalidPointer;

	const size_t pipeSize = Arch::PageSize;
	
	//Create kernel pipe
	KPipe* pipe = KeAlloc<KPipe>(AllocType::Shared, pipeSize);
	pipe->Init();
	pipe->Readers.Increment();
	pipe->Writers.Increment();

	//Create reader
	{
		UObject* reader = KeAlloc<UObject>(AllocType::User, UObjectType::Pipe);
		reader->Pipe = KeAlloc<UPipe>(AllocType::User, *pipe, KPipeOp::Read);
		proc.Objects.Add(reader);
		*readHandle = (HFile)reader->Handle;
	}

	//Create writer
	{
		UObject* writer = KeAlloc<UObject>(AllocType::User, UObjectType::Pipe);
		writer->Pipe = KeAlloc<UPipe>(AllocType::User, *pipe, KPipeOp::Write);
		proc.Objects.Add(writer);
		*writeHandle = (HFile)writer->Handle;
	}

	return SyscallResult::Success;
}

SyscallResult PeekNamedPipe(const HFile file, size_t* bytesAvailable)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!bytesAvailable || !proc.Space.IsValidPointer(bytesAvailable))
		return SyscallResult::InvalidPointer;

	UObject* obj = proc.GetObject((handle_t)file);
	if (!obj)
		return SyscallResult::InvalidHandle;

	if (obj->Type != UObjectType::Pipe)
		return SyscallResult::InvalidObject;

	*bytesAvailable = obj->Pipe->KPipe.Count();
	return obj->Pipe->KPipe.IsBroken() ? SyscallResult::BrokenPipe : SyscallResult::Success;
}

SyscallResult CloseHandle(const Handle handle)
{
	UProcess& proc = Scheduler::GetUProcess();

	UObject* obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;

	switch (obj->Type)
	{
		case UObjectType::Pipe:
		{
			switch (obj->Pipe->Op)
			{
				case KPipeOp::Read:
					obj->Pipe->KPipe.Readers.Decrement();
					break;

				case KPipeOp::Write:
					obj->Pipe->KPipe.Writers.Decrement();
					break;

				default:
					Assert(false);
			}
		}
		break;

		case UObjectType::File:
		case UObjectType::Event:
		case UObjectType::Process:
		case UObjectType::Thread:
		case UObjectType::Debug:
		case UObjectType::Socket:
		case UObjectType::SharedMemory:
			//do nothing here; type-specific teardown happens in UProcess::CloseObject
			break;

		default:
			Printf("Closing: %d\n", obj->Type);
			NotImplemented();
			break;
	}

	const bool result = proc.CloseObject((handle_t)handle);
	return result ? SyscallResult::Success : SyscallResult::Failed;
}

/*
 * 0x500.
 */
SyscallResult WaitForSingleObject(const Handle handle, const milli_t time, WaitStatus* status)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!status || !proc.Space.IsValidPointer(status))
		return SyscallResult::InvalidPointer;

	UObject* obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;

	switch (obj->Type)
	{
		case UObjectType::Event:
			*status = FromWaitResult(KeWait(*obj->Event, time));
			break;

		default:
			return SyscallResult::InvalidObject;
	}

	return SyscallResult::Success;
}

SyscallResult CreateEvent(HEvent* event, const bool manual, const bool initial)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!event || !proc.Space.IsValidPointer(event))
		return SyscallResult::InvalidPointer;

	//Create Event
	UObject* created = KeAlloc<UObject>(AllocType::User, UObjectType::Event);
	created->Event = KeAlloc<KEvent>(AllocType::User, manual, initial);
	proc.Objects.Add(created);

	*event = (HEvent)created->Handle;
	return SyscallResult::Success;
}

SyscallResult SetEvent(const HEvent event)
{
	UProcess& proc = Scheduler::GetUProcess();
	UObject* obj = proc.GetObject((handle_t)event);
	if (!obj)
		return SyscallResult::InvalidHandle;
	if (obj->Type != UObjectType::Event)
		return SyscallResult::InvalidObject;

	Assert(obj->Event);
	obj->Event->Set();
	return SyscallResult::Success;
}

SyscallResult ResetEvent(const HEvent event)
{
	UProcess& proc = Scheduler::GetUProcess();
	UObject* obj = proc.GetObject((handle_t)event);
	if (!obj)
		return SyscallResult::InvalidHandle;
	if (obj->Type != UObjectType::Event)
		return SyscallResult::InvalidObject;

	Assert(obj->Event);
	obj->Event->Reset();
	return SyscallResult::Success;
}

/*
 * 0x600.
 */
void* VirtualAlloc(const void* address, const size_t size)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!size)
		return nullptr;

	if (!address)
		return KeVirtualAlloc(proc, size);

	return KeVirtualAlloc(proc, address, size);
}

SyscallResult CreateSharedMemory(const size_t size, HSharedMem* handle, void** address)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!handle || !proc.Space.IsValidPointer(handle))
		return SyscallResult::InvalidPointer;
	if (!address || !proc.Space.IsValidPointer(address))
		return SyscallResult::InvalidPointer;
	if (!size)
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

//Map a shared-memory handle the caller owns (created or claimed) into its address
//space. Idempotent: returns the existing mapping if already mapped.
SyscallResult MapSharedMemory(const HSharedMem handle, void** address)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!address || !proc.Space.IsValidPointer(address))
		return SyscallResult::InvalidPointer;

	UObject* const obj = proc.GetObject((handle_t)handle);
	if (!obj || obj->Type != UObjectType::SharedMemory)
		return SyscallResult::InvalidHandle;

	if (!obj->Shm)
		return SyscallResult::InvalidObject;

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

//Mint a single-use capability token for a handle the caller owns, so it can be
//handed to another process (over the endpoint registry / an IPC ring). Generic
//over handle types; only shared memory is grantable today.
SyscallResult ShareHandle(const Handle handle, uint64_t* token)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!token || !proc.Space.IsValidPointer(token))
		return SyscallResult::InvalidPointer;

	UObject* const obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;

	switch (obj->Type)
	{
		case UObjectType::SharedMemory:
			if (!obj->Shm)
				return SyscallResult::InvalidObject;
			*token = KeGrantShare(UObjectType::SharedMemory, obj->Shm);
			return SyscallResult::Success;

		default:
			return SyscallResult::InvalidObject; //not shareable yet
	}
}

//Redeem a grant token: install a fresh handle to the same object in the caller.
//The handle is not mapped/opened here; use the type's own call (e.g. MapSharedMemory).
SyscallResult ClaimHandle(const uint64_t token, Handle* handle)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!handle || !proc.Space.IsValidPointer(handle))
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

//Map the linear framebuffer into the calling process (used by the usermode WM).
SyscallResult MapFramebuffer(GraphicsDevice* device)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!device || !proc.Space.IsValidPointer(device))
		return SyscallResult::InvalidPointer;

	const KFramebufferInfo fb = KeGetFramebuffer();
	void* const va = KeVirtualMap(proc, fb.Base, fb.Size);
	if (!va)
		return SyscallResult::Failed;

	device->FrameBase = va;
	device->Width = fb.Width;
	device->Height = fb.Height;
	device->PixelsPerScanLine = fb.Pitch;
	return SyscallResult::Success;
}

/*
 * 0x900. IPC endpoint registry (declared alongside 0x600 for locality).
 */
SyscallResult RegisterEndpoint(const char* name, const uint64_t id)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!name || !proc.Space.IsValidPointer(name))
		return SyscallResult::InvalidPointer;

	return KeEndpointRegister(name, id) ? SyscallResult::Success : SyscallResult::Failed;
}

SyscallResult LookupEndpoint(const char* name, uint64_t* id)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!name || !proc.Space.IsValidPointer(name))
		return SyscallResult::InvalidPointer;
	if (!id || !proc.Space.IsValidPointer(id))
		return SyscallResult::InvalidPointer;

	return KeEndpointLookup(name, *id) ? SyscallResult::Success : SyscallResult::Failed;
}

SyscallResult PostEndpoint(const char* name, const uint64_t value)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!name || !proc.Space.IsValidPointer(name))
		return SyscallResult::InvalidPointer;

	return KeEndpointPost(name, value) ? SyscallResult::Success : SyscallResult::Failed;
}

SyscallResult PollEndpoint(const char* name, uint64_t* value)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!name || !proc.Space.IsValidPointer(name))
		return SyscallResult::InvalidPointer;
	if (!value || !proc.Space.IsValidPointer(value))
		return SyscallResult::InvalidPointer;

	return KeEndpointPoll(name, *value) ? SyscallResult::Success : SyscallResult::Failed;
}

/*
 * 0x700.
 */
SyscallResult DebugPrint(const char* s)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(s))
		return SyscallResult::InvalidPointer;

	Printf(s);
	return SyscallResult::Success;
}

SyscallResult DebugPrintBytes(const void* s, const size_t length)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(s))
		return SyscallResult::InvalidPointer;

	if (!length)
		return SyscallResult::InvalidArg;

	PrintBytes(s, length);
	return SyscallResult::Success;
}

//TODO(tsharpe): Untested
SyscallResult DebugPrintStack(const uint64_t rip)
{
	UThread& thread = Scheduler::GetUThread();
	thread.Process.Display();

	Arch::Context context = {};
	context.Rip = rip;
	context.Rsp = (uintptr_t)thread.Stack;
	context.Rbp = *(uintptr_t*)thread.Stack;
	PrintStack(&context, thread.Process);

	return SyscallResult::Success;
}

/*
* 0x800: Network. sockaddr_in is network byte order; the stack uses host order, so
* convert at this boundary (ByteSwap on little-endian == hton/ntoh).
*/
namespace
{
	bool ToEndpoint(const sockaddr_in& sa, Net::endpoint_t& out)
	{
		if (sa.sin_family != AF_INET)
			return false;
		out.addr.addr = ByteSwap<uint32_t>(sa.sin_addr.s_addr);
		out.port = ByteSwap<uint16_t>(sa.sin_port);
		return true;
	}

	void FromEndpoint(const Net::endpoint_t& ep, sockaddr_in& sa)
	{
		sa.sin_family = AF_INET;
		sa.sin_port = ByteSwap<uint16_t>(ep.port);
		sa.sin_addr.s_addr = ByteSwap<uint32_t>(ep.addr.addr);
		for (size_t i = 0; i < sizeof(sa.sin_zero); i++)
			sa.sin_zero[i] = 0;
	}

	KSocket* GetSocket(UProcess& proc, const HSocket sock)
	{
		UObject* obj = proc.GetObject((handle_t)sock);
		if (!obj || obj->Type != UObjectType::Socket)
			return nullptr;
		return obj->Socket;
	}

	Net::ipv4_addr_t ToIpv4(const in_addr& a)
	{
		Net::ipv4_addr_t out;
		out.addr = ByteSwap<uint32_t>(a.s_addr);
		return out;
	}

	in_addr FromIpv4(const Net::ipv4_addr_t& a)
	{
		in_addr out;
		out.s_addr = ByteSwap<uint32_t>(a.addr);
		return out;
	}
}

HSocket SocketCreate(int af, int type, int protocol)
{
	if (af != AF_INET)
		return INVALID_SOCKET;

	UProcess& proc = Scheduler::GetUProcess();

	KSocket* socket = KeSocketCreate(type, protocol);
	if (!socket)
		return INVALID_SOCKET;

	UObject* obj = proc.CreateObject(UObjectType::Socket);
	if (!obj)
	{
		KeSocketClose(*socket);
		KeFree(socket, AllocType::Kernel);
		return INVALID_SOCKET;
	}

	obj->Socket = socket;
	return (HSocket)obj->Handle;
}

SyscallResult SocketBind(HSocket sock, const sockaddr_in* addr)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(addr))
		return SyscallResult::InvalidPointer;

	KSocket* socket = GetSocket(proc, sock);
	if (!socket)
		return SyscallResult::InvalidHandle;

	Net::endpoint_t local;
	if (!ToEndpoint(*addr, local))
		return SyscallResult::InvalidArg;

	return KeSocketBind(*socket, local) ? SyscallResult::Success : SyscallResult::Failed;
}

SyscallResult SocketConnect(HSocket sock, const sockaddr_in* peer)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(peer))
		return SyscallResult::InvalidPointer;

	KSocket* socket = GetSocket(proc, sock);
	if (!socket)
		return SyscallResult::InvalidHandle;

	Net::endpoint_t endpoint;
	if (!ToEndpoint(*peer, endpoint))
		return SyscallResult::InvalidArg;

	return KeSocketConnect(*socket, endpoint) ? SyscallResult::Success : SyscallResult::Failed;
}

SyscallResult SocketSendTo(HSocket sock, const void* buf, size_t len, const sockaddr_in* to)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(buf) || !proc.Space.IsValidPointer(to))
		return SyscallResult::InvalidPointer;
	if (!len)
		return SyscallResult::InvalidArg;

	KSocket* socket = GetSocket(proc, sock);
	if (!socket)
		return SyscallResult::InvalidHandle;

	Net::endpoint_t dst;
	if (!ToEndpoint(*to, dst))
		return SyscallResult::InvalidArg;

	return KeSocketSend(*socket, dst, buf, len) ? SyscallResult::Success : SyscallResult::Failed;
}

SyscallResult SocketSend(HSocket sock, const void* buf, size_t len)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(buf))
		return SyscallResult::InvalidPointer;
	if (!len)
		return SyscallResult::InvalidArg;

	KSocket* socket = GetSocket(proc, sock);
	if (!socket)
		return SyscallResult::InvalidHandle;
	if (!socket->Connected)
		return SyscallResult::InvalidArg;

	return KeSocketSend(*socket, socket->Peer, buf, len) ? SyscallResult::Success : SyscallResult::Failed;
}

SyscallResult SocketRecvFromSys(HSocket sock, SocketRecvParams* params)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(params))
		return SyscallResult::InvalidPointer;
	if (!proc.Space.IsValidPointer(params->buf))
		return SyscallResult::InvalidPointer;
	if (params->bytesRecv && !proc.Space.IsValidPointer(params->bytesRecv))
		return SyscallResult::InvalidPointer;
	if (params->from && !proc.Space.IsValidPointer(params->from))
		return SyscallResult::InvalidPointer;
	if (!params->maxLen)
		return SyscallResult::InvalidArg;

	KSocket* socket = GetSocket(proc, sock);
	if (!socket)
		return SyscallResult::InvalidHandle;

	Net::endpoint_t src = {};
	size_t bytes = 0;
	const Net::Socket::read_t result = KeSocketRecv(*socket, src, params->buf, params->maxLen, bytes, params->timeoutMs);

	if (params->bytesRecv)
		*params->bytesRecv = bytes;

	switch (result)
	{
		case Net::Socket::read_t::Success:
			if (params->from)
				FromEndpoint(src, *params->from);
			return SyscallResult::Success;

		case Net::Socket::read_t::Failure:
			return SyscallResult::BrokenPipe;

		default:
			//Empty / timeout: no datagram available
			return SyscallResult::Failed;
	}
}

SyscallResult SocketClose(HSocket sock)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!GetSocket(proc, sock))
		return SyscallResult::InvalidHandle;

	return proc.CloseObject((handle_t)sock) ? SyscallResult::Success : SyscallResult::Failed;
}

SyscallResult GetInterfaces(InterfaceInfo* buffer, size_t maxCount, size_t* count)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(buffer) || !proc.Space.IsValidPointer(count))
		return SyscallResult::InvalidPointer;

	const size_t total = KeNetInterfaceCount();
	size_t n = 0;
	for (size_t i = 0; i < total && n < maxCount; i++)
	{
		Net::ipv4_addr_t addr, subnet, gateway;
		Net::eth_mac_t mac;
		if (!KeNetGetInterface(i, addr, subnet, gateway, mac))
			continue;

		InterfaceInfo& info = buffer[n];
		info.index = (uint32_t)i;
		//MACs are stored little-endian internally; expose canonical (wire) order.
		for (size_t b = 0; b < sizeof(info.mac); b++)
			info.mac[b] = mac.bytes[sizeof(mac.bytes) - 1 - b];
		info.reserved = 0;
		info.addr = FromIpv4(addr);
		info.subnet = FromIpv4(subnet);
		info.gateway = FromIpv4(gateway);
		n++;
	}

	*count = n;
	return SyscallResult::Success;
}

SyscallResult GetInterfaceIp(uint32_t index, in_addr* addr, in_addr* subnet)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(addr) || !proc.Space.IsValidPointer(subnet))
		return SyscallResult::InvalidPointer;

	Net::ipv4_addr_t a, s, g;
	Net::eth_mac_t mac;
	if (!KeNetGetInterface(index, a, s, g, mac))
		return SyscallResult::InvalidArg;

	*addr = FromIpv4(a);
	*subnet = FromIpv4(s);
	return SyscallResult::Success;
}

SyscallResult SetInterfaceIp(uint32_t index, const in_addr* addr, const in_addr* subnet)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(addr) || !proc.Space.IsValidPointer(subnet))
		return SyscallResult::InvalidPointer;

	if (!KeNetSetInterface(index, ToIpv4(*addr), ToIpv4(*subnet)))
		return SyscallResult::InvalidArg;

	return SyscallResult::Success;
}

SyscallResult SetGateway(uint32_t index, const in_addr* gateway)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.Space.IsValidPointer(gateway))
		return SyscallResult::InvalidPointer;

	if (!KeNetSetGateway(index, ToIpv4(*gateway)))
		return SyscallResult::InvalidArg;

	return SyscallResult::Success;
}
