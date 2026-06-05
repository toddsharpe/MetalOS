#pragma once

#include "Lib/Time.h"
#include "kernel/Types.h"
#include "kernel/Objects/KPipe.h"

/*
 * Forward declarations.
 */
class KSignalObject;
class KDevice;
class UProcess;
class KProcess;
class UThread;

/*
 * Kernel API types.
 */
enum class VirtualAllocType
{
	KStack,
};

enum class HeapAllocType
{
	Acpi,
};

enum class MapType
{
	Acpi,
	Driver
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
 * Physical memory.
 */
paddr_t KePhysicalAlloc();
paddr_t KePhysicalAlloc(const size_t count);
void KePhysicalFree(const paddr_t address, const size_t count);

/*
 * Virtual memory.
 */
void* MapPages(const paddr_t address, const size_t count, const MapType type);
void* KeVirtualMap(const paddr_t* addresses, const size_t count);
void* KeVirtualMap(KProcess& process, const paddr_t* addresses, const size_t count);
void* KeVirtualAlloc(const size_t size);
void* KeVirtualAlloc(const void* address, const size_t size);
void* KeVirtualAlloc(KProcess& process, const void* address, const size_t size);

/*
 * Heap. Replace with arena.
 */
void* KeHeapAlloc(const size_t size, const HeapAllocType type);
void KeHeapFree(void* ptr, const HeapAllocType type);

/*
 * Time.
 */
milli_t KeGetTicks();
nano_t KeGetNanoseconds();
void KeGetSystemTime(KSystemTime& time);

/*
 * Modules.
 */
KModule* KeLoadLibrary(const CString& name);

/*
 * KThreads.
 */
KThread* KeCreateThread(const KThreadStart start, void* const arg, const CString& name);
void KeExitThread();
void KeExitThread(KThread& thread);
void KThreadInit();
KWaitResult KeWait(KSignalObject& obj, const milli_t timeout = TimeoutMax);
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
bool KeCreateFile(KFile& file, const CString& path, const KFileAccess access);
bool KeReadFile(KFile& file, void* buffer, const size_t bufferSize, size_t* bytesRead);
bool KeSetFilePosition(KFile& file, const size_t position);
void* KeLoadFile(const CString& path);

/*
 * Prints.
 */
void CPrintf(const bool enabled, const char* format, ...);
void Printf(const char* format, va_list args);
void Bugcheck(const char* file, const char* line, const char* format, va_list args);
void PrintStack();
void PrintStack(const Context* context, const KProcess& process);
void PrintBytes(const void* data, const size_t length);
#define printf Printf

/*
 * Devices.
 */
KDevice* KeGetDevice(const CString& path);

/*
* Windows.
*/
UWindow* CreateWindow(UThread& owner);
void Delete(UWindow& window);
Graphics::Rectangle GetScreenRect2();
void KePostMessage(Message& msg);

/*
 * Pipes.
 */
KPipe* KeCreatePipe(const size_t size);

/*
 * Interrupts.
 */
void KeRegisterInterrupt(const InterruptVector interrupt, const ActionContext& context);

void* GetAcpiTable();
paddr_t ResolveImageVA(void* const address);

void KePauseSystem();
void KeResumeSystem();
bool KeIsValid(const void* address);
