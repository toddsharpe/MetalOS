#pragma once

#include <MetalOS.h>
#include <cstdint>
#include <time.h>
#include "msvc.h"
#include <string>
#include <list>
#include <vector>

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
#pragma pack(pop)

#define UINT64_MAX 0xFFFFFFFFFFFFFFFF

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

#define KERNEL_STACK_SIZE (1 << 20)
#define BOOT_HEAP_SIZE EFI_PAGE_SIZE //4KB Boot heap

#define IDT_COUNT 256
#define IST_STACK_SIZE (1 << 12)
#define IST_DOUBLEFAULT_IDX 1
#define IST_NMI_IDX 2
#define IST_DEBUG_IDX 3
#define IST_MCE_IDX 4

#define QWordHigh(x) (((uint64_t)x) >> 32)
#define QWordLow(x) ((uint32_t)((uint64_t)x))

#define PLACEHOLDER 0

#define KERNEL_GLOBAL_ALIGN __declspec(align(64))
#define KERNEL_PAGE_ALIGN __declspec(align(PAGE_SIZE))

//Forward declarations of structs
struct _KERNEL_PROCESS;

enum MemoryAllocationType
{
	MemCommit = 1 << 0,
	MemReserve = 1 << 1,
	MemReset = 1 << 2,
	MemResetUndo = 1 << 3
};

// https://docs.microsoft.com/en-us/windows/win32/memory/memory-protection-constants
//enum MemoryProtection
//{
//	PageNoAccess,
//	PageReadOnly,
//	PageReadWrite,
//	PageWriteCopy,
//	PageExecute,
//	PageExecuteRead,
//	PageExecuteReadWrite,
//	PageExecuteWriteCopy,
//	PageGuard
//};

typedef size_t cpu_flags_t;
typedef uint64_t nano_t;//Time in nanoseconds
typedef uint64_t nano100_t;//Time in 100 nanoseconds


struct MemoryAllocation
{
	uint8_t Commit : 1;
	uint8_t Reserve : 1;
};

struct MemoryProtection
{
	uint8_t Read : 1;
	uint8_t Write : 1;
	uint8_t Execute : 1;
	//uint8_t CopyOnWrite : 1;
	//uint8_t Guard : 1;

	MemoryProtection(bool read = false, bool write = false, bool execute = false)
	{
		Read = read;
		Write = write;
		Execute = execute;
	}

	static const uint8_t NoAccess = 0;
};

//Virtual Address Descriptor
//Virtual Page Number
typedef struct _VAD_NODE
{
	uint64_t StartPN;
	uint64_t EndPN;

	struct
	{
		MemoryAllocation Allocation;
		MemoryProtection Protection;
	} Flags;

	_KERNEL_PROCESS* Process;
	//commit - mapped, private
	//protection - readwrite, readonly, writecopy, execute_writecopy
} VAD_NODE, *PVAD_NODE;

class VirtualAddressSpace;

typedef struct __declspec(align(16)) {
	uint64_t Low;
	uint64_t High;
} M128A, * PM128A;

//TODO:
typedef struct __declspec(align(16))
{
	uint16_t ControlWord;
	uint16_t StatusWord;
	uint8_t TagWord;
	uint8_t Reserved1;
	uint16_t Opcode;
} XSAVE_FORMAT, *PXSAVE_FORMAT;

typedef struct _CONTEXT
{
	//Segment Registers and Flags
	uint16_t SegCs;
	uint16_t SegDs;
	uint16_t SegEs;
	uint16_t SegFs;
	uint16_t SegGs;
	uint16_t SegSs;
	uint64_t RFlags;

	//Debug registers TODO
	
	//Integer registers
	uint64_t Rax;
	uint64_t RcX;
	uint64_t RdX;
	uint64_t RbX;
	uint64_t Rsp;
	uint64_t Rbp;
	uint64_t Rsi;
	uint64_t Rdi;
	uint64_t R8;
	uint64_t R9;
	uint64_t R10;
	uint64_t R11;
	uint64_t R12;
	uint64_t R13;
	uint64_t R14;
	uint64_t R15;

	//Program counter
	uint64_t Rip;

	//Floating point state
	//FXSAVE vs XSAVE?
	union
	{
		XSAVE_FORMAT FltSave;
		struct
		{
			M128A Header[2];
			M128A Legacy[8];
			M128A Xmm0;
			M128A Xmm1;
			M128A Xmm2;
			M128A Xmm3;
			M128A Xmm4;
			M128A Xmm5;
			M128A Xmm6;
			M128A Xmm7;
			M128A Xmm8;
			M128A Xmm9;
			M128A Xmm10;
			M128A Xmm11;
			M128A Xmm12;
			M128A Xmm13;
			M128A Xmm14;
			M128A Xmm15;
		};
	};

	//Vector registers
	M128A VectorRegister[26];
	uint64_t VectorControl;

	//TODO: debug control registers

} CONTEXT, *PCONTEXT;

template <class T>
class Node
{
	T Data;
	Node* Next;
	Node* Previous;
};

typedef enum
{
	Ready,
	Running,
	Waiting,
	Terminated,
	Initialized
} ThreadState;

struct ThreadEnvironmentBlock
{
	ThreadEnvironmentBlock* SelfPointer;
	uint32_t ThreadId;
};

//Structure just for threads in the kernel
struct KernelThread
{
	uint32_t Id;
	ThreadState State;
	ThreadStart Start;
	void* Arg;
	//TODO: x64_CONTEXT_SIZE is const, pipe constant from masm to c
	void* Context;//Pointer to x64 CONTEXT structure (masm)
	ThreadEnvironmentBlock* TEB;
	uint64_t Scheduler;
	nano100_t SleepWake; //100ns
};

#define KERNEL_THREAD_STACK_SIZE 8

//Handle could be smarter to have upper bits to specify type
typedef struct _KERNEL_PROCESS
{
	uint32_t Id;
	std::string Name;
	time_t CreateTime;
	time_t ExitTime;

	//TODO: something holding CR3
	uint64_t CR3;

	VirtualAddressSpace* VirtualAddress;
} KERNEL_PROCESS, * PKERNEL_PROCESS;

typedef uintptr_t paddr_t;

enum InterruptVector : uint8_t
{
	//Externel Interrupts
	DivideError = 0,
	DebugException = 1,
	NMIInterrupt = 2,
	Breakpoint = 3,
	Overflow = 4,
	BoundRangeExceeded = 5,
	InvalidOpcode = 6,
	DeviceNotAvailable = 7,
	DoubleFault = 8,
	CoprocessorSegmentOverrun = 9,
	InvalidTSS = 10,
	SegmentNotPresent = 11,
	StackSegmentFault = 12,
	GeneralProtectionFault = 13,
	PageFault = 14,
	Reserved = 15,
	FPUMathFault = 16,
	AlignmentCheck = 17,
	MachineCheck = 18,
	SIMDException = 19,
	VirtualizationException = 20,
	Last = 32,

	//IRQs
	Timer0 = 0x80,

};

enum InterruptSubsystemType
{
	Native, //Platform interrupts (intel x64, etc)
	Irq
};

#define SECOND 1000000000 //1billion nano seconds
#define SECOND100NS (SECOND / 100) //# of 100ns segments in a second
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define HEAP_ALIGNMENT 16 //Bytes
#define HEAP_ALIGNMENT_MASK (HEAP_ALIGNMENT - 1)
#define HEAP_ALIGN(x) ((x + HEAP_ALIGNMENT_MASK) & ~(HEAP_ALIGNMENT_MASK))
