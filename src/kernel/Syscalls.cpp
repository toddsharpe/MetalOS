#include "MetalOS.Syscalls.h"
#include "kernel/Scheduler.h"
#include "kernel/Arch.h"
#include "user/MetalOS.h"
#include "Lib/String.h"
#include "kernel/Api.h"
#include "kernel/Objects/KPredicate.h"
#include "kernel/Objects/KPipe.h"
#include "kernel/Objects/KSharedMemory.h"
#include "kernel/Drivers/Driver.h"
#include "Assert.h"

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

	//Longer debug output is truncated by Printf's buffer anyway
	static constexpr size_t MaxDebugPrint = 1024;

	//Length of a NUL-terminated user string, never reading past maxLength or the process's reservations
	size_t UserStringLength(const UProcess& proc, const char* const s, const size_t maxLength)
	{
		size_t length = 0;
		while (length < maxLength)
		{
			const char* const c = s + length;
			const bool newPage = (length == 0) || (((uintptr_t)c & Arch::PageMask) == 0);
			if (newPage && !proc.Space.IsValidRange(c, 1))
				break;
			if (*c == '\0')
				break;
			length++;
		}
		return length;
	}

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

	//Look up a caller's handle that must refer to an object of the given type
	SyscallResult GetObject(UProcess& proc, const Handle handle, const UObjectType type, UObject*& obj)
	{
		obj = proc.GetObject((handle_t)handle);
		if (!obj)
			return SyscallResult::InvalidHandle;
		if (obj->Type != type)
			return SyscallResult::InvalidObject;

		return SyscallResult::Success;
	}

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

	//Keeps a pipe alive while a syscall can block on it, as another thread may close the handle meanwhile
	class PipeHold
	{
	public:
		PipeHold(UThread& thread, KPipe& pipe) :
			m_thread(thread)
		{
			Assert(!thread.PipeWait);
			pipe.Waiters.Increment();
			thread.PipeWait = &pipe;
		}

		//Never runs for a thread killed while blocked, so the reaper drops its hold instead
		~PipeHold()
		{
			KPipe& pipe = *m_thread.PipeWait;
			m_thread.PipeWait = nullptr;
			KePipeUnref(pipe, pipe.Waiters);
		}

	private:
		UThread& m_thread;
	};

	//Keeps an event alive while a thread waits on it, as another thread may close the handle meanwhile; the wait goes on, as on Windows
	class EventHold
	{
	public:
		EventHold(UThread& thread, KEvent& event) :
			m_thread(thread)
		{
			Assert(!thread.EventWait);
			event.Refs.Increment();
			thread.EventWait = &event;
		}

		//Never runs for a thread killed while waiting, so the reaper drops its hold instead
		~EventHold()
		{
			KEvent& event = *m_thread.EventWait;
			m_thread.EventWait = nullptr;
			KeEventUnref(event);
		}

	private:
		UThread& m_thread;
	};
}

//Syscalls that the kernel adds params
static SyscallResult DebugPrintStack(const uint64_t rip);

