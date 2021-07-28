/*++ NDK Version: 0098

Copyright (c) Alex Ionescu.  All rights reserved.

Header Name:

	ketypes.h (AMD64)

Abstract:

	amd64 Type definitions for the Kernel services.

Author:

	Alex Ionescu (alexi@tinykrnl.org) - Updated - 27-Feb-2006
	Timo Kreuzer (timo.kreuzer@reactos.org) - Updated - 14-Aug-2008

--*/

#ifndef _AMD64_KETYPES_H
#define _AMD64_KETYPES_H

//
// Dependencies
//

//
// KPCR Access for non-IA64 builds
//
//#define K0IPCR                  ((ULONG_PTR)(KIP0PCRADDRESS))
//#define PCR                     ((volatile KPCR * const)K0IPCR)
#define PCR ((volatile KPCR * const)__readgsqword(FIELD_OFFSET(KPCR, Self)))
//#if defined(CONFIG_SMP) || defined(NT_BUILD)
//#undef  KeGetPcr
//#define KeGetPcr()              ((volatile KPCR * const)__readfsdword(0x1C))
//#endif

// KPROCESSOR_MODE Type
//
typedef CCHAR KPROCESSOR_MODE;

//
// CPU Vendors
//
typedef enum
{
	CPU_UNKNOWN,
	CPU_AMD,
	CPU_INTEL,
	CPU_VIA
} CPU_VENDORS;

//
// Machine Types
//
#define MACHINE_TYPE_ISA        0x0000
#define MACHINE_TYPE_EISA       0x0001
#define MACHINE_TYPE_MCA        0x0002

//
// X86 80386 Segment Types
//
#define I386_TASK_GATE          0x5
#define I386_TSS                0x9
#define I386_ACTIVE_TSS         0xB
#define I386_CALL_GATE          0xC
#define I386_INTERRUPT_GATE     0xE
#define I386_TRAP_GATE          0xF

//
// Selector Names
//
#define RPL_MASK                0x0003
#define MODE_MASK               0x0001
#define KGDT64_NULL             0x0000
#define KGDT64_R0_CODE          0x0010
#define KGDT64_R0_DATA          0x0018
#define KGDT64_R3_CMCODE        0x0020
#define KGDT64_R3_DATA          0x0028
#define KGDT64_R3_CODE          0x0030
#define KGDT64_SYS_TSS          0x0040
#define KGDT64_R3_CMTEB         0x0050
#define KGDT64_R0_LDT           0x0060

//
// CR4
//
#define CR4_VME                 0x1
#define CR4_PVI                 0x2
#define CR4_TSD                 0x4
#define CR4_DE                  0x8
#define CR4_PSE                 0x10
#define CR4_PAE                 0x20
#define CR4_MCE                 0x40
#define CR4_PGE                 0x80
#define CR4_FXSR                0x200
#define CR4_XMMEXCPT            0x400
#define CR4_CHANNELS            0x800
#define CR4_XSAVE               0x40000

//
// DR7
//
#define DR7_LEGAL               0xFFFF0355
#define DR7_ACTIVE              0x00000355
#define DR7_TRACE_BRANCH        0x00000200
#define DR7_LAST_BRANCH         0x00000100

//
// Debug flags
//
#define DEBUG_ACTIVE_DR7                        0x0001
#define DEBUG_ACTIVE_INSTRUMENTED               0x0002
#define DEBUG_ACTIVE_DBG_INSTRUMENTED           0x0003
#define DEBUG_ACTIVE_MINIMAL_THREAD             0x0004
#define DEBUG_ACTIVE_PRIMARY_THREAD             0x0080
#define DEBUG_ACTIVE_PRIMARY_THREAD_BIT         0x0007
#define DEBUG_ACTIVE_PRIMARY_THREAD_LOCK_BIT    0x001F
#define DEBUG_ACTIVE_SCHEDULED_THREAD           0x0040
#define DEBUG_ACTIVE_SCHEDULED_THREAD_BIT       0x0006
#define DEBUG_ACTIVE_SCHEDULED_THREAD_LOCK_BIT  0x001E
#define DEBUG_ACTIVE_SCHEDULED_THREAD_LOCK      0x40000000

