#include "MetalOS.Syscalls.h"
#include "kernel/Syscalls/Syscalls.h"

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

static uint64_t Dispatch(const Arch::SyscallFrame& frame)
{
	Syscall call = static_cast<Syscall>(frame.SystemCall);
	switch (call)
	{
		//0x100: System
		case Syscall::GetTickCount:
			return (uint64_t)GetTickCount();

		case Syscall::GetSystemTime:
			return (uint64_t)GetSystemTime((SystemTime*)frame.Arg0);

		//0x200: Processes
		case Syscall::CreateProcess:
			return (uint64_t)CreateProcess((char*)frame.Arg0, (CreateProcessArgs*)frame.Arg1, (CreateProcessResult*)frame.Arg2);

		case Syscall::TerminateProcess:
			return (uint64_t)TerminateProcess((HProcess)frame.Arg0, (uint32_t)frame.Arg1);

		case Syscall::ExitProcess:
			ExitProcess((uint32_t)frame.Arg0);
			return 0;

		case Syscall::IsProcessAlive:
			return (uint64_t)IsProcessAlive((uint32_t)frame.Arg0);

		case Syscall::GetProcesses:
			return (uint64_t)GetProcesses((ProcessListEntry*)frame.Arg0, (size_t)frame.Arg1, (size_t*)frame.Arg2);

		//0x300: Threads
		case Syscall::GetCurrentThread:
			return (uint64_t)GetCurrentThread();

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

		case Syscall::TerminateThread:
			return (uint64_t)TerminateThread((HThread)frame.Arg0, (uint32_t)frame.Arg1);

		case Syscall::ExitThread:
			ExitThread((uint32_t)frame.Arg0);
			return 0;

		//0x400: Files and pipes
		case Syscall::CreateFile:
			return (uint64_t)CreateFile((char*)frame.Arg0, (FileAccess)frame.Arg1, (FileCreate)frame.Arg2);

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

		case Syscall::ListDirectory:
			return (uint64_t)ListDirectory((char*)frame.Arg0, (DirectoryEntry*)frame.Arg1, (size_t)frame.Arg2, (size_t*)frame.Arg3);

		//0x500: Handles (closing, and passing one to another process)
		case Syscall::CloseHandle:
			return (uint64_t)CloseHandle((Handle)frame.Arg0);

		case Syscall::ShareHandle:
			return (uint64_t)ShareHandle((Handle)frame.Arg0, (uint64_t*)frame.Arg1);

		case Syscall::ClaimHandle:
			return (uint64_t)ClaimHandle((uint64_t)frame.Arg0, (Handle*)frame.Arg1);

		//0x600: Synchronization
		case Syscall::WaitForSingleObject:
			return (uint64_t)WaitForSingleObject((Handle)frame.Arg0, (milli_t)frame.Arg1, (WaitStatus*)frame.Arg2);

		case Syscall::CreateEvent:
			return (uint64_t)CreateEvent((HEvent*)frame.Arg0, (bool)frame.Arg1, (bool)frame.Arg2);

		case Syscall::SetEvent:
			return (uint64_t)SetEvent((HEvent)frame.Arg0);

		case Syscall::ResetEvent:
			return (uint64_t)ResetEvent((HEvent)frame.Arg0);

		//0x700: Virtual memory
		case Syscall::VirtualAlloc:
			return (uintptr_t)VirtualAlloc((void*)frame.Arg0, (size_t)frame.Arg1);

		case Syscall::VirtualFree:
			return (uint64_t)VirtualFree((void*)frame.Arg0);

		//0x800: Shared memory
		case Syscall::CreateSharedMemory:
			return (uint64_t)CreateSharedMemory((size_t)frame.Arg0, (HSharedMem*)frame.Arg1, (void**)frame.Arg2);

		case Syscall::MapSharedMemory:
			return (uint64_t)MapSharedMemory((HSharedMem)frame.Arg0, (void**)frame.Arg1);

		case Syscall::DeleteSharedMemory:
			return (uint64_t)DeleteSharedMemory((HSharedMem)frame.Arg0);

		//0x900: Display
		case Syscall::MapFramebuffer:
			return (uint64_t)MapFramebuffer((GraphicsDevice*)frame.Arg0);

		case Syscall::FlushFramebuffer:
			return (uint64_t)FlushFramebuffer();

		//0xA00: Network interfaces (TCP/IP is the usermode netstack)
		case Syscall::NetSend:
			return (uint64_t)NetSend((uint32_t)frame.Arg0, (const void*)frame.Arg1, (size_t)frame.Arg2);

		case Syscall::GetInterfaces:
			return (uint64_t)GetInterfaces((InterfaceInfo*)frame.Arg0, (size_t)frame.Arg1, (size_t*)frame.Arg2);

		//0xB00: IPC endpoints (name to capability token rendezvous)
		case Syscall::RegisterEndpoint:
			return (uint64_t)RegisterEndpoint((const char*)frame.Arg0, (uint64_t)frame.Arg1);

		case Syscall::LookupEndpoint:
			return (uint64_t)LookupEndpoint((const char*)frame.Arg0, (uint64_t*)frame.Arg1);

		case Syscall::PostEndpoint:
			return (uint64_t)PostEndpoint((const char*)frame.Arg0, (uint64_t)frame.Arg1);

		case Syscall::PollEndpoint:
			return (uint64_t)PollEndpoint((const char*)frame.Arg0, (uint64_t*)frame.Arg1);

		//0xC00: Debug
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

extern "C" uint64_t KeSyscall(const Arch::SyscallFrame& frame)
{
	//Printf("Syscall: 0x%x\n", frame.SystemCall);
	const uint64_t ret = Dispatch(frame);
	//Printf("    Result: 0x%016x\n", ret);
	return ret;
}