static uint64_t Dispatch(const Arch::SyscallFrame& frame)
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

		case Syscall::GetProcesses:
			return (uint64_t)GetProcesses((ProcessListEntry*)frame.Arg0, (size_t)frame.Arg1, (size_t*)frame.Arg2);

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

		case Syscall::VirtualFree:
			return (uint64_t)VirtualFree((void*)frame.Arg0);

		case Syscall::CreateSharedMemory:
			return (uint64_t)CreateSharedMemory((size_t)frame.Arg0, (HSharedMem*)frame.Arg1, (void**)frame.Arg2);

		case Syscall::MapSharedMemory:
			return (uint64_t)MapSharedMemory((HSharedMem)frame.Arg0, (void**)frame.Arg1);

		case Syscall::DeleteSharedMemory:
			return (uint64_t)DeleteSharedMemory((HSharedMem)frame.Arg0);

		//0x300
		case Syscall::MapFramebuffer:
			return (uint64_t)MapFramebuffer((GraphicsDevice*)frame.Arg0);
		case Syscall::FlushFramebuffer:
			return (uint64_t)FlushFramebuffer();

		case Syscall::GetInterfaces:
			return (uint64_t)GetInterfaces((InterfaceInfo*)frame.Arg0, (size_t)frame.Arg1, (size_t*)frame.Arg2);

		//0x700
		case Syscall::DebugPrint:
			return (uint64_t)DebugPrint((char*)frame.Arg0);

		case Syscall::DebugPrintBytes:
			return (uint64_t)DebugPrintBytes((char*)frame.Arg0, (size_t)frame.Arg1);

		case Syscall::DebugPrintStack:
			return (uint64_t)DebugPrintStack(frame.UserIP);

		//0x302: raw ethernet frame transmit (netstack owns the TCP/IP stack)
		case Syscall::NetSend:
			return (uint64_t)NetSend((uint32_t)frame.Arg0, (const void*)frame.Arg1, (size_t)frame.Arg2);

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
	char copy[128];
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

	//The pointer check covers only the first byte, so bound the copy by what's mapped
	char copy[128];
	StrCopy(copy, CStr(path, UserStringLength(proc, path, sizeof(copy) - 1)));
	for (char* c = copy; *c != '\0'; c++)
		*c = (char)tolower(*c);

	const bool result = KeCreateFile(uFile->File, copy, ToAccess(access));
	if (!result)
	{
		//Don't leave an unopened file object behind
		proc.CloseObject(uFile->Handle);
		return nullptr;
	}
	return (HFile)uFile->Handle;
}

