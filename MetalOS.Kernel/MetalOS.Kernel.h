#pragma once

#include "msvc.h"
#include <MetalOS.h>
#include <cstdint>
#include <time.h>
#include <string>
#include <list>
#include <vector>
#include <WindowsTypes.h>
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

#define QWordHigh(x) (((uint64_t)x) >> 32)
#define QWordLow(x) ((uint32_t)((uint64_t)x))

#define KERNEL_GLOBAL_ALIGN __declspec(align(64))
#define KERNEL_PAGE_ALIGN __declspec(align(PAGE_SIZE))

typedef size_t cpu_flags_t;
typedef uint64_t nano_t;//Time in nanoseconds
typedef uint64_t nano100_t;//Time in 100 nanoseconds

enum class ThreadState
{
	Ready,
	Running,
	Sleeping,
	Waiting,
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

//
// Define 128-bit 16-byte aligned xmm register type.
//

typedef struct DECLSPEC_ALIGN(16) _M128A {
	ULONGLONG Low;
	LONGLONG High;
} M128A, * PM128A;

//
// Format of data for (F)XSAVE/(F)XRSTOR instruction
//

typedef struct DECLSPEC_ALIGN(16) _XSAVE_FORMAT {
	WORD   ControlWord;
	WORD   StatusWord;
	BYTE  TagWord;
	BYTE  Reserved1;
	WORD   ErrorOpcode;
	DWORD ErrorOffset;
	WORD   ErrorSelector;
	WORD   Reserved2;
	DWORD DataOffset;
	WORD   DataSelector;
	WORD   Reserved3;
	DWORD MxCsr;
	DWORD MxCsr_Mask;
	M128A FloatRegisters[8];

#if defined(_WIN64)

	M128A XmmRegisters[16];
	BYTE  Reserved4[96];

#else

	M128A XmmRegisters[8];
	BYTE  Reserved4[224];

#endif

} XSAVE_FORMAT, * PXSAVE_FORMAT;

// end_ntoshvp

typedef XSAVE_FORMAT XMM_SAVE_AREA32, * PXMM_SAVE_AREA32;

typedef struct DECLSPEC_ALIGN(16) DECLSPEC_NOINITALL _CONTEXT {

	//
	// Register parameter home addresses.
	//
	// N.B. These fields are for convience - they could be used to extend the
	//      context record in the future.
	//

	DWORD64 P1Home;
	DWORD64 P2Home;
	DWORD64 P3Home;
	DWORD64 P4Home;
	DWORD64 P5Home;
	DWORD64 P6Home;

	//
	// Control flags.
	//

	DWORD ContextFlags;
	DWORD MxCsr;

	//
	// Segment Registers and processor flags.
	//

	WORD   SegCs;
	WORD   SegDs;
	WORD   SegEs;
	WORD   SegFs;
	WORD   SegGs;
	WORD   SegSs;
	DWORD EFlags;

	//
	// Debug registers
	//

	DWORD64 Dr0;
	DWORD64 Dr1;
	DWORD64 Dr2;
	DWORD64 Dr3;
	DWORD64 Dr6;
	DWORD64 Dr7;

	//
	// Integer registers.
	//

	DWORD64 Rax;
	DWORD64 Rcx;
	DWORD64 Rdx;
	DWORD64 Rbx;
	DWORD64 Rsp;
	DWORD64 Rbp;
	DWORD64 Rsi;
	DWORD64 Rdi;
	DWORD64 R8;
	DWORD64 R9;
	DWORD64 R10;
	DWORD64 R11;
	DWORD64 R12;
	DWORD64 R13;
	DWORD64 R14;
	DWORD64 R15;

	//
	// Program counter.
	//

	DWORD64 Rip;

	//
	// Floating point state.
	//

	union {
		XMM_SAVE_AREA32 FltSave;
		struct {
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

	//
	// Vector registers.
	//

	M128A VectorRegister[26];
	DWORD64 VectorControl;

	//
	// Special debug control registers.
	//

	DWORD64 DebugControl;
	DWORD64 LastBranchToRip;
	DWORD64 LastBranchFromRip;
	DWORD64 LastExceptionToRip;
	DWORD64 LastExceptionFromRip;
} CONTEXT, * PCONTEXT;

typedef struct _KNONVOLATILE_CONTEXT_POINTERS {
	union {
		PM128A FloatingContext[16];
		struct {
			PM128A Xmm0;
			PM128A Xmm1;
			PM128A Xmm2;
			PM128A Xmm3;
			PM128A Xmm4;
			PM128A Xmm5;
			PM128A Xmm6;
			PM128A Xmm7;
			PM128A Xmm8;
			PM128A Xmm9;
			PM128A Xmm10;
			PM128A Xmm11;
			PM128A Xmm12;
			PM128A Xmm13;
			PM128A Xmm14;
			PM128A Xmm15;
		};
	};

	union {
		PDWORD64 IntegerContext[16];
		struct {
			PDWORD64 Rax;
			PDWORD64 Rcx;
			PDWORD64 Rdx;
			PDWORD64 Rbx;
			PDWORD64 Rsp;
			PDWORD64 Rbp;
			PDWORD64 Rsi;
			PDWORD64 Rdi;
			PDWORD64 R8;
			PDWORD64 R9;
			PDWORD64 R10;
			PDWORD64 R11;
			PDWORD64 R12;
			PDWORD64 R13;
			PDWORD64 R14;
			PDWORD64 R15;
		};
	};

} KNONVOLATILE_CONTEXT_POINTERS, * PKNONVOLATILE_CONTEXT_POINTERS;

enum UnwindHandlerType
{
	UNW_FLAG_NHANDLER = 0,
	UNW_FLAG_EHANDLER = 1,
	UNW_FLAG_UHANDLER = 2,
	UNW_FLAG_CHAININFO = 4
};

enum class Result
{
	Success,
	Failed,
	NotImplemented
};

#define AlignSize(x,a) (((x) + ((a) - 1)) & ~((a)-1))
struct kvec {
	void* iov_base; /* and that should *never* hold a userland pointer */
	size_t iov_len;
};

typedef void (*MemberFunction)(void* arg);
struct CallContext
{
	MemberFunction Handler;
	void* Context;
};

//TODO: delete data?
struct Buffer
{
	void* Data;
	size_t Length;
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