//
// EFlags
//
#define EFLAGS_CF               0x01L
#define EFLAGS_ZF               0x40L
#define EFLAGS_TF               0x100L
#define EFLAGS_INTERRUPT_MASK   0x200L
#define EFLAGS_DF               0x400L
#define EFLAGS_IOPL             0x3000L
#define EFLAGS_NESTED_TASK      0x4000L
//#define EFLAGS_NF               0x4000
#define EFLAGS_RF               0x10000
#define EFLAGS_V86_MASK         0x20000
#define EFLAGS_ALIGN_CHECK      0x40000
#define EFLAGS_VIF              0x80000
#define EFLAGS_VIP              0x100000
#define EFLAGS_ID               0x200000
#define EFLAGS_USER_SANITIZE    0x3F4DD7
#define EFLAG_SIGN              0x8000
#define EFLAG_ZERO              0x4000
#define EFLAGS_TF_MASK          0x0100
#define EFLAGS_TF_SHIFT         0x0008
#define EFLAGS_ID_MASK          0x200000
#define EFLAGS_IF_MASK          0x0200
#define EFLAGS_IF_SHIFT         0x0009

//
// MXCSR Floating Control/Status Bit Masks
//
#define XSW_INVALID_OPERATION   0x0001
#define XSW_DENORMAL            0x0002
#define XSW_ZERO_DIVIDE         0x0004
#define XSW_OVERFLOW            0x0008
#define XSW_UNDERFLOW           0x0010
#define XSW_PRECISION           0x0020
#define XCW_INVALID_OPERATION   0x0080
#define XCW_DENORMAL            0x0100
#define XCW_ZERO_DIVIDE         0x0200
#define XCW_OVERFLOW            0x0400
#define XCW_UNDERFLOW           0x0800
#define XCW_PRECISION           0x1000
#define XCW_ROUND_CONTROL       0x6000
#define XCW_FLUSH_ZERO          0x8000
#define XSW_ERROR_MASK          0x003F
#define XSW_ERROR_SHIFT         7

//
// Legacy floating status word bit masks.
//
#define FSW_INVALID_OPERATION   0x0001
#define FSW_DENORMAL            0x0002
#define FSW_ZERO_DIVIDE         0x0004
#define FSW_OVERFLOW            0x0008
#define FSW_UNDERFLOW           0x0010
#define FSW_PRECISION           0x0020
#define FSW_STACK_FAULT         0x0040
#define FSW_ERROR_SUMMARY       0x0080
#define FSW_CONDITION_CODE_0    0x0100
#define FSW_CONDITION_CODE_1    0x0200
#define FSW_CONDITION_CODE_2    0x0400
#define FSW_CONDITION_CODE_3    0x4000
#define FSW_ERROR_MASK          0x003F

//
// Machine Specific Registers
//
#define MSR_EFER                0xC0000080
#define MSR_STAR                0xC0000081
#define MSR_LSTAR               0xC0000082
#define MSR_CSTAR               0xC0000083
#define MSR_SYSCALL_MASK        0xC0000084
#define MSR_FS_BASE             0xC0000100
#define MSR_GS_BASE             0xC0000101
#define MSR_GS_SWAP             0xC0000102
#define MSR_MCG_STATUS          0x017A
#define MSR_AMD_ACCESS          0x9C5A203A
#define MSR_IA32_MISC_ENABLE    0x01A0
#define MSR_LAST_BRANCH_FROM    0x01DB
#define MSR_LAST_BRANCH_TO      0x01DC
#define MSR_LAST_EXCEPTION_FROM 0x01DD
#define MSR_LAST_EXCEPTION_TO   0x01DE

//
// Caching values for the PAT MSR
//
#define PAT_UC                  0ULL
#define PAT_WC                  1ULL
#define PAT_WT                  4ULL
#define PAT_WP                  5ULL
#define PAT_WB                  6ULL
#define PAT_UCM                 7ULL

//
// Flags in MSR_EFER
//
#define MSR_SCE                 0x0001
#define MSR_LME                 0x0100
#define MSR_LMA                 0x0400
#define MSR_NXE                 0x0800
#define MSR_PAT                 0x0277
#define MSR_DEBUG_CTL           0x01D9

//
//  Flags in MSR_IA32_MISC_ENABLE
//
#define MSR_XD_ENABLE_MASK      0xFFFFFFFB

//
//  Flags in MSR_DEBUG_CTL
//
#define MSR_DEBUG_CTL_LBR       0x0001
#define MSR_DEBUG_CTL_BTF       0x0002

//
// IPI Types
//
#define IPI_APC                 1
#define IPI_DPC                 2
#define IPI_FREEZE              4
#define IPI_PACKET_READY        8
#define IPI_SYNCH_REQUEST       16

//
// PRCB Flags
//
#define PRCB_MAJOR_VERSION      1
#define PRCB_BUILD_DEBUG        1
#define PRCB_BUILD_UNIPROCESSOR 2