SyscallResult ReadFile(const HFile handle, void* buffer, const size_t bufferSize, size_t* bytesRead)
{
	UProcess& proc = Scheduler::GetUProcess();

	//A zero-length read touches no buffer, as on Windows
	if ((bufferSize && !proc.Space.IsValidRange(buffer, bufferSize)) || (bytesRead && !proc.Space.IsValid(bytesRead)))
		return SyscallResult::InvalidPointer;

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
			//The handle can be closed during the wait, so use the pipe itself and hold it until done
			KPipe& pipe = *obj->Pipe;
			PipeHold hold(Scheduler::GetUThread(), pipe);

			//Wait for at least one byte or for every write end to close; readers woken by the same bytes take turns, so one that finds the pipe empty again waits again
			while (pipe.Count() == 0 && !pipe.Writers.IsEmpty())
			{
				KPipeSignal signal(pipe, KPipeOp::Read, 1);
				KPredicate wait(&KPipe::EventSignal, &signal);
				KWaitResult result = KeWait(wait);
				Assert(result == KWaitResult::Signaled);
			}

			//Another thread can also free the buffer or the count during the wait, so check both again before using either
			if ((bufferSize && !proc.Space.IsValidRange(buffer, bufferSize)) || (bytesRead && !proc.Space.IsValid(bytesRead)))
				return SyscallResult::InvalidPointer;

			//Bytes a writer left before closing are still returned; only an empty pipe with no write end left is the end
			if (pipe.Count() == 0)
			{
				if (bytesRead)
					*bytesRead = 0;
				return SyscallResult::BrokenPipe;
			}

			//Read what's there, up to the buffer; a zero-length read only waits for data
			size_t bytes = 0;
			Assert(pipe.Read(buffer, bufferSize, bytes));
			if (bytesRead)
				*bytesRead = bytes;
			return SyscallResult::Success;
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
	if (!bufferSize)
		return SyscallResult::InvalidArg;
	if (!proc.Space.IsValidRange(buffer, bufferSize) || (bytesWritten && !proc.Space.IsValid(bytesWritten)))
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
			//The handle can be closed during the wait, so use the pipe itself and hold it until done
			KPipe& pipe = *obj->Pipe;
			UThread& writer = Scheduler::GetUThread();
			PipeHold hold(writer, pipe);
			const uint8_t* const data = static_cast<const uint8_t*>(buffer);

			//A write that fits in the pipe goes in whole, a larger one in pieces as the reader drains it
			const size_t needed = (bufferSize <= pipe.Capacity()) ? bufferSize : 1;

			//Writes are queued in arrival order and each finishes before the next starts, so writers' bytes never interleave
			pipe.QueueWriter(writer.PipeWriteLink);
			writer.PipeWriteQueue = &pipe;

			size_t written = 0;
			SyscallResult status = SyscallResult::Success;
			while (written < bufferSize)
			{
				//Wait for this write's turn and for room
				KPipeSignal signal(pipe, KPipeOp::Write, needed, &writer.PipeWriteLink);
				KPredicate wait(&KPipe::EventSignal, &signal);
				KWaitResult result = KeWait(wait);
				Assert(result == KWaitResult::Signaled);

				//The wait also ends when the reader is gone, with possibly no room to write
				if (pipe.IsBroken())
				{
					status = SyscallResult::BrokenPipe;
					break;
				}

				//Another thread of the process may have freed the rest of the buffer during the wait
				if (!proc.Space.IsValidRange(data + written, bufferSize - written))
				{
					status = SyscallResult::InvalidPointer;
					break;
				}

				size_t bytes = 0;
				const size_t piece = Min(bufferSize - written, pipe.Space());
				Assert(pipe.Write(data + written, piece, bytes));
				written += bytes;
			}

			pipe.DequeueWriter(writer.PipeWriteLink);
			writer.PipeWriteQueue = nullptr;

			//The count may have been freed during a wait as well
			if (bytesWritten)
			{
				if (!proc.Space.IsValid(bytesWritten))
					return SyscallResult::InvalidPointer;
				*bytesWritten = written;
			}
			return status;
		}
		break;

		case UObjectType::Debug:
		{
			//User text is data, never a format string. Buffer was validated for bufferSize bytes.
			const char* const s = (char*)buffer;
			const size_t length = (bufferSize < MaxDebugPrint) ? bufferSize : MaxDebugPrint;
			Printf("%.*s", (int)length, s);
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
	if (newPosition && !proc.Space.IsValid(newPosition))
		return SyscallResult::InvalidPointer;

	UObject* uFile = nullptr;
	const SyscallResult status = GetObject(proc, handle, UObjectType::File, uFile);
	if (status != SyscallResult::Success)
		return status;

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
			newPos = uFile->File.Length + position;
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

//The only storage is the read-only RAM drive, so nothing can be renamed, deleted or created
SyscallResult MoveFile(const char* existingFileName, const char* newFileName)
{
	return SyscallResult::NotImplemented;
}

SyscallResult DeleteFile(const char* fileName)
{
	return SyscallResult::NotImplemented;
}

SyscallResult CreateDirectory(const char* path)
{
	return SyscallResult::NotImplemented;
}

SyscallResult CreatePipe(HFile* readHandle, HFile* writeHandle)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!readHandle || !proc.Space.IsValid(readHandle))
		return SyscallResult::InvalidPointer;
	if (!writeHandle || !proc.Space.IsValid(writeHandle))
		return SyscallResult::InvalidPointer;

	static constexpr size_t pipeSize = Arch::PageSize;
	
	//Create kernel pipe; out of memory fails the call with nothing left allocated
	KPipe* pipe = KeAlloc<KPipe>(AllocType::Kernel, pipeSize);
	if (!pipe)
		return SyscallResult::Failed;
	if (!pipe->Init())
	{
		KeFree(pipe, AllocType::Kernel);
		return SyscallResult::Failed;
	}
	pipe->Readers.Increment();
	pipe->Writers.Increment();

	//Create reader
	{
		UObject* reader = proc.CreateObject(UObjectType::Pipe);
		reader->Pipe = pipe;
		reader->PipeOp = KPipeOp::Read;
		*readHandle = (HFile)reader->Handle;
	}

	//Create writer
	{
		UObject* writer = proc.CreateObject(UObjectType::Pipe);
		writer->Pipe = pipe;
		writer->PipeOp = KPipeOp::Write;
		*writeHandle = (HFile)writer->Handle;
	}

	return SyscallResult::Success;
}

SyscallResult PeekNamedPipe(const HFile file, size_t* bytesAvailable)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!bytesAvailable || !proc.Space.IsValid(bytesAvailable))
		return SyscallResult::InvalidPointer;

	UObject* obj = nullptr;
	const SyscallResult status = GetObject(proc, file, UObjectType::Pipe, obj);
	if (status != SyscallResult::Success)
		return status;

	*bytesAvailable = obj->Pipe->Count();
	return obj->Pipe->IsBroken() ? SyscallResult::BrokenPipe : SyscallResult::Success;
}

