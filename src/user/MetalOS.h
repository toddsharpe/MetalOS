#pragma once

/******************************/
/*MetalOS Public Facing Header*/
/******************************/

#include "Lib/String.h"
#include "Lib/List.h"

#include "user/MetalOS.Keys.h"
#include "user/MetalOS.Types.h"
#include "user/MetalOS.UI.h"

//System Call interface is kept as C-compatible despite this header not being C-friendly
//This is to allow potential C# interop to be clean

#define RetNullIfNot(x) if (!(x)) return nullptr;
#define RetNullIfFailed(x) if ((x) != SyscallResult::Success) return nullptr;

#define AssertSuccess(EXP) \
	{ \
		const SyscallResult r = (EXP); \
		if (r != SyscallResult::Success) \
		{ \
			DebugPrintf("SyscallResult: %d\n", r); \
			Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  "Assert: " #EXP ", Result: 0x%x", r); \
		} \
	}

enum class SyscallResult
{
	Success = 0,
	InvalidPointer,
	InvalidArg,
	InvalidHandle,
	InvalidObject,
	BrokenPipe,
	Failed,
	NotImplemented
};

//TODO(tsharpe): Convert pointers to references where able
extern "C"
{
	//Provided by runtime
	uintptr_t GetProcAddress(HModule hModule, const char* lpProcName);
	SyscallResult GetProcessInfo(ProcessInfo* info);
	uint32_t GetCurrentThreadId();
	uint32_t GetLastError();
	void SetLastError(uint32_t errorCode);
	void DebugBreak();
	void DebugPrintf(const char* format, ...);
	void CDebugPrintf(const bool enabled, const char* format, ...);
	void Bugcheck(const char* file, const char* line, const char* format, ...);
	HModule LoadLibrary(char* lpLibFileName);
	uint32_t GetErrno();
	
	//0x100: System
	milli_t GetTickCount();
	SyscallResult GetSystemTime(SystemTime* time);
	
	//0x200: Threads/Processes
	HThread GetCurrentThread();
	SyscallResult CreateProcess(const char* commandLine, const CreateProcessArgs* args, CreateProcessResult* result);
	HThread CreateThread(size_t stackSize, ThreadStart startAddress, void* arg);
	uint32_t GetThreadId(HThread thread);
	void Sleep(const milli_t time);
	void SwitchToThread();
	SyscallResult SuspendThread(const HThread file);
	SyscallResult ResumeThread(const HThread file);
	SyscallResult TerminateProcess(const HProcess proc, const uint32_t exitCode);
	void ExitProcess(const uint32_t exitCode);
	SyscallResult TerminateThread(const HThread thread, const uint32_t exitCode);
	void ExitThread(const uint32_t exitCode);

	//0x300: Windowing
	SyscallResult AllocWindow(HWindow* handle, const Graphics::Rectangle* frame);
	SyscallResult PaintWindow(HWindow handle, const Buffer* buffer);
	SyscallResult MoveWindow(HWindow handle, const Graphics::Rectangle* frame);
	SyscallResult GetWindowRect(HWindow handle, Graphics::Rectangle* frame);
	SyscallResult GetMessage(Message* message);
	SyscallResult PeekMessage(Message* message);
	SyscallResult GetScreenRect(Graphics::Rectangle* rect);

	//0x400: Files/pipes
	HFile CreateFile(const char* path, const FileAccess access);
	SyscallResult ReadFile(const HFile handle, void* buffer, const size_t bufferSize, size_t* bytesRead);
	SyscallResult WriteFile(const HFile handle, const void* buffer, const size_t bufferSize, size_t* bytesWritten);
	SyscallResult SetFilePointer(const HFile handle, const ssize_t position, const Seek seek, size_t* newPosition);
	SyscallResult MoveFile(const char* existingFileName, const char* newFileName);
	SyscallResult DeleteFile(const char* fileName);
	SyscallResult CreateDirectory(const char* path);
	SyscallResult CreatePipe(HFile* readHandle, HFile* writeHandle);
	SyscallResult PeekNamedPipe(const HFile file, size_t* bytesAvailable);
	SyscallResult CloseHandle(const Handle handle);

	//0x500: Syncronization
	SyscallResult WaitForSingleObject(const Handle handle, const milli_t time, WaitStatus* status);
	SyscallResult CreateEvent(HEvent* event, const bool manual, const bool initial);
	SyscallResult SetEvent(const HEvent event);
	SyscallResult ResetEvent(const HEvent event);

	//0x600: Memory
	void* VirtualAlloc(const void* address, const size_t size);
	//HRingBuffer CreateRingBuffer(const char* name, const size_t indexSize, const size_t ringSize);
	//HSharedMemory CreateSharedMemory(const char* name, const size_t size);
	//void* MapObject(const void* address, Handle handle);
	//void* MapSharedObject(const void* address, const char* name);
	
	//0x700: Debug
	SyscallResult DebugPrint(const char* s);
	SyscallResult DebugPrintBytes(const void* s, const size_t length);
	SyscallResult DebugPrintStack();
}
