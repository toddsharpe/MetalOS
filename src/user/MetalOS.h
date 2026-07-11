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
	/*
	 * MetalOS Runtime.
	 */
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
	bool FreeLibrary(HModule hModule);
	uint32_t* GetErrno();
	
	/*
	 * MetalOS Window Manager.
	 */
	SyscallResult AllocWindow(HWindow* handle, const Graphics::Rectangle* frame);
	SyscallResult MoveWindow(HWindow handle, const Graphics::Rectangle* frame);
	SyscallResult GetWindowRect(HWindow handle, Graphics::Rectangle* frame);
	SyscallResult GetMessage(Message* message);
	SyscallResult PeekMessage(Message* message);
	SyscallResult GetScreenRect(Graphics::Rectangle* rect);
	void* GetWindowSurface(HWindow handle);

	/*
	 * MetalOS Networking.
	 */
	HSocket SocketCreate(int af, int type, int protocol);
	SyscallResult SocketBind(HSocket sock, const sockaddr_in* addr);
	SyscallResult SocketConnect(HSocket sock, const sockaddr_in* peer);
	SyscallResult SocketSendTo(HSocket sock, const void* buf, size_t len, const sockaddr_in* to);
	SyscallResult SocketSend(HSocket sock, const void* buf, size_t len);
	SyscallResult SocketRecvFrom(HSocket sock, void* buf, size_t maxLen, size_t* bytesRecv, sockaddr_in* from, milli_t timeoutMs);
	SyscallResult SocketRecv(HSocket sock, void* buf, size_t maxLen, size_t* bytesRecv, milli_t timeoutMs);
	SyscallResult SocketClose(HSocket sock);
	SyscallResult GetInterfaceIp(uint32_t index, in_addr* addr, in_addr* subnet);
	SyscallResult SetInterfaceIp(uint32_t index, const in_addr* addr, const in_addr* subnet);
	SyscallResult GetGateway(uint32_t index, in_addr* gateway);
	SyscallResult SetGateway(uint32_t index, const in_addr* gateway);

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
	bool IsProcessAlive(const uint32_t processId);
	SyscallResult GetProcesses(ProcessListEntry* buffer, const size_t maxCount, size_t* count);

	//0x300: Devices
	SyscallResult MapFramebuffer(GraphicsDevice* device);
	SyscallResult GetInterfaces(InterfaceInfo* buffer, const size_t maxCount, size_t* count);
	SyscallResult NetSend(const uint32_t ifIdx, const void* frame, size_t length);

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
	bool VirtualFree(const void* address);
	SyscallResult CreateSharedMemory(const size_t size, HSharedMem* handle, void** address);
	SyscallResult MapSharedMemory(const HSharedMem handle, void** address);
	SyscallResult DeleteSharedMemory(const HSharedMem handle);
	
	//0x700: Debug
	SyscallResult DebugPrint(const char* s);
	SyscallResult DebugPrintBytes(const void* s, const size_t length);
	SyscallResult DebugPrintStack();

	//0x900: IPC endpoints (name -> capability-token rendezvous)
	SyscallResult RegisterEndpoint(const char* name, uint64_t id);
	SyscallResult LookupEndpoint(const char* name, uint64_t* id);
	SyscallResult PostEndpoint(const char* name, uint64_t value);
	SyscallResult PollEndpoint(const char* name, uint64_t* value);

	//0xA00: Handle capabilities. ShareHandle mints a single-use token for a handle
	//you own (currently shared memory); another process redeems it with ClaimHandle
	//to get its own handle to the same object. The token travels over the endpoint/
	//ring. ClaimHandle only installs the handle; open/map it with the type's own call
	//(e.g. MapSharedMemory for shared memory).
	SyscallResult ShareHandle(Handle handle, uint64_t* token);
	SyscallResult ClaimHandle(uint64_t token, Handle* handle);
}