//
// Exception active flags
//
#define KEXCEPTION_ACTIVE_INTERRUPT_FRAME 0x0000
#define KEXCEPTION_ACTIVE_EXCEPTION_FRAME 0x0001
#define KEXCEPTION_ACTIVE_SERVICE_FRAME   0x0002

//
// HAL Variables
//
#define INITIAL_STALL_COUNT     100
#define MM_HAL_VA_START         0xFFFFFFFFFFC00000ULL /* This is Vista+ */
#define MM_HAL_VA_END           0xFFFFFFFFFFFFFFFFULL
#define APIC_BASE               0xFFFFFFFFFFFE0000ULL

//
// IOPM Definitions
//
#define IO_ACCESS_MAP_NONE      0
#define IOPM_OFFSET             FIELD_OFFSET(KTSS, IoMaps[0].IoMap)
#define KiComputeIopmOffset(MapNumber)              \
    (MapNumber == IO_ACCESS_MAP_NONE) ?             \
        (USHORT)(sizeof(KTSS)) :                    \
        (USHORT)(FIELD_OFFSET(KTSS, IoMaps[MapNumber-1].IoMap))

//
// Static Kernel-Mode Address start (use MM_KSEG0_BASE for actual)
//
#define KSEG0_BASE 0xfffff80000000000ULL

//
// Synchronization-level IRQL
//
#define SYNCH_LEVEL 12

#define NMI_STACK_SIZE 0x2000
#define ISR_STACK_SIZE 0x6000

//
// Number of pool lookaside lists per pool in the PRCB
//
#define NUMBER_POOL_LOOKASIDE_LISTS 32

//
// Structure for CPUID
//
typedef union _CPU_INFO
{
	UINT32 AsUINT32[4];
	struct
	{
		ULONG Eax;
		ULONG Ebx;
		ULONG Ecx;
		ULONG Edx;
	};
} CPU_INFO, * PCPU_INFO;

//
// Trap Frame Definition
//
typedef struct _KTRAP_FRAME
{
	UINT64 P1Home;
	UINT64 P2Home;
	UINT64 P3Home;
	UINT64 P4Home;
	UINT64 P5;
	CHAR PreviousMode;
	UCHAR PreviousIrql;
	UCHAR FaultIndicator;
	UCHAR ExceptionActive;
	ULONG MxCsr;
	UINT64 Rax;
	UINT64 Rcx;
	UINT64 Rdx;
	UINT64 R8;
	UINT64 R9;
	UINT64 R10;
	UINT64 R11;
	union
	{
		UINT64 GsBase;
		UINT64 GsSwap;
	};
	M128A Xmm0;
	M128A Xmm1;
	M128A Xmm2;
	M128A Xmm3;
	M128A Xmm4;
	M128A Xmm5;
	union
	{
		UINT64 FaultAddress;
		UINT64 ContextRecord;
		UINT64 TimeStampCKCL;
	};
	UINT64 Dr0;
	UINT64 Dr1;
	UINT64 Dr2;
	UINT64 Dr3;
	UINT64 Dr6;
	UINT64 Dr7;
	union
	{
		struct
		{
			UINT64 DebugControl;
			UINT64 LastBranchToRip;
			UINT64 LastBranchFromRip;
			UINT64 LastExceptionToRip;
			UINT64 LastExceptionFromRip;
		};
		struct
		{
			UINT64 LastBranchControl;
			ULONG LastBranchMSR;
		};
	};
	USHORT SegDs;
	USHORT SegEs;
	USHORT SegFs;
	USHORT SegGs;
	UINT64 TrapFrame;
	UINT64 Rbx;
	UINT64 Rdi;
	UINT64 Rsi;
	UINT64 Rbp;
	union
	{
		UINT64 ErrorCode;
		UINT64 ExceptionFrame;
		UINT64 TimeStampKlog;
	};
	UINT64 Rip;
	USHORT SegCs;
	UCHAR Fill0;
	UCHAR Logging;
	USHORT Fill1[2];
	ULONG EFlags;
	ULONG Fill2;
	UINT64 Rsp;
	USHORT SegSs;
	USHORT Fill3;
	LONG CodePatchCycle;
} KTRAP_FRAME, * PKTRAP_FRAME;

//
// Dummy LDT_ENTRY
//
#ifndef _LDT_ENTRY_DEFINED
#define _LDT_ENTRY_DEFINED
typedef ULONG LDT_ENTRY;
#endif

