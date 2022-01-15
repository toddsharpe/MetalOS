#pragma once

#include "msvc.h"
#include <MetalOS.h>
#include <cstdint>
#include <time.h>
#include <string>
#include <list>
#include <vector>
#include "MetalOS.Internal.h"

#define NO_COPY_OR_ASSIGN(X) X(const X&) = delete; X& operator = (const X&) = delete;
#define MakePtr( cast, ptr, addValue ) (cast)( (uintptr_t)(ptr) + (uintptr_t)(addValue))

//Paging Structures - https://gist.github.com/mvankuipers/
//https://queazan.wordpress.com/2013/12/21/paging-under-amd64/
//https://software.intel.com/sites/default/files/managed/39/c5/325462-sdm-vol-1-2abcd-3abcd.pdf

#pragma pack(push, 1)
typedef struct
{
	//Pushed by PUSH_INTERRUPT_FRAME
	//Order: http://www.c-jump.com/CIS77/CPU/x86/X77_0060_mod_reg_r_m_byte.htm
	uint64_t RAX;
	uint64_t RCX;
	uint64_t RDX;
	uint64_t RBX;
	uint64_t RSI;
	uint64_t RDI;
	uint64_t R8;
	uint64_t R9;
	uint64_t R10;
	uint64_t R11;
	uint64_t R12;
	uint64_t R13;
	uint64_t R14;
	uint64_t R15;

	uint64_t FS;
	uint64_t GS;

	uint64_t RBP; //Position between automatically pushed context and additional context

	//Intel SDM Vol3A Figure 6-4
	//Pushed conditionally by CPU, ensured to exist by x64_INTERRUPT_HANDLER, 0 by default
	uint64_t ErrorCode;

	//Pushed automatically
	uint64_t RIP;
	uint64_t CS;
	uint64_t RFlags;
	uint64_t RSP;
	uint64_t SS;
} INTERRUPT_FRAME, * PINTERRUPT_FRAME;

struct SystemCallFrame
{
	SystemCall SystemCall;
	uint64_t UserIP;
	uint64_t RFlags;

	//Args - TODO: find a way for the compiler to do this for us
	uint64_t Arg0;
	uint64_t Arg1;
	uint64_t Arg2;
	uint64_t Arg3;
};
#pragma pack(pop)

#ifndef UINT64_MAX
#define UINT64_MAX 0xFFFFFFFFFFFFFFFF
#endif

#define PAGE_SHIFT  12
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PAGE_MASK   0xFFF

#define SIZE_TO_PAGES(a)  \
    ( ((a) >> PAGE_SHIFT) + ((a) & PAGE_MASK ? 1 : 0) )

//4mb reserved space
#define BootloaderPagePoolCount 256
#define ReservedPageTablePages 512

//User space starts at   0x00000000 00000000
//User space stops at    0x00007FFF FFFFFFFF
//Kernel space starts at 0xFFFF8000 00000000
//Kernel space stops at  0xFFFFFFFF FFFFFFFF

#define KernelHeapSize 0x1000000

#define MemoryMapReservedSize PAGE_SIZE

#define KernelStop UINT64_MAX

#define ISR_HANDLER(x) x64_interrupt_handler_ ## x
#define DEF_ISR_HANDLER(x) void ISR_HANDLER(x) ## ()

#define QWordHigh(x) (((uint64_t)x) >> 32)
#define QWordLow(x) ((uint32_t)((uint64_t)x))

#define KERNEL_GLOBAL_ALIGN __declspec(align(64))
#define KERNEL_PAGE_ALIGN __declspec(align(PAGE_SIZE))


typedef uint64_t nano_t;//Time in nanoseconds
typedef uint64_t nano100_t;//Time in 100 nanoseconds

enum class ThreadState
{
	Ready,
	Running,
	Sleeping,
	Waiting,
	Suspended,
	MessageWait,
	Terminated,
	Initialized
};

typedef uintptr_t paddr_t;

enum class InterruptVector : uint8_t
{
	//External Interrupts
	DivideError = 0x0,
	DebugException = 0x1,
	NMIInterrupt = 0x2,
	Breakpoint = 0x3,
	Overflow = 0x4,
	BoundRangeExceeded = 0x5,
	InvalidOpcode = 0x6,
	DeviceNotAvailable = 0x7,
	DoubleFault = 0x8,
	CoprocessorSegmentOverrun = 0x9,
	InvalidTSS = 0xA,
	SegmentNotPresent = 0xB,
	StackSegmentFault = 0xC,
	GeneralProtectionFault = 0xD,
	PageFault = 0xE,
	Reserved = 0xF,
	FPUMathFault = 0x10,
	AlignmentCheck = 0x11,
	MachineCheck = 0x12,
	SIMDException = 0x13,
	VirtualizationException = 0x14,
	Last = 0x20,

	//IRQs
	Timer0 = 0x80,
	COM2 = 0x83,
	COM1 = 0x84,
	HypervisorVmBus = 0x90,

};

typedef void (*InterruptHandler)(void* arg);
struct InterruptContext
{
	InterruptHandler Handler;
	void* Context;
};

#define SECOND 1000000000 //1billion nano seconds
#define SECOND100NS (SECOND / 100) //# of 100ns segments in a second
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define HEAP_ALIGNMENT 16 //Bytes
#define HEAP_ALIGNMENT_MASK (HEAP_ALIGNMENT - 1)
#define HEAP_ALIGN(x) ((x + HEAP_ALIGNMENT_MASK) & ~(HEAP_ALIGNMENT_MASK))

constexpr size_t ByteAlign(const size_t size, const size_t alignment)
{
	const size_t mask = alignment - 1;
	return ((size + mask) & ~(mask));
}

#define BYTE_ALIGN(x, alignment) ((x + (alignment - 1)) & ~(alignment - 1))
#define PAGE_ALIGN(x) BYTE_ALIGN(x, PAGE_SIZE)

#ifndef DECLSPEC_ALIGN
#if (_MSC_VER >= 1300) && !defined(MIDL_PASS)
#define DECLSPEC_ALIGN(x)   __declspec(align(x))
#else
#define DECLSPEC_ALIGN(x)
#endif
#endif

#ifndef DECLSPEC_NOINITALL
#if (_MSC_VER >= 1915) && !defined(MIDL_PASS)
#define DECLSPEC_NOINITALL __declspec(no_init_all)
#else
#define DECLSPEC_NOINITALL
#endif
#endif

#define AlignSize(x,a) (((x) + ((a) - 1)) & ~((a)-1))

typedef void (*MemberFunction)(void* arg);
struct CallContext
{
	MemberFunction Handler;
	void* Context;
};

#define COUNT_OF( arr) (sizeof(arr)/sizeof(0[arr]))

//To avoid heap allocations, pointers point inside pdb
struct StackEntry
{
	char* Function;
	uint32_t Line;
};

struct FileHandle
{
	void* Context;
	size_t Position;
	size_t Length;
	GenericAccess Access;
};

//TODO: somehow export struct from asm
//This is just for thread init
struct x64_context
{
	uint64_t R12;
	uint64_t R13;
	uint64_t R14;
	uint64_t R15;
	uint64_t Rdi;
	uint64_t Rsi;
	uint64_t Rbx;
	uint64_t Rbp;
	uint64_t Rsp;
	uint64_t Rip;
	uint64_t Rflags;
};

class Pdb;
struct KeLibrary
{
	std::string Name;
	Handle Handle;
	Pdb* Pdb;
};

struct PdbFunctionLookup
{
	std::string Name;
	size_t LineNumber;
};
