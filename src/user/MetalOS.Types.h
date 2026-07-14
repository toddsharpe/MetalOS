#pragma once

//Forward declare kernel types

#include "Lib/List.h"
#include "Lib/StaticVector.h"
#include "Lib/String.h"

#define DECLARE_HANDLE(name) struct name##__; typedef struct name##__ *name

//User handles
typedef void* Handle;
DECLARE_HANDLE(HFile);
DECLARE_HANDLE(HThread);
DECLARE_HANDLE(HWindow);
DECLARE_HANDLE(HProcess);
DECLARE_HANDLE(HEvent);
DECLARE_HANDLE(HModule);
DECLARE_HANDLE(HSharedMem);

struct ProcessInfo
{
	uint32_t Id;
};

constexpr size_t MaxProcessName = 32;

//Process lifecycle state, shared with the kernel (UProcess stores this directly).
enum class ProcessState : uint32_t
{
	Created,
	Running,
	Terminated,
};

constexpr const char* StateName(const ProcessState state)
{
	switch (state)
	{
		case ProcessState::Created: return "Created";
		case ProcessState::Running: return "Running";
		case ProcessState::Terminated: return "Terminated";
		default: return "?";
	}
}

struct ProcessListEntry
{
	uint32_t Id;
	uint64_t VirtualSize;
	ProcessState State;
	char Name[MaxProcessName];
};

constexpr size_t MaxModuleName = 16;
struct Module
{
	void* ImageBase;
	void* PDB;
	char Name[MaxModuleName];
};

constexpr size_t MaxLoadedModules = 8;
struct ProcessEnvironmentBlock
{
	void *ImageBase;
	CString CommandLine;
	HProcess Handle;
	uint32_t ProcessId;
	Module LoadedModules[MaxLoadedModules];
	size_t ModuleIndex;
	bool Debug;
};

typedef uint32_t (*ThreadStart)(void *arg);
struct ThreadEnvironmentBlock
{
	ThreadEnvironmentBlock *SelfPointer;
	ProcessEnvironmentBlock *PEB;
	ThreadStart ThreadStart;
	void *Arg;
	HThread Handle;
	uint32_t ThreadId;
	uint32_t Error;
};

enum class DllReason
{
	ProcessAttach,
	ProcessDetach,
	ThreadAttach,
	ThreadDetach
};

typedef bool (*DllMainCall)(HModule hinstDLL, DllReason reason);

enum class DayOfWeek
{
	Sunday,
	Monday,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday
};

struct SystemTime
{
	uint16_t Year;
	uint16_t Month;
	DayOfWeek DayOfTheWeek;
	uint16_t Day;
	uint16_t Hour;
	uint16_t Minute;
	uint16_t Second;
	uint16_t Milliseconds;
};

//STARTUPINFOA
struct CreateProcessArgs
{
	HFile StdInput;
	HFile StdOutput;
	HFile StdError;
};

//PROCESS_INFORMATION
struct CreateProcessResult
{
	HProcess Process;
};

//Framebuffer descriptor returned by MapFramebuffer. FrameBase is mapped into the
//caller's address space; PixelsPerScanLine is the row stride in pixels (>= Width).
struct GraphicsDevice
{
	void*    FrameBase;
	uint32_t Width;
	uint32_t Height;
	uint32_t PixelsPerScanLine;
};

enum class FileAccess
{
	Read = (1 << 0),
	Write = (1 << 1),
	ReadWrite = Read | Write
};

enum class Seek
{
	Begin,
	Current,
	End
};

enum class WaitStatus
{
	None,
	Signaled,
	Timeout,
};

typedef uint64_t milli_t;//Time in milliseconds

typedef void* HSocket;
static constexpr int AF_INET      = 2;
static constexpr int SOCK_DGRAM   = 2;
static constexpr int SOCK_RAW     = 3;
static constexpr int IPPROTO_ICMP = 1;
static constexpr int IPPROTO_UDP  = 17;
static constexpr HSocket INVALID_SOCKET = nullptr;

struct in_addr { uint32_t s_addr; };
struct sockaddr_in
{
	uint16_t sin_family;
	uint16_t sin_port;   //network byte order
	in_addr  sin_addr;   //network byte order
	uint8_t  sin_zero[8];
};



//Network interface descriptor (addr/subnet are network byte order).
struct InterfaceInfo
{
	uint32_t index;
	uint8_t  mac[6];
};
