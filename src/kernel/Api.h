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
class Message;
class KSharedMemory;
enum class UObjectType;

/*
 * System.
 */
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

void KeWriteProcessMemory(KProcess& process, void* const destVirtual, const void* const src, const size_t size);
void KeAliasInto(KProcess& process, const void* const source, const void* const dest, const size_t size);

//Validity check on an address.
bool KeIsValid(const void* address);

//Kernel page tables.
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

/*
 * Framebuffer. The loader hands the kernel a linear framebuffer; KeGetFramebuffer
 * exposes its physical descriptor so a process (the usermode WM) can map it.
 */
struct KFramebufferInfo
{
	paddr_t Base;
	size_t Size;
	uint32_t Width;
	uint32_t Height;
	uint32_t Pitch; //pixels per scan line (>= Width)
};
KFramebufferInfo KeGetFramebuffer();

/*
 * Shared memory. Regions are reached only through the handles/grants that reference
 * them (see ShareHandle/ClaimHandle); there is no global id lookup.
 */
KSharedMemory* KeShmCreate(const size_t size);
void* KeShmMap(KProcess& process, KSharedMemory& shm);
void KeShmUnref(KSharedMemory& shm);

/*
 * Handle grants (capability passing). Generic over handle types; the grant carries the
 * object's type + kernel pointer. Mint a single-use token, redeem it with KeGrantClaim.
 */
uint64_t KeGrantShare(const UObjectType type, void* const object);
bool KeGrantClaim(const uint64_t token, UObjectType& type, void*& object);

/*
 * IPC endpoint registry: a name -> shared-memory-id rendezvous so unrelated
 * processes can find a server's well-known shared region.
 */
bool KeEndpointRegister(const char* const name, const uint64_t id);
bool KeEndpointLookup(const char* const name, uint64_t& id);
//Connection handoff: a client posts an id, the server polls to accept it.
bool KeEndpointPost(const char* const name, const uint64_t value);
bool KeEndpointPoll(const char* const name, uint64_t& value);

/*
 * Kernel heap.
 */
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
const KModule* KeLoadLibrary(const char* const name);
//Resolve a virtual address back to its loaded image (used by stack traces).
paddr_t ResolveImageVA(void* const address);

/*
 * KThreads.
 */
struct KThread;

KThread* KeCreateThread(const KThreadStart start, void* const arg, const char* const name);
void KeExitThread();
void KeExitThread(KThread& thread);
void KThreadInit();
KWaitResult KeWait(KSignal& obj, const milli_t timeout = TimeoutMax);
void KeSleepThread(const nano_t time);
void KeYield();

/*
 * User Threads/Process.
 */
struct ProcessListEntry; //user/MetalOS.Types.h

UThread* KeCreateUThread(UProcess& process, const size_t stackSize, const UThreadStart entry = nullptr, void* const arg = nullptr);
uint32_t UThreadInit(void* const arg);
UProcess* KeCreateProcess(const char* const commandLine);
void KeTerminateProcess(UProcess& process, const uint32_t exitCode);
bool KeIsProcessAlive(const uint32_t id);
size_t KeGetProcessList(ProcessListEntry* const buffer, const size_t maxCount);

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
KDevice* KeGetDevice(const char* const path);

//Network device: the kernel owns NIC enumeration + raw ethernet frame I/O (RX via KeNetRxFrame
//from IRQ, published by KeInitNetDevice); the TCP/IP stack lives in the usermode netstack.
struct KMacAddress { uint8_t bytes[6]; };
class NicDriver;

void KeInitNetDevice();
void KeNetRxFrame(NicDriver* const nic, const void* const frame, const size_t length);
bool KeNetSend(const uint32_t ifIdx, const void* const frame, const size_t length);
size_t KeNetInterfaceCount();
NicDriver* KeNetGetInterface(const size_t index);

/*
 * Input. Drivers post Message records into a kernel->WM shared ring (published
 * under the "input" endpoint) that the usermode WM (MetalOS-WMSvr.exe) drains and routes.
 * Compositing and window management live entirely in the usermode WM process.
 */
void KeInputInitialize();
void KeInputPost(const Message& message);

/*
 * Interrupts.
 */
void KeRegisterInterrupt(const Arch::InterruptVector interrupt, const ActionContext& context);

/*
 * ACPI.
 */
void* GetAcpiTable();