SyscallResult CloseHandle(const Handle handle)
{
	UProcess& proc = Scheduler::GetUProcess();

	//Type-specific teardown happens in UProcess::CloseObject (shared with KeTerminateProcess)
	return proc.CloseObject((handle_t)handle) ? SyscallResult::Success : SyscallResult::InvalidHandle;
}

/*
 * 0x500.
 */
SyscallResult WaitForSingleObject(const Handle handle, const milli_t time, WaitStatus* status)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!status || !proc.Space.IsValid(status))
		return SyscallResult::InvalidPointer;

	UObject* obj = proc.GetObject((handle_t)handle);
	if (!obj)
		return SyscallResult::InvalidHandle;

	switch (obj->Type)
	{
		case UObjectType::Event:
		{
			KEvent& event = *obj->Event;
			EventHold hold(Scheduler::GetUThread(), event);
			*status = FromWaitResult(KeWait(event, time));
		}
		break;

		default:
			return SyscallResult::InvalidObject;
	}

	return SyscallResult::Success;
}

SyscallResult CreateEvent(HEvent* event, const bool manual, const bool initial)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!event || !proc.Space.IsValid(event))
		return SyscallResult::InvalidPointer;

	//Create Event
	UObject* created = proc.CreateObject(UObjectType::Event);
	created->Event = KeAlloc<KEvent>(AllocType::User, manual, initial);
	created->Event->Refs.Increment(); //The handle's, dropped when it's closed

	*event = (HEvent)created->Handle;
	return SyscallResult::Success;
}

SyscallResult SetEvent(const HEvent event)
{
	UProcess& proc = Scheduler::GetUProcess();
	UObject* obj = nullptr;
	const SyscallResult status = GetObject(proc, event, UObjectType::Event, obj);
	if (status != SyscallResult::Success)
		return status;

	Assert(obj->Event);
	obj->Event->Set();
	return SyscallResult::Success;
}

SyscallResult ResetEvent(const HEvent event)
{
	UProcess& proc = Scheduler::GetUProcess();
	UObject* obj = nullptr;
	const SyscallResult status = GetObject(proc, event, UObjectType::Event, obj);
	if (status != SyscallResult::Success)
		return status;

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
	if (!size || size > UserEnd) //Larger can never fit, and would overflow the page count
		return nullptr;

	if (!address)
		return KeVirtualAlloc(proc, size);

	//Fixed addresses must be page aligned (the allocator asserts on it)
	if (((uintptr_t)address & Arch::PageMask) != 0)
		return nullptr;

	return KeVirtualAlloc(proc, address, size);
}

bool VirtualFree(const void* address)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!address)
		return false;

	//Must be the start of a reservation (the allocator asserts otherwise)
	if (proc.Space.GetCount((uintptr_t)address) == 0)
		return false;

	//Only VirtualAlloc'd memory: shared memory and the framebuffer are released through their handles
	if (!proc.Space.OwnsPages((uintptr_t)address))
		return false;

	return KeVirtualFree(proc, address);
}

SyscallResult CreateSharedMemory(const size_t size, HSharedMem* handle, void** address)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!handle || !proc.Space.IsValid(handle))
		return SyscallResult::InvalidPointer;
	if (!address || !proc.Space.IsValid(address))
		return SyscallResult::InvalidPointer;
	if (!size || size > UserEnd)
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
	if (!address || !proc.Space.IsValid(address))
		return SyscallResult::InvalidPointer;

	UObject* obj = nullptr;
	const SyscallResult status = GetObject(proc, handle, UObjectType::SharedMemory, obj);
	if (status != SyscallResult::Success)
		return status;

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

//Opposite of CreateSharedMemory: unmap our view and drop this handle's ref (the physical
//pages are freed once the last ref across all processes goes). Handle is invalid after.
SyscallResult DeleteSharedMemory(const HSharedMem handle)
{
	UProcess& proc = Scheduler::GetUProcess();

	UObject* obj = nullptr;
	const SyscallResult status = GetObject(proc, handle, UObjectType::SharedMemory, obj);
	if (status != SyscallResult::Success)
		return status;

	return proc.CloseObject((handle_t)handle) ? SyscallResult::Success : SyscallResult::Failed;
}

