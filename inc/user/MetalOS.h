#pragma once

/******************************/
/*MetalOS Public Facing Header*/
/******************************/

//System Call interface is kept as C-compatible despite this header not being C-friendly
//This is to allow potential C# interop to be clean

#include <cstdint>
#include <Graphics/Types.h>
#include <shared/MetalOS.Keys.h>
#include <shared/MetalOS.Types.h>
#include <shared/MetalOS.System.h>

struct ProcessInfo
{
	uint32_t Id;
};

enum class SystemArchitecture : uint32_t
{
	Unknown = 0,
	x64 = 1
};

struct SystemInfo
{
	uint32_t PageSize;
	SystemArchitecture Architecture;
	uint32_t NumberOfProcessors;
	uint32_t AllocationGranularity;
	uintptr_t MinimumApplicationAddress;
	uintptr_t MaximumApplicationAddress;
};

constexpr bool CanRead(GenericAccess access)
{
	return access == GenericAccess::Read || access == GenericAccess::ReadWrite;
}

constexpr bool CanWrite(GenericAccess access)
{
	return access == GenericAccess::Write || access == GenericAccess::ReadWrite;
}

enum class FilePointerMove
{
	Begin,
	Current,
	End
};

enum class SystemCallResult
{
	Success = 0,
	InvalidPointer,
	InvalidHandle,
	InvalidObject,
	BrokenPipe,
	Failed,
	NotImplemented
};

struct CreateProcessArgs
{
	HFile StdInput;
	HFile StdOutput;
	HFile StdError;
};

struct CreateProcessResult
{
	HProcess Process;
};

struct PipeInfo
{
	size_t BytesAvailable;
	bool IsBroken;
};

extern "C"
{
	//Info
	SystemCallResult GetSystemInfo(SystemInfo& info);
	size_t GetTickCount();
	SystemCallResult GetSystemTime(SystemTime& time);

	SystemCallResult GetProcessInfo(ProcessInfo& info);


	//Process/Thread
	SystemCallResult CreateProcess(const char* processName, const CreateProcessArgs* args, CreateProcessResult* result);
	uint32_t GetThreadId(HThread thread);
	HThread GetCurrentThread();
	uint64_t GetCurrentThreadId();
	HThread CreateThread(size_t stackSize, ThreadStart startAddress, void* arg);
	void Sleep(const uint32_t milliseconds);
	void SwitchToThread();
	SystemCallResult ExitProcess(const uint32_t exitCode);
	SystemCallResult ExitThread(const uint32_t exitCode);

	uint32_t GetLastError();
	void SetLastError(uint32_t errorCode);

	//Semaphores
	//Handle CreateSemaphore(size_t initial, size_t maximum, const char* name);
	//SYSTEMCALL ReleaseSemaphore(Handle hSemaphore, size_t releaseCount, size_t* previousCount);

	//Windows
	SystemCallResult AllocWindow(HWindow& handle, const Graphics::Rectangle& frame);
	//SystemCallResult GetWindowInfo(HWindow handle, Rectangle& frame, WindowStyle& style);
	SystemCallResult PaintWindow(HWindow handle, const ReadOnlyBuffer& buffer);
	SystemCallResult MoveWindow(HWindow handle, const Graphics::Rectangle& frame);
	SystemCallResult GetWindowRect(HWindow handle, Graphics::Rectangle& frame);
	SystemCallResult GetMessage(Message& message);
	SystemCallResult PeekMessage(Message& message);
	SystemCallResult GetScreenRect(Graphics::Rectangle& rect);

	//Files, Pipes
	HFile CreateFile(const char* path, const GenericAccess access);
	SystemCallResult CreatePipe(HFile& readHandle, HFile& writeHandle);
	SystemCallResult ReadFile(const HFile handle, void* buffer, const size_t bufferSize, size_t* bytesRead);
	SystemCallResult WriteFile(const HFile handle, const void* buffer, const size_t bufferSize, size_t* bytesWritten);
	SystemCallResult SetFilePointer(const HFile handle, const size_t position, const FilePointerMove moveType, size_t* newPosition);
	SystemCallResult CloseFile(const HFile handle);
	SystemCallResult MoveFile(const char* existingFileName, const char* newFileName);
	SystemCallResult DeleteFile(const char* fileName);
	SystemCallResult CreateDirectory(const char* path);
	SystemCallResult WaitForSingleObject(const Handle handle, const uint32_t milliseconds, WaitStatus& status);
	SystemCallResult GetPipeInfo(const HFile handle, PipeInfo& info);
	SystemCallResult CloseHandle(const Handle handle);

	SystemCallResult CreateEvent(HEvent& event, const bool manual, const bool initial);
	SystemCallResult SetEvent(const HEvent event);
	SystemCallResult ResetEvent(const HEvent event);

	void* VirtualAlloc(const void* address, const size_t size);
	HRingBuffer CreateRingBuffer(const char* name, const size_t indexSize, const size_t ringSize);
	HSharedMemory CreateSharedMemory(const char* name, const size_t size);
	void* MapObject(const void* address, Handle handle);
	void* MapSharedObject(const void* address, const char* name);

	SystemCallResult DebugPrint(const char* s);
	SystemCallResult DebugPrintBytes(const char* s, const size_t length);

	Handle LoadLibrary(const char* lpLibFileName);
	uintptr_t GetProcAddress(Handle hModule, const char* lpProcName);
}


enum class DllEntryReason
{
	ProcessAttach,
	ProcessDetach,
	ThreadAttach,
	ThreadDetach
};

//True - LoadLibrary returns handle
//False - DLL is unloaded
typedef size_t (*DllMainCall)(Handle hinstDLL, enum DllEntryReason fdwReason);
const char DllMainName[] = "DllMain";

struct RingBufferHeader
{
	size_t Capacity;
	size_t ReadIndex;
	size_t WriteIndex;
	Handle ReadWriteLock;
};

constexpr size_t UIChannelSize = 2 * PageSize;

struct Channel
{
	struct
	{
		RingBufferHeader* Header;
		char* Buffer;
	} Inbound;
	struct
	{
		RingBufferHeader* Header;
		char* Buffer;
	} Outbound;
};