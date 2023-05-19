#pragma once

#include <cstdint>
#include <time.h>
#include <string>
#include <kernel/MetalOS.Internal.h>

#define NO_COPY_OR_ASSIGN(X) X(const X&) = delete; X& operator = (const X&) = delete;

//X64 Architecture structs
#pragma pack(push, 1)
struct X64_CONTEXT
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

struct X64_INTERRUPT_FRAME
{
	//Pushed by PUSH_INTERRUPT_FRAME
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
};

struct X64_SYSCALL_FRAME
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

enum class X64_INTERRUPT_VECTOR : uint8_t
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

//Alignment macros
#define KERNEL_GLOBAL_ALIGN __declspec(align(64))
#define KERNEL_PAGE_ALIGN __declspec(align(PageSize))

//Time
typedef uint64_t milli_t;//Time in milliseconds
typedef uint64_t nano_t;//Time in nanoseconds
static constexpr nano_t Second = 1'000'000'000; //1billion nano seconds
constexpr nano_t ToNano(const milli_t time)
{
	return time * Second / 1000;
}
typedef uint64_t nano100_t;//Time in 100 nanoseconds
static constexpr nano100_t Second100Ns = Second / 100; //# of 100ns segments in a second

typedef uintptr_t paddr_t;

typedef void (*InterruptHandler)(void* arg);
struct InterruptContext
{
	InterruptHandler Handler;
	void* Context;
};

class Pdb;
struct KeLibrary
{
	std::string Name;
	void* ImageBase;
	Pdb* Pdb;
};

struct PdbFunctionLookup
{
	void* Base;
	uint32_t RVA;
	size_t LineNumber;
	std::string Name;
};
