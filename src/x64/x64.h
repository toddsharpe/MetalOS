#pragma once

#include "core_crt/stdint.h"
#include "x64/CpuId.h"
#include "Lib/Math.h"
#include "Lib/System.h"

typedef uintptr_t paddr_t;
typedef uintptr_t cpu_flags_t;

namespace x64
{
	//Convenient defines for all targets
	static constexpr size_t PageShift = 12;
	static constexpr size_t PageSize = (1 << PageShift);
	static constexpr size_t PageMask = (PageSize - 1);

	static constexpr uint16_t KernelDpl = 0;
	static constexpr uint16_t UserDpl = 3;

	inline constexpr size_t SizeToPages(const size_t bytes)
	{
		return DivRoundUp(bytes, PageSize);
	}

	constexpr size_t PageAlign(const size_t size)
	{
		return ByteAlign(size, PageSize);
	}

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

	struct InterruptFrame
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

	//Non-Volatile registers, with IP and flags
	struct Context
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

	#pragma pack(push, 1)
	struct SyscallFrame
	{
		uint64_t SystemCall;
		uint64_t UserIP;
		uint64_t RFlags;

		//Args - TODO: find a way for the compiler to do this for us
		uint64_t Arg0;
		uint64_t Arg1;
		uint64_t Arg2;
		uint64_t Arg3;
	};
	#pragma pack(pop)
}

//TODO(tsharpe): replace with constexpr?
#define QWordHigh(x) (((uint64_t)x) >> 32)
#define QWordLow(x) ((uint32_t)((uint64_t)x))
