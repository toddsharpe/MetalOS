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
bool KePhysicalTryAlloc(paddr_t& address); //False when out of memory, for allocations a process can trigger
bool KePhysicalTryAlloc(paddr_t& address, const size_t count);
void KePhysicalRef(const paddr_t address); //Another owner (an owning mapping) of an allocated page
void KePhysicalFree(const paddr_t address); //Drops one reference; frees the page with the last
void KePhysicalFree(const paddr_t address, const size_t count);

/*
 * Virtual memory. All sizes are in bytes; forms without a KProcess act on m_process.
 */

//Allocate backed by fresh physical pages
void* KeVirtualAlloc(const size_t size);
void* KeVirtualAlloc(KProcess& process, const size_t size);
void* KeVirtualAlloc(KProcess& process, const void* address, const size_t size);

//Free
bool KeVirtualFree(const void* const address);
bool KeVirtualFree(KProcess& process, const void* const address);

//Map caller-supplied physical addresses
bool KeVirtualMap(KProcess& process, const void* const virtualAddr, const paddr_t physicalAddr, const size_t size);
void* KeVirtualMap(const paddr_t* addresses, const size_t size);
void* KeVirtualMap(KProcess& process, const paddr_t* addresses, const size_t size);
void* KeVirtualMap(const paddr_t address, const size_t size);
void* KeVirtualMap(KProcess& process, const paddr_t address, const size_t size);

void KeWriteProcessMemory(KProcess& process, void* const destVirtual, const void* const src, const size_t size);
void KeAliasInto(KProcess& process, const void* const source, const void* const dest, const size_t size);
void KeFreePageTables(KProcess& process);

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
		return KePhysicalTryAlloc(address);
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
//A kernel-held region (e.g. a ring), initialized and then published as a grant under an endpoint for a server to claim
void* KePublishRegion(const char* const endpoint, const size_t size, void (*const initialize)(void* region));

/*
 * Handle grants (capability passing). Generic over handle types; the grant carries the
 * object's type + kernel pointer. Mint a single-use token, redeem it with KeGrantClaim.
 */
static constexpr uint32_t KernelGrantOwner = 0; //Process ids start at 1
uint64_t KeGrantShare(const UObjectType type, void* const object, const uint32_t granter);
bool KeGrantClaim(const uint64_t token, UObjectType& type, void*& object);
void KeGrantRevoke(const uint32_t granter);

/*
 * IPC endpoint registry: a name -> opaque value (e.g. a grant token) rendezvous so unrelated processes can find a server.
 */
static constexpr size_t MaxEndpointName = 32; //Including the NUL
static constexpr uint32_t KernelEndpointOwner = 0; //Process ids start at 1
bool KeEndpointRegister(const char* const name, const uint64_t id, const uint32_t owner);
bool KeEndpointLookup(const char* const name, uint64_t& id);
//Connection handoff: a client posts an id, the server polls to accept it.
bool KeEndpointPost(const char* const name, const uint64_t value);
bool KeEndpointPoll(const char* const name, const uint32_t owner, uint64_t& value);
void KeEndpointRelease(const uint32_t owner);

/*
 * Kernel heap.
 */
enum class AllocType
{
	Kernel,
	Acpi,
	Malloc,
	User
};

void* KeAlloc(size_t size, AllocType type);
void KeFree(void* ptr, AllocType type);

inline CString KeCopy(const CString& source, const AllocType type)
{
	char* const buf = reinterpret_cast<char*>(KeAlloc(source.Length + 1, type));
	memcpy(buf, source.c_str(), source.Length);
	buf[source.Length] = '\0';
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
void KeReleaseUThread(UThread& thread);
void KeReleaseUProcess(UProcess& process);
void KeSignalReaper();
uint32_t KeReaperThread(void* const arg);
bool KeIsProcessAlive(const uint32_t id);
size_t KeGetProcessList(ProcessListEntry* const buffer, const size_t maxCount);

/*
 * Files.
 */
bool KeCreateFile(KFile& file, const char* const path, const KFileAccess access);
bool KeReadFile(KFile& file, void* buffer, const size_t bufferSize, size_t* bytesRead);
bool KeSetFilePosition(KFile& file, const size_t position);
void* KeLoadFile(const char* const path); //Null if the file is missing or empty, or there's no memory for it
void KePipeUnref(KPipe& pipe, KRef& ref); //Drops one of the pipe's Readers/Writers/Waiters; the pipe is freed when all three are empty

/*
 * Prints.
 */
void VPrintf(const char* format, va_list args); //Not a Printf overload: va_list is char*, so it would capture Printf("%s", buffer)
void VBugcheck(const char* file, const char* line, const char* format, va_list args);
void TracePrint(KPipe* const output, const char* const format, ...); //Kernel console, and output too when it isn't null
void PrintStack();
void PrintStack(const Arch::Context* context, const KProcess& process, KPipe* const output = nullptr); //Prints like TracePrint
void PrintBytes(const void* data, const size_t length);
#define printf Printf

/*
 * Devices.
 */

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
