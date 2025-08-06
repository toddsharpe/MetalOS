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

uint64_t Dispatch(const SyscallFrame& frame)
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

		//0x300
		case Syscall::AllocWindow:
			return (uint64_t)AllocWindow((HWindow*)frame.Arg0, (Graphics::Rectangle*)frame.Arg1);

		case Syscall::PaintWindow:
			return (uint64_t)PaintWindow((HWindow)frame.Arg0, (Buffer*)frame.Arg1);

		case Syscall::MoveWindow:
			return (uint64_t)MoveWindow((HWindow)frame.Arg0, (Graphics::Rectangle*)frame.Arg1);

		case Syscall::GetWindowRect:
			return (uint64_t)GetWindowRect((HWindow)frame.Arg0, (Graphics::Rectangle*)frame.Arg1);

		case Syscall::GetMessage:
			return (uint64_t)GetMessage((Message*)frame.Arg0);

		case Syscall::PeekMessage:
			return (uint64_t)PeekMessage((Message*)frame.Arg0);

		case Syscall::GetScreenRect:
			return (uint64_t)GetScreenRect((Graphics::Rectangle*)frame.Arg0);

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

		//0x700
		case Syscall::DebugPrint:
			return (uint64_t)DebugPrint((char*)frame.Arg0);

		case Syscall::DebugPrintBytes:
			return (uint64_t)DebugPrintBytes((char*)frame.Arg0, (size_t)frame.Arg1);

		case Syscall::DebugPrintStack:
			return (uint64_t)DebugPrintStack(frame.UserIP);

		default:
			//Syscall not found
			Printf("Syscall not implemented: 0x%x\n", frame.SystemCall);
			Printf("  IP: 0x%016x\n", frame.UserIP);
			return (uint64_t)SyscallResult::NotImplemented;
	}
}

extern "C" uint64_t KeSyscall(const SyscallFrame& frame)
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
	if (!proc.IsValidPointer(time))
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
	if ((!proc.IsValidPointer(commandLine)) || (args && !proc.IsValidPointer(args)) || (result && !proc.IsValidPointer(result)))
		return SyscallResult::InvalidPointer;

	//Copy string locally to automatically crop it at max bytes
	StaticString<128> copy = {};
	copy.AppendClip(commandLine);

	UProcess* created = KeCreateProcess(copy);
	if (!created)
		return SyscallResult::Failed;

	if (result && proc.IsValidPointer(result))
	{
		//Add uobject to calling process
		UObject* obj = proc.CreateObject(UObjectType::Process);
		obj->Process = created;

		result->Process = (HProcess)obj->Handle;
	}

	//If console application, set standard handles
	if (created->IsConsole)
	{
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
			created->CreatePipe(backing, KPipeOp::Read, Handles::StdIn);
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

			//Set stdin to new process
			created->CreatePipe(backing, KPipeOp::Write, Handles::StdOut);
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

			//Set stdin to new process
			created->CreatePipe(backing, KPipeOp::Write, Handles::StdErr);
		}
	}

	return SyscallResult::Success;
}

HThread CreateThread(size_t stackSize, ThreadStart startAddress, void* arg)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.IsValidPointer(startAddress))
		return nullptr;

	UThread* created = KeCreateUThread(proc, stackSize, startAddress, arg);

	//Add uobject to calling process
	UObject* obj = proc.CreateObject(UObjectType::Thread);
	obj->Thread = created;

	return (HThread)obj->Handle;
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
 * 0x300/
 */
SyscallResult AllocWindow(HWindow* handle, const Graphics::Rectangle* frame)
{
	UThread& thread = Scheduler::GetUThread();
	UProcess& proc = thread.Process;
	if (!handle || !frame)
		return SyscallResult::InvalidArg;
	if (!proc.IsValidPointer(handle) || !proc.IsValidPointer(frame))
		return SyscallResult::InvalidPointer;

	UWindow* window = CreateWindow(thread);
	window->Frame = {nullptr, frame->Height, frame->Width};
	window->Frame.Buffer = (Graphics::Color*)KeVirtualAlloc(window->Frame.Size());
	window->Point = { frame->X, frame->Y };

	UObject* obj = proc.CreateObject(UObjectType::Window);
	obj->Window = window;
	obj->Display();

	*handle = (HWindow)obj->Handle;
	return SyscallResult::Success;
}

SyscallResult PaintWindow(HWindow handle, const Buffer* buffer)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!buffer)
		return SyscallResult::InvalidArg;
	if (!proc.IsValidPointer(buffer))
		return SyscallResult::InvalidPointer;
	
	UObject* obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;
	if (obj->Type != UObjectType::Window)
		return SyscallResult::InvalidObject;

	UWindow* window = obj->Window;
	Assert(window->Frame.Buffer);

	memcpy(window->Frame.Buffer, buffer->Data, buffer->Length);
	return SyscallResult::Success;
}

SyscallResult MoveWindow(HWindow handle, const Graphics::Rectangle* frame)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!frame)
		return SyscallResult::InvalidArg;
	if (!proc.IsValidPointer(frame))
		return SyscallResult::InvalidPointer;
	
	UObject* obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;
	if (obj->Type != UObjectType::Window)
		return SyscallResult::InvalidObject;

	UWindow* window = obj->Window;

	//TODO(tsharpe): Dealloc. For now enforce bounds are the same
	if ((window->Frame.Height != frame->Height) || (window->Frame.Width != frame->Width))
		return SyscallResult::Failed;
	
	window->Point = { frame->X, frame->Y };
	return SyscallResult::Success;
}

