#pragma once

#include "Lib/Math.h"
#include "Lib/System.h"
#include "core_crt/stdint.h"
#include "x64/CpuId.h"

typedef uintptr_t paddr_t;
typedef uintptr_t cpu_flags_t;

namespace x64
{
	// Convenient defines for all targets
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
		// External Interrupts
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

		// IRQs
		Timer0 = 0x80,
		COM2 = 0x83,
		COM1 = 0x84,
		HypervisorVmBus = 0x90,
	};

	struct InterruptFrame
	{
		// Pushed by PUSH_INTERRUPT_FRAME
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

		uint64_t RBP; // Position between automatically pushed context and additional context

		// Intel SDM Vol3A Figure 6-4
		// Pushed conditionally by CPU, ensured to exist by x64_INTERRUPT_HANDLER, 0 by default
		uint64_t ErrorCode;

		// Pushed automatically
		uint64_t RIP;
		uint64_t CS;
		uint64_t RFlags;
		uint64_t RSP;
		uint64_t SS;
	};

	// Non-Volatile registers, with IP and flags
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

		// Args - TODO: find a way for the compiler to do this for us
		uint64_t Arg0;
		uint64_t Arg1;
		uint64_t Arg2;
		uint64_t Arg3;
	};
#pragma pack(pop)