//Mint a single-use capability token for a handle the caller owns, so it can be
//handed to another process (over the endpoint registry / an IPC ring). Generic
//over handle types; only shared memory is grantable today.
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

//Redeem a grant token: install a fresh handle to the same object in the caller.
//The handle is not mapped/opened here; use the type's own call (e.g. MapSharedMemory).
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

//0x300: map the linear framebuffer into the calling process (used by the usermode WM).

SyscallResult MapFramebuffer(GraphicsDevice* device)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!device || !proc.Space.IsValid(device))
		return SyscallResult::InvalidPointer;

	//The display driver has the current (possibly host-negotiated) mode; without one (its init failed) use the boot GOP descriptor
	KDevice* const display = m_deviceTree.GetDeviceByClass(KDeviceClass::Display);
	DisplayDriver* const driver = display ? static_cast<DisplayDriver*>(display->Driver) : nullptr;
	const KFramebufferInfo fb = driver ? driver->GetFramebuffer() : KeGetFramebuffer();

	void* const va = KeVirtualMap(proc, fb.Base, fb.Size);
	if (!va)
		return SyscallResult::Failed;

	device->FrameBase = va;
	device->Width = fb.Width;
	device->Height = fb.Height;
	device->PixelsPerScanLine = fb.Pitch;
	return SyscallResult::Success;
}

//Present: ask the display driver to push the current framebuffer to the display (synthvid DIRT).
SyscallResult FlushFramebuffer()
{
	KDevice* const display = m_deviceTree.GetDeviceByClass(KDeviceClass::Display);
	if (!display || !display->Driver)
		return SyscallResult::Success; //no display driver (boot GOP is scanned out directly)

	static_cast<DisplayDriver*>(display->Driver)->Flush();
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
	char copy[MaxEndpointName];
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
	char copy[MaxEndpointName];
	if (!CopyEndpointName(proc, name, copy))
		return SyscallResult::InvalidArg;

	return KeEndpointLookup(copy, *id) ? SyscallResult::Success : SyscallResult::Failed;
}

SyscallResult PostEndpoint(const char* name, const uint64_t value)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!name || !proc.Space.IsValidPointer(name))
		return SyscallResult::InvalidPointer;
	char copy[MaxEndpointName];
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
	char copy[MaxEndpointName];
	if (!CopyEndpointName(proc, name, copy))
		return SyscallResult::InvalidArg;

	return KeEndpointPoll(copy, proc.Id, *value) ? SyscallResult::Success : SyscallResult::Failed;
}

/*
 * 0x700.
 */
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
static SyscallResult DebugPrintStack(const uint64_t rip)
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

//0x302: transmit a raw ethernet frame through the NIC (the TCP/IP stack is the usermode netstack)
SyscallResult NetSend(const uint32_t ifIdx, const void* frame, size_t length)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!length)
		return SyscallResult::InvalidArg;
	if (!proc.Space.IsValidRange(frame, length))
		return SyscallResult::InvalidPointer;

	return KeNetSend(ifIdx, frame, length) ? SyscallResult::Success : SyscallResult::Failed;
}

//0x300: Hardware interface enumeration -- NIC devices in the device tree, reported as index +
//MAC only. The IPv4 config is owned by the usermode netstack (MetalOS-NET IPC), not syscalls.
SyscallResult GetInterfaces(InterfaceInfo* buffer, const size_t maxCount, size_t* count)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (maxCount > static_cast<size_t>(-1) / sizeof(InterfaceInfo))
		return SyscallResult::InvalidArg; //Byte size would overflow
	if (!proc.Space.IsValidRange(buffer, maxCount * sizeof(InterfaceInfo)) || !proc.Space.IsValid(count))
		return SyscallResult::InvalidPointer;

	const size_t total = KeNetInterfaceCount();

	size_t n = 0;
	for (size_t i = 0; i < total && n < maxCount; i++)
	{
		NicDriver* driver = KeNetGetInterface(i);
		if (!driver)
			continue;

		InterfaceInfo& info = buffer[n];
		info.index = (uint32_t)i;
		driver->GetMac(info.mac);

		n++;
	}

	*count = n;
	return SyscallResult::Success;
}