SyscallResult GetWindowRect(HWindow handle, Graphics::Rectangle* frame)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!frame)
		return SyscallResult::InvalidArg;
	if (!proc.IsValidPointer(frame))
		return SyscallResult::InvalidPointer;
	
	UObject* obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;
	if (obj->Type != UObjectType::Window)
		return SyscallResult::InvalidObject;

	UWindow* window = obj->Window;
	*frame = { window->Point.X, window->Point.Y, window->Frame.Width, window->Frame.Height };
	return SyscallResult::Success;
}

SyscallResult GetMessage(Message* message)
{
	UThread& thread = Scheduler::GetUThread();
	UProcess& proc = thread.Process;
	if (!message)
		return SyscallResult::InvalidArg;
	if (!proc.IsValidPointer(message))
		return SyscallResult::InvalidPointer;

	//Wait for message
	KPredicate signal(&UThread::HasMessage, &thread);
	KeWait(signal);

	Assert(thread.HasMessage());
	thread.Dequeue(*message);

	return SyscallResult::Success;
}

SyscallResult PeekMessage(Message* message)
{
	UThread& thread = Scheduler::GetUThread();
	UProcess& proc = thread.Process;
	if (!message)
		return SyscallResult::InvalidArg;
	if (!proc.IsValidPointer(message))
		return SyscallResult::InvalidPointer;

	if (!thread.HasMessage())
		return SyscallResult::Failed;

	Assert(thread.Dequeue(*message));
	return SyscallResult::Success;
}

SyscallResult GetScreenRect(Graphics::Rectangle* rect)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!rect)
		return SyscallResult::InvalidArg;
	if (!proc.IsValidPointer(rect))
		return SyscallResult::InvalidPointer;
	
	*rect = GetScreenRect2();
	Printf("X: %d, Y: %d\n", rect->X, rect->Y);
	return SyscallResult::Success;
}

/*
* 0x400.
*/
HFile CreateFile(const char* path, const FileAccess access)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.IsValidPointer(path))
		return nullptr;

	UObject* uFile = proc.CreateObject(UObjectType::File);
	if (!uFile)
		return nullptr;

	StaticString<128> copy;
	copy.AppendClip(path);
	copy.ToLower();

	const bool result = KeCreateFile(uFile->File, copy, ToAccess(access));
	if (!result)
		return nullptr;
	return (HFile)uFile->Handle;
}

SyscallResult ReadFile(const HFile handle, void* buffer, const size_t bufferSize, size_t* bytesRead)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.IsValidPointer(buffer) || (bytesRead && !proc.IsValidPointer(bytesRead)))
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
	if (!proc.IsValidPointer(buffer))
		return SyscallResult::InvalidPointer;
	if (!bufferSize)
		return SyscallResult::InvalidArg;
	if (bytesWritten && !proc.IsValidPointer(bytesWritten))
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
	if (newPosition && !proc.IsValidPointer(newPosition))
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
	if (!readHandle || !proc.IsValidPointer(readHandle))
		return SyscallResult::InvalidPointer;
	if (!writeHandle || !proc.IsValidPointer(writeHandle))
		return SyscallResult::InvalidPointer;

	const size_t pipeSize = PageSize;
	KPipe* created = KeCreatePipe(pipeSize);
	created->Readers.Increment();
	created->Writers.Increment();

	UObject* reader = proc.CreatePipe(*created, KPipeOp::Read);
	*readHandle = (HFile)reader->Handle;

	UObject* writer = proc.CreatePipe(*created, KPipeOp::Write);
	*writeHandle = (HFile)writer->Handle;
	
	return SyscallResult::Success;
}

SyscallResult PeekNamedPipe(const HFile file, size_t* bytesAvailable)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!bytesAvailable || !proc.IsValidPointer(bytesAvailable))
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
		case UObjectType::Window:
			Delete(*obj->Window);
			break;

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
			//do nothing
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
	if (!status || !proc.IsValidPointer(status))
		return SyscallResult::InvalidPointer;

	UObject* obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;

	switch (obj->Type)
	{
		case UObjectType::Process:
			*status = FromWaitResult(KeWait(*obj->Process, time));
			break;

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
	if (!event || !proc.IsValidPointer(event))
		return SyscallResult::InvalidPointer;

	//Create Event
	UObject* created = proc.CreateEvent(manual, initial);
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
	
	return KeVirtualAlloc(proc, address, size);
}

/*
 * 0x700.
 */
SyscallResult DebugPrint(const char* s)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.IsValidPointer(s))
		return SyscallResult::InvalidPointer;

	Printf(s);
	return SyscallResult::Success;
}

SyscallResult DebugPrintBytes(const void* s, const size_t length)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!proc.IsValidPointer(s))
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

	Context context = {};
	context.Rip = rip;
	context.Rsp = (uintptr_t)thread.Stack;
	context.Rbp = *(uintptr_t*)thread.Stack;
	PrintStack(&context, thread.Process);
	
	return SyscallResult::Success;
}