#pragma pack(push, 1)
	// Intel SDM Vol3A Table 4-14
	typedef struct _PML4E
	{
		union
		{
			struct
			{
				uint64_t Present : 1;		   // Must be 1, region invalid if 0.
				uint64_t ReadWrite : 1;		   // If 0, writes not allowed.
				uint64_t UserSupervisor : 1;   // If 0, user-mode accesses not allowed.
				uint64_t PageWriteThrough : 1; // Determines the memory type used to access PDPT.
				uint64_t PageCacheDisable : 1; // Determines the memory type used to access PDPT.
				uint64_t Accessed : 1;		   // If 0, this entry has not been used for translation.
				uint64_t Ignored1 : 1;
				uint64_t PageSize : 1; // Reserved, must be 0 for PML4E.
				uint64_t Ignored2 : 4;
				uint64_t PageFrameNumber : 36; // The page frame number of the PDPT of this PML4E.
				uint64_t Reserved : 4;
				uint64_t Ignored3 : 11;
				uint64_t ExecuteDisable : 1; // If 1, instruction fetches not allowed.
			};
			uint64_t Value;
		};
	} PML4E, *PPML4E;
	static_assert(sizeof(_PML4E) == sizeof(uintptr_t), "Size mismatch, only 64-bit supported.");

	// Intel SDM Vol3A Table 4-15
	typedef struct _PDPTE_PAGE // Maps 1GB Page
	{
		union
		{
			struct
			{
				uint64_t Present : 1;		   // Must be 1, region invalid if 0.
				uint64_t ReadWrite : 1;		   // If 0, writes not allowed.
				uint64_t UserSupervisor : 1;   // If 0, user-mode accesses not allowed.
				uint64_t PageWriteThrough : 1; // Determines the memory type used to access PD.
				uint64_t PageCacheDisable : 1; // Determines the memory type used to access PD.
				uint64_t Accessed : 1;		   // If 0, this entry has not been used for translation.
				uint64_t Dirty : 1;			   // Ignored if PageSize=0, Dirty if 1GB page
				uint64_t PageSize : 1;		   // If 1, this entry maps a 1GB page.
				uint64_t Global : 1;		   // Ignored if PageSize=1, Global if 1GB page
				uint64_t Ignored1 : 3;		   //
				uint64_t PageAccessType : 1;   //
				uint64_t Reserved1 : 17;	   // Reserved (must be 0).
				uint64_t PageFrameNumber : 18; //
				uint64_t Reserved2 : 4;
				uint64_t Ignored2 : 7;
				uint64_t Ignored3 : 4;
				uint64_t ExecuteDisable : 1; // If 1, instruction fetches not allowed.
			};
			uint64_t Value;
		};
	} PDPTE_PAGE, *PPDPTE_PAGE;
	static_assert(sizeof(_PDPTE_PAGE) == sizeof(uintptr_t), "Incorrect.");

	// Intel SDM Vol3A Table 4-16
	typedef struct _PDPTE_DIR
	{
		union
		{
			struct
			{
				uint64_t Present : 1;		   // Must be 1, region invalid if 0.
				uint64_t ReadWrite : 1;		   // If 0, writes not allowed.
				uint64_t UserSupervisor : 1;   // If 0, user-mode accesses not allowed.
				uint64_t PageWriteThrough : 1; // Determines the memory type used to access PD.
				uint64_t PageCacheDisable : 1; // Determines the memory type used to access PD.
				uint64_t Accessed : 1;		   // If 0, this entry has not been used for translation.
				uint64_t Ignored1 : 1;
				uint64_t PageSize : 1; // If 1, this entry maps a 1GB page.
				uint64_t Ignored2 : 4;
				uint64_t PageFrameNumber : 36; // The page frame number of the PD of this PDPTE.
				uint64_t Reserved : 4;
				uint64_t Ignored3 : 11;
				uint64_t ExecuteDisable : 1; // If 1, instruction fetches not allowed.
			};
			uint64_t Value;
		};
	} PDPTE_DIR, *PPDPTE_DIR;
	static_assert(sizeof(_PDPTE_DIR) == sizeof(uintptr_t), "Incorrect.");

	// Intel SDM Vol3A Table 4-17
	typedef struct _PDE_PAGE // Maps 2MB page
	{
		union
		{
			struct
			{
				uint64_t Present : 1;		   // Must be 1, region invalid if 0.
				uint64_t ReadWrite : 1;		   // If 0, writes not allowed.
				uint64_t UserSupervisor : 1;   // If 0, user-mode accesses not allowed.
				uint64_t PageWriteThrough : 1; // Determines the memory type used to access PT.
				uint64_t PageCacheDisable : 1; // Determines the memory type used to access PT.
				uint64_t Accessed : 1;		   // If 0, this entry has not been used for translation.
				uint64_t Dirty : 1;
				uint64_t PageSize : 1; // If 1, this entry maps a 2MB page.
				uint64_t Global : 1;
				uint64_t Ignored1 : 3;
				uint64_t PageAccessType : 1; //
				uint64_t Reserved1 : 8;
				uint64_t PageFrameNumber : 27; // The page frame number of the PT of this PDE.
				uint64_t Reserved2 : 4;
				uint64_t Ignored2 : 7;
				uint64_t Ignored3 : 4;
				uint64_t ExecuteDisable : 1; // If 1, instruction fetches not allowed.
			};
			uint64_t Value;
		};
	} PDE_PAGE, *PPDE_PAGE;
	static_assert(sizeof(_PDE_PAGE) == sizeof(uintptr_t), "Incorrect.");

	// Intel SDM Vol3A Table 4-18
	typedef struct _PDE_DIR
	{
		union
		{
			struct
			{
				uint64_t Present : 1;		   // Must be 1, region invalid if 0.
				uint64_t ReadWrite : 1;		   // If 0, writes not allowed.
				uint64_t UserSupervisor : 1;   // If 0, user-mode accesses not allowed.
				uint64_t PageWriteThrough : 1; // Determines the memory type used to access PT.
				uint64_t PageCacheDisable : 1; // Determines the memory type used to access PT.
				uint64_t Accessed : 1;		   // If 0, this entry has not been used for translation.
				uint64_t Ignored1 : 1;
				uint64_t PageSize : 1; // If 1, this entry maps a 2MB page.
				uint64_t Ignored2 : 4;
				uint64_t PageFrameNumber : 36; // The page frame number of the PT of this PDE.
				uint64_t Reserved : 4;
				uint64_t Ignored3 : 11;
				uint64_t ExecuteDisable : 1; // If 1, instruction fetches not allowed.
			};
			uint64_t Value;
		};
	} PDE_DIR, *PPDE_DIR;
	static_assert(sizeof(_PDE_DIR) == sizeof(uintptr_t), "Size mismatch, only 64-bit supported.");

	// Intel SDM Vol3A Table 4-19
	typedef struct _PTE // Maps a 4KB page
	{
		union
		{
			struct
			{
				uint64_t Present : 1;		   // Must be 1, region invalid if 0.
				uint64_t ReadWrite : 1;		   // If 0, writes not allowed.
				uint64_t UserSupervisor : 1;   // If 0, user-mode accesses not allowed.
				uint64_t PageWriteThrough : 1; // Determines the memory type used to access the memory.
				uint64_t PageCacheDisable : 1; // Determines the memory type used to access the memory.
				uint64_t Accessed : 1;		   // If 0, this entry has not been used for translation.
				uint64_t Dirty : 1;			   // If 0, the memory backing this page has not been written to.
				uint64_t PageAccessType : 1;   // Determines the memory type used to access the memory.
				uint64_t Global : 1;		   // If 1 and the PGE bit of CR4 is set, translations are global.
				uint64_t OSCopyOnWrite : 1;
				uint64_t OSPrototypePTE : 1;
				uint64_t OSWrite : 1;
				uint64_t PageFrameNumber : 36; // The page frame number of the backing physical page.
				uint64_t Reserved : 4;
				uint64_t Ignored3 : 7;
				uint64_t ProtectionKey : 4;	 // If the PKE bit of CR4 is set, determines the protection key.
				uint64_t ExecuteDisable : 1; // If 1, instruction fetches not allowed.
			};
			uint64_t Value;
		};
	} PTE, *PPTE;
	static_assert(sizeof(_PTE) == sizeof(uintptr_t), "Incorrect.");

	struct VirtualAddress
	{
		union
		{
			struct
			{
				uint64_t offset : 12;
				uint64_t index1 : 9;
				uint64_t index2 : 9;
				uint64_t index3 : 9;
				uint64_t index4 : 9;
				uint64_t upper : 16;
			};
			uint64_t AsUint64;
		};
	};
	static_assert(sizeof(VirtualAddress) == sizeof(uint64_t), "Size mismatch");
#pragma pack(pop)
} // namespace x64

// TODO(tsharpe): replace with constexpr?
#define QWordHigh(x) (((uint64_t)x) >> 32)
#define QWordLow(x) ((uint32_t)((uint64_t)x))
