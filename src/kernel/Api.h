#pragma once

#include "Lib/Time.h"
#include "kernel/Types.h"
#include "kernel/Objects/KPipe.h"

/*
 * Forward declarations.
 */
class KSignal;
class KDevice;
class UProcess;
class KProcess;
class UThread;
class UWindow;
class Message;

/*
 * Kernel API types.
 */
enum class VirtualAllocType
{
	KStack,
};

enum class AllocType
{
	Boot,
	Kernel,
	Acpi,
	Temp,
	Shared,
	Malloc,
	WM,
	User
};

struct KSystemTime
{
	uint16_t Year;
	uint16_t Month;
	uint16_t Day;
	uint16_t Hour;
	uint16_t Minute;
	uint16_t Second;

	//Fractional second in both milli and nano
	uint16_t Milliseconds;
	uint32_t Nanoseconds;
};

struct KThread;

/*
 * System.
 */
void KePauseSystem();
void KeResumeSystem();
milli_t KeGetTicks();
nano_t KeGetNanoseconds();
void KeGetSystemTime(KSystemTime& time);

/*
 * Physical memory.
 */
void KePhysicalInitialize();
paddr_t KePhysicalAlloc();
paddr_t KePhysicalAlloc(const size_t count);
void KePhysicalFree(const paddr_t address);
void KePhysicalFree(const paddr_t address, const size_t count);

/*
 * Virtual memory. All sizes are in bytes; each routine has an m_process form
 * and a form taking an explicit KProcess.
 */
//Allocate backed by fresh physical pages
void* KeVirtualAlloc(const size_t size);
void* KeVirtualAlloc(KProcess& process, const size_t size);
void* KeVirtualAlloc(const void* address, const size_t size);
void* KeVirtualAlloc(KProcess& process, const void* address, const size_t size);

//Free
bool KeVirtualFree(const void* const address);
bool KeVirtualFree(KProcess& process, const void* const address);

//Map caller-supplied physical addresses
bool KeVirtualMap(const void* const virtualAddr, const paddr_t physicalAddr, const size_t size);
bool KeVirtualMap(KProcess& process, const void* const virtualAddr, const paddr_t physicalAddr, const size_t size);
void* KeVirtualMap(const paddr_t* addresses, const size_t size);
void* KeVirtualMap(KProcess& process, const paddr_t* addresses, const size_t size);
void* KeVirtualMap(const paddr_t address, const size_t size);
void* KeVirtualMap(KProcess& process, const paddr_t address, const size_t size);

/*
 * Kernel heap.
 */
void* KeAlloc(size_t size, AllocType type);
void KeFree(void* ptr, AllocType type);

inline String KeCopy(const CString& source, const AllocType type)
{
	char* const buf = reinterpret_cast<char*>(KeAlloc(source.Length + 1, type));
	memcpy(buf, source.c_str(), source.Length);
	return { buf, source.Length };
}

inline void* malloc(size_t size) { return KeAlloc(size, AllocType::Malloc); }
inline void free(void* ptr) { KeFree(ptr, AllocType::Malloc); }

template <typename T, typename... Args>
T* KeAlloc(AllocType type, Args&&... args)
{
	void* const mem = KeAlloc(sizeof(T), type);
	Assert(mem);
	return mem ? new (mem) T(static_cast<Args&&>(args)...) : nullptr;
}

/*
 * Modules.
 */
const KModule* KeLoadLibrary(const CString& name);

/*
 * KThreads.
 */
KThread* KeCreateThread(const KThreadStart start, void* const arg, const CString& name);
void KeExitThread();
void KeExitThread(KThread& thread);
void KThreadInit();
KWaitResult KeWait(KSignal& obj, const milli_t timeout = TimeoutMax);
void KeSleepThread(const nano_t time);
void KeYield();

/*
 * User Threads/Process.
 */
UThread* KeCreateUThread(UProcess& process, const size_t stackSize, const UThreadStart entry = nullptr, void* const arg = nullptr);
uint32_t UThreadInit(void* const arg);
UProcess* KeCreateProcess(const CString& commandLine);
void KeTerminateProcess(UProcess& process, const uint32_t exitCode);

/*
 * Files.
 */
bool KeCreateFile(KFile& file, const char* const path, const KFileAccess access);
bool KeReadFile(KFile& file, void* buffer, const size_t bufferSize, size_t* bytesRead);
bool KeSetFilePosition(KFile& file, const size_t position);
void* KeLoadFile(const char* const path);

/*
 * Prints.
 */
void CPrintf(const bool enabled, const char* format, ...);
void Printf(const char* format, va_list args);
void Bugcheck(const char* file, const char* line, const char* format, va_list args);
void PrintStack();
void PrintStack(const Arch::Context* context, const KProcess& process);
void PrintBytes(const void* data, const size_t length);
#define printf Printf

/*
 * Devices.
 */
KDevice* KeGetDevice(const CString& path);

/*
* Windows.
*/
void KeWindowingInitialize();
void KeWindowingEnable();
UWindow* CreateWindow(UThread& owner);
void Delete(UWindow& window);
Graphics::Rectangle GetScreen();
void KePostMessage(Message& msg);

/*
 * Interrupts.
 */
void KeRegisterInterrupt(const Arch::InterruptVector interrupt, const ActionContext& context);

void* GetAcpiTable();
paddr_t ResolveImageVA(void* const address);


bool KeIsValid(const void* address);

static constexpr PageTablesOps PtOps =
{
	.Resolve = [](const paddr_t address) -> void*
	{
		return reinterpret_cast<void*>(KernelPhysicalStart + address);
	},
	.PhyAlloc = [](paddr_t &address) -> bool
	{
		address = KePhysicalAlloc();
		return true;
	}
};
using KernelPageTables = PageTabes<PtOps>;