//
// GDT Entry Definition
//
typedef union _KGDTENTRY64
{
	struct
	{
		USHORT LimitLow;
		USHORT BaseLow;
		union
		{
			struct
			{
				UCHAR BaseMiddle;
				UCHAR Flags1;
				UCHAR Flags2;
				UCHAR BaseHigh;
			} Bytes;
			struct
			{
				ULONG BaseMiddle : 8;
				ULONG Type : 5;
				ULONG Dpl : 2;
				ULONG Present : 1;
				ULONG LimitHigh : 4;
				ULONG System : 1;
				ULONG LongMode : 1;
				ULONG DefaultBig : 1;
				ULONG Granularity : 1;
				ULONG BaseHigh : 8;
			} Bits;
		};
		ULONG BaseUpper;
		ULONG MustBeZero;
	};
	UINT64 Alignment;
} KGDTENTRY64, * PKGDTENTRY64;
#define KGDTENTRY KGDTENTRY64
#define PKGDTENTRY PKGDTENTRY64

//
// IDT Entry Access Definition
//
typedef struct _KIDT_ACCESS
{
	union
	{
		struct
		{
			UCHAR Reserved;
			UCHAR SegmentType : 4;
			UCHAR SystemSegmentFlag : 1;
			UCHAR Dpl : 2;
			UCHAR Present : 1;
		};
		USHORT Value;
	};
} KIDT_ACCESS, * PKIDT_ACCESS;

//
// IDT Entry Definition
//
typedef union _KIDTENTRY64
{
	struct
	{
		USHORT OffsetLow;
		USHORT Selector;
		USHORT IstIndex : 3;
		USHORT Reserved0 : 5;
		USHORT Type : 5;
		USHORT Dpl : 2;
		USHORT Present : 1;
		USHORT OffsetMiddle;
		ULONG OffsetHigh;
		ULONG Reserved1;
	};
	UINT64 Alignment;
} KIDTENTRY64, * PKIDTENTRY64;
#define KIDTENTRY KIDTENTRY64
#define PKIDTENTRY PKIDTENTRY64

typedef struct _KDESCRIPTOR
{
	USHORT Pad[3];
	USHORT Limit;
	PVOID Base;
} KDESCRIPTOR, * PKDESCRIPTOR;

#ifndef NTOS_MODE_USER

//
// Special Registers Structure (outside of CONTEXT)
//
typedef struct _KSPECIAL_REGISTERS
{
	ULONG64 Cr0;
	ULONG64 Cr2;
	ULONG64 Cr3;
	ULONG64 Cr4;
	ULONG64 KernelDr0;
	ULONG64 KernelDr1;
	ULONG64 KernelDr2;
	ULONG64 KernelDr3;
	ULONG64 KernelDr6;
	ULONG64 KernelDr7;
	KDESCRIPTOR Gdtr;
	KDESCRIPTOR Idtr;
	USHORT Tr;
	USHORT Ldtr;
	ULONG MxCsr;
	ULONG64 DebugControl;
	ULONG64 LastBranchToRip;
	ULONG64 LastBranchFromRip;
	ULONG64 LastExceptionToRip;
	ULONG64 LastExceptionFromRip;
	ULONG64 Cr8;
	ULONG64 MsrGsBase;
	ULONG64 MsrGsSwap;
	ULONG64 MsrStar;
	ULONG64 MsrLStar;
	ULONG64 MsrCStar;
	ULONG64 MsrSyscallMask;
} KSPECIAL_REGISTERS, * PKSPECIAL_REGISTERS;

//
// Processor State Data
//
typedef struct _KPROCESSOR_STATE
{
	KSPECIAL_REGISTERS SpecialRegisters;
	CONTEXT ContextFrame;
} KPROCESSOR_STATE, * PKPROCESSOR_STATE;

#if (NTDDI_VERSION < NTDDI_LONGHORN)
#define GENERAL_LOOKASIDE_POOL PP_LOOKASIDE_LIST
#endif

typedef struct _KREQUEST_PACKET
{
	PVOID CurrentPacket[3];
	PVOID WorkerRoutine;
} KREQUEST_PACKET, * PKREQUEST_PACKET;

typedef struct _REQUEST_MAILBOX
{
	INT64 RequestSummary;
	KREQUEST_PACKET RequestPacket;
	PVOID Virtual[7];
} REQUEST_MAILBOX, * PREQUEST_MAILBOX;

//
// Processor Region Control Block
// *HEAVILY* Modified
//
#pragma pack(push,4)
typedef struct _KPRCB
{
	USHORT Number;
	KPROCESSOR_STATE ProcessorState;
	CHAR CpuType;
	CHAR CpuID;
} KPRCB, * PKPRCB;
#pragma pack(pop)

