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

struct ProcessInfo
{
	uint32_t Id;
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

typedef size_t (*DllMainCall)(HModule hinstDLL);

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

//Trailing args for SocketRecvFrom, packed by the userland wrapper because the
//syscall ABI only carries 4 register args (see SocketRecvFromSys).
struct SocketRecvParams
{
	void*        buf;
	size_t       maxLen;
	size_t*      bytesRecv;
	sockaddr_in* from;
	milli_t      timeoutMs;
};

//Network interface descriptor (addr/subnet are network byte order).
struct InterfaceInfo
{
	uint32_t index;
	uint8_t  mac[6];
	uint16_t reserved;
	in_addr  addr;
	in_addr  subnet;
	in_addr  gateway;
};