//
// TSS Definition
//
typedef struct _KiIoAccessMap
{
	UCHAR DirectionMap[32];
	UCHAR IoMap[8196];
} KIIO_ACCESS_MAP;


#pragma pack(push,4)
typedef struct _KTSS64
{
	/* 000 */  ULONG Reserved0;
	/* 004 */  UINT64 Rsp0;
	/* 00c */  UINT64 Rsp1;
	/* 014 */  UINT64 Rsp2;
	/* 01c */  UINT64 Ist[8];
	/* 05c */  UINT64 Reserved1;
	/* 064 */  USHORT Reserved2;
	/* 066 */  USHORT IoMapBase;
} KTSS64, * PKTSS64;
#pragma pack(pop)
#define KTSS KTSS64
#define PKTSS PKTSS64

//
// KEXCEPTION_FRAME
//
typedef struct _KEXCEPTION_FRAME
{
	ULONG64 P1Home;
	ULONG64 P2Home;
	ULONG64 P3Home;
	ULONG64 P4Home;
	ULONG64 P5;
#if (NTDDI_VERSION >= NTDDI_WIN8)
	ULONG64 Spare1;
#else
	ULONG64 InitialStack;
#endif
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
	ULONG64 TrapFrame;
#if (NTDDI_VERSION < NTDDI_WIN8)
	ULONG64 CallbackStack;
#endif
	ULONG64 OutputBuffer;
	ULONG64 OutputLength;
#if (NTDDI_VERSION >= NTDDI_WIN8)
	ULONG64 Spare2;
#endif
	ULONG64 MxCsr;
	ULONG64 Rbp;
	ULONG64 Rbx;
	ULONG64 Rdi;
	ULONG64 Rsi;
	ULONG64 R12;
	ULONG64 R13;
	ULONG64 R14;
	ULONG64 R15;
	ULONG64 Return;
} KEXCEPTION_FRAME, * PKEXCEPTION_FRAME;

typedef struct _MACHINE_FRAME
{
	ULONG64 Rip;
	USHORT SegCs;
	USHORT Fill1[3];
	ULONG EFlags;
	ULONG Fill2;
	ULONG64 Rsp;
	USHORT SegSs;
	USHORT Fill3[3];
} MACHINE_FRAME, * PMACHINE_FRAME;

//
// Defines the Callback Stack Layout for User Mode Callbacks
//
typedef KEXCEPTION_FRAME KCALLOUT_FRAME, * PKCALLOUT_FRAME;

//
// User side callout frame
//
typedef struct _UCALLOUT_FRAME
{
	ULONG64 P1Home;
	ULONG64 P2Home;
	ULONG64 P3Home;
	ULONG64 P4Home;
	PVOID Buffer;
	ULONG Length;
	ULONG ApiNumber;
	MACHINE_FRAME MachineFrame;
} UCALLOUT_FRAME, * PUCALLOUT_FRAME; // size = 0x0058

//
// Stack frame layout for KiUserExceptionDispatcher
// The name is totally made up
//
typedef struct _KUSER_EXCEPTION_STACK
{
	CONTEXT Context;
	EXCEPTION_RECORD ExceptionRecord;
	ULONG64 Alignment;
	MACHINE_FRAME MachineFrame;
} KUSER_EXCEPTION_STACK, * PKUSER_EXCEPTION_STACK;

typedef struct _KSTART_FRAME
{
	ULONG64 P1Home;
	ULONG64 P2Home;
	ULONG64 P3Home;
	ULONG64 P4Home;
	ULONG64 Reserved;
	ULONG64 Return;
} KSTART_FRAME, * PKSTART_FRAME;

#define PROCESSOR_START_FLAG_FORCE_ENABLE_NX 0x0001
typedef struct _KPROCESSOR_START_BLOCK
{
	ULONG CompletionFlag; // 0x0004
	ULONG Flags; // 0x0008
	ULONG Gdt32; // 0x000C
	ULONG Idt32; // 0x0012
	PVOID Gdt; // 0x0018
	// ???
	ULONG64 TiledMemoryMap; // 0x0058
	UCHAR PmTarget[6]; // 0x0060
	UCHAR LmIdentityTarget[6]; // 0x0066
	ULONG64 LmTarget; // 0x0070
	struct _KPROCESSOR_START_BLOCK* SelfMap; // 0x0078
	ULONG64 MsrPat; // 0x0080
	ULONG64 MsrEFER; // 0x0088
	KPROCESSOR_STATE ProcessorState; // 0x0090
} KPROCESSOR_START_BLOCK, * PKPROCESSOR_START_BLOCK; // size 00640

#endif
#endif
