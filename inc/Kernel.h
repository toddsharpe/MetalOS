#pragma once

#include "MetalOS.h"

#define NO_COPY_OR_ASSIGN(X) X(const X&) = delete; X& operator = (const X&) = delete;
#define Assert(x) if (!(x)) { KernelBugcheck(#x); }
#define Fatal(x) KernelBugcheck(x);

#define MakePtr( cast, ptr, addValue ) (cast)( (UINT64)(ptr) + (UINT64)(addValue))

//Paging Structures - https://gist.github.com/mvankuipers/
//https://queazan.wordpress.com/2013/12/21/paging-under-amd64/
//https://software.intel.com/sites/default/files/managed/39/c5/325462-sdm-vol-1-2abcd-3abcd.pdf
typedef unsigned __int64 ULONG64;
typedef void* PVOID;

#define GDT_EMPTY 0
#define GDT_KERNEL_CODE 1
#define GDT_KERNEL_DATA 2
#define GDT_USER_CODE 3
#define GDT_USER_DATA 4
#define GDT_TSS_ENTRY 5

#define UserDPL 3
#define KernelDPL 0

enum IDT_GATE_TYPE
{
	TaskGate32 = 0x5,
	InterruptGate16 = 0x6,
	TrapGate16 = 0x7,
	InterruptGate32 = 0xE,
	TrapGate32 = 0xF
};

#pragma pack(push, 1)
typedef struct _PML4E
{
	union
	{
		struct
		{
			ULONG64 Present : 1;              // Must be 1, region invalid if 0.
			ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
			ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
			ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access PDPT.
			ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access PDPT.
			ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
			ULONG64 Ignored1 : 1;
			ULONG64 PageSize : 1;             // Must be 0 for PML4E.
			ULONG64 Ignored2 : 4;
			ULONG64 PageFrameNumber : 36;     // The page frame number of the PDPT of this PML4E.
			ULONG64 Reserved : 4;
			ULONG64 Ignored3 : 11;
			ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
		};
		ULONG64 Value;
	};
} PML4E, * PPML4E;
static_assert(sizeof(PML4E) == sizeof(PVOID), "Size mismatch, only 64-bit supported.");

typedef struct _PDPTE
{
	union
	{
		struct
		{
			ULONG64 Present : 1;              // Must be 1, region invalid if 0.
			ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
			ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
			ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access PD.
			ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access PD.
			ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
			ULONG64 Ignored1 : 1;
			ULONG64 PageSize : 1;             // If 1, this entry maps a 1GB page.
			ULONG64 Ignored2 : 4;
			ULONG64 PageFrameNumber : 36;     // The page frame number of the PD of this PDPTE.
			ULONG64 Reserved : 4;
			ULONG64 Ignored3 : 11;
			ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
		};
		ULONG64 Value;
	};
} PDPTE, * PPDPTE;
static_assert(sizeof(PDPTE) == sizeof(PVOID), "Size mismatch, only 64-bit supported.");

typedef struct _PDE
{
	union
	{
		struct
		{
			ULONG64 Present : 1;              // Must be 1, region invalid if 0.
			ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
			ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
			ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access PT.
			ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access PT.
			ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
			ULONG64 Ignored1 : 1;
			ULONG64 PageSize : 1;             // If 1, this entry maps a 2MB page.
			ULONG64 Ignored2 : 4;
			ULONG64 PageFrameNumber : 36;     // The page frame number of the PT of this PDE.
			ULONG64 Reserved : 4;
			ULONG64 Ignored3 : 11;
			ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
		};
		ULONG64 Value;
	};
} PDE, * PPDE;
static_assert(sizeof(PDE) == sizeof(PVOID), "Size mismatch, only 64-bit supported.");

typedef struct _PTE
{
	union
	{
		struct
		{
			ULONG64 Present : 1;              // Must be 1, region invalid if 0.
			ULONG64 ReadWrite : 1;            // If 0, writes not allowed.
			ULONG64 UserSupervisor : 1;       // If 0, user-mode accesses not allowed.
			ULONG64 PageWriteThrough : 1;     // Determines the memory type used to access the memory.
			ULONG64 PageCacheDisable : 1;     // Determines the memory type used to access the memory.
			ULONG64 Accessed : 1;             // If 0, this entry has not been used for translation.
			ULONG64 Dirty : 1;                // If 0, the memory backing this page has not been written to.
			ULONG64 PageAccessType : 1;       // Determines the memory type used to access the memory.
			ULONG64 Global : 1;                // If 1 and the PGE bit of CR4 is set, translations are global.
			ULONG64 Ignored2 : 3;
			ULONG64 PageFrameNumber : 36;     // The page frame number of the backing physical page.
			ULONG64 Reserved : 4;
			ULONG64 Ignored3 : 7;
			ULONG64 ProtectionKey : 4;         // If the PKE bit of CR4 is set, determines the protection key.
			ULONG64 ExecuteDisable : 1;       // If 1, instruction fetches not allowed.
		};
		ULONG64 Value;
	};
} PTE, * PPTE;
static_assert(sizeof(PTE) == sizeof(PVOID), "Size mismatch, only 64-bit supported.");

// Intel SDM Vol 3A Figure 3-6
typedef struct _SEGMENT_SELECTOR
{
	union
	{
		struct
		{
			UINT16 PrivilegeLevel : 2;
			UINT16 TableIndicator : 1; // 0 is GDT, 1 is LDT
			UINT16 Index : 13;
		};
		UINT16 Value;
	};
} SEGMENT_SELECTOR, *PSEGME;
static_assert(sizeof(SEGMENT_SELECTOR) == sizeof(UINT16), "Size mismatch, only 64-bit supported.");

// Intel SDM Vol 3A Figure 3-8
typedef struct _SEGMENT_DESCRIPTOR
{
	union
	{
		struct
		{
			UINT64 SegmentLimit1 : 16;
			UINT64 BaseAddress1 : 16;
			UINT64 BaseAddress2 : 8;
			UINT64 Type : 4;
			UINT64 S : 1; // 1 if code/data, 0 if system segment
			UINT64 DPL : 2; //Descriptor Privilege Level
			UINT64 Present : 1;
			UINT64 SegmentLimit2 : 4;
			UINT64 Available : 1; // For use by OS
			UINT64 L : 1; //Should always be 0 for data
			UINT64 DB : 1;
			UINT64 Granulatiry : 1; // 0=1b-1mb, 1=4kb-4gb
			UINT64 BaseAddress3 : 8;
		};
		UINT64 Value;
	};
} SEGMENT_DESCRIPTOR, *PSEGMENT_DESCRIPTOR;
static_assert(sizeof(SEGMENT_DESCRIPTOR) == sizeof(PVOID), "Size mismatch, only 64-bit supported.");

// Intel SDM Vol 3A Figure 7-4
typedef struct _TSS_LDT_ENTRY
{
	UINT16 SegmentLimit1;
	UINT16 BaseAddress1;

	UINT16 BaseAddress2 : 8;
	UINT16 Type : 4;
	UINT16 Zero1 : 1;
	UINT16 PrivilegeLevel : 2; // DPL
	UINT16 Present : 1;

	UINT16 Limit : 4;
	UINT16 Available : 1;
	UINT16 Zero2 : 1;
	UINT16 Zero3 : 1;
	UINT16 Granularity : 1;
	UINT16 BaseAddress3 : 8;

	UINT32 BaseAddress4;

	UINT32 Reserved1 : 8;
	UINT32 Zeros : 4;
	UINT32 Reserved2 : 20;
} TSS_LDT_ENTRY, * PTSS_LDT_ENTRY;
static_assert(sizeof(TSS_LDT_ENTRY) == 16, "Size mismatch, only 64-bit supported.");

// Intel SDM Vol 3A Figure 7-11
typedef struct _TASK_STATE_SEGMENT_64
{
	UINT32 Reserved_0;
	//RSP for privilege levels 0-2
	UINT32 RSP_0_low;
	UINT32 RSP_0_high;
	UINT32 RSP_1_low;
	UINT32 RSP_1_high;
	UINT32 RSP_2_low;
	UINT32 RSP_2_high;
	UINT32 Reserved_1;
	UINT32 Reserved_2;
	//ISTs
	UINT32 IST_1_low;
	UINT32 IST_1_high;
	UINT32 IST_2_low;
	UINT32 IST_2_high;
	UINT32 IST_3_low;
	UINT32 IST_3_high;
	UINT32 IST_4_low;
	UINT32 IST_4_high;
	UINT32 IST_5_low;
	UINT32 IST_5_high;
	UINT32 IST_6_low;
	UINT32 IST_6_high;
	UINT32 IST_7_low;
	UINT32 IST_7_high;
	UINT32 Reserved_3;
	UINT32 Reserved_4;
	UINT16 Reserved_5;
	UINT16 IO_Map_Base;
} TASK_STATE_SEGMENT_64, *PTASK_STATE_SEGMENT_64;
static_assert(sizeof(TASK_STATE_SEGMENT_64) == 104, "Size mismatch, only 64-bit supported.");

// Intel SDM Vol 3A Figure 6-7
typedef struct _IDT_GATE
{
	UINT16 Offset1;
	SEGMENT_SELECTOR SegmentSelector;
	UINT16 InterruptStackTable : 3;
	UINT16 Zeros : 5;
	UINT16 Type : 4;
	UINT16 Zero : 1;
	UINT16 PrivilegeLevel : 2; // DPL
	UINT16 Present : 1;
	UINT16 Offset2;
	UINT32 Offset3;
	UINT32 Reserved;

	_IDT_GATE() {}

	_IDT_GATE(UINT64 isrAddress, UINT16 stack, IDT_GATE_TYPE type)
	{
		Offset1 = (UINT16)isrAddress;
		SegmentSelector.Value = 0;
		SegmentSelector.PrivilegeLevel = GDT_KERNEL_CODE;
		InterruptStackTable = stack;
		Zeros = 0;
		Type = type;
		Zero = 0;
		PrivilegeLevel = KernelDPL;
		Present = true;
		Offset2 = (UINT16)(isrAddress >> 16);
		Offset3 = (UINT32)(isrAddress >> 32);
	}
} IDT_GATE, *PIDT_GATE;
static_assert(sizeof(IDT_GATE) == 16, "Size mismatch, only 64-bit supported.");

// Intel SDM Vol 3A Figure 3-11
typedef struct _DESCRIPTOR_TABLE
{
	UINT16 Limit;
	UINT64 BaseAddress;
} DESCRIPTOR_TABLE, *PDESCRIPTOR_TABLE;
static_assert(sizeof(DESCRIPTOR_TABLE) == 10, "Size mismatch, only 64-bit supported.");

//Modern kernel has 5 GDTs (first has to be empty, plus 2x user and 2x kernel), plus the last entry is actually a TSS entry, mandatory.
typedef struct _KERNEL_GDTS
{
	SEGMENT_DESCRIPTOR Empty;
	SEGMENT_DESCRIPTOR KernelCode;
	SEGMENT_DESCRIPTOR KernelData;
	SEGMENT_DESCRIPTOR UserCode;
	SEGMENT_DESCRIPTOR UserData;
	TSS_LDT_ENTRY TssEntry;
} KERNEL_GDTS, *PKERNEL_GDTS;

typedef struct
{
	UINT64 ISR_NUM;
	
	UINT64 RAX;
	UINT64 RBX;
	UINT64 RCX;
	UINT64 RDX;
	UINT64 RSI;
	UINT64 RDI;
	UINT64 R8;
	UINT64 R9;
	UINT64 R10;
	UINT64 R11;
	UINT64 R12;
	UINT64 R13;
	UINT64 R14;
	UINT64 R15;
	UINT64 RBP;

	//Intel SDM Vol3A Figure 6-4
	UINT64 RIP;
	UINT64 CS;
	UINT64 RFlags;
	UINT64 RSP;
	UINT64 SS;
} INTERRUPT_FRAME, *PINTERRUPT_FRAME;

typedef struct
{
	UINT64 ISR_NUM;
	
	UINT64 RAX;
	UINT64 RBX;
	UINT64 RCX;
	UINT64 RDX;
	UINT64 RSI;
	UINT64 RDI;
	UINT64 R8;
	UINT64 R9;
	UINT64 R10;
	UINT64 R11;
	UINT64 R12;
	UINT64 R13;
	UINT64 R14;
	UINT64 R15;
	UINT64 RBP;

	//Intel SDM Vol3A Figure 6-4
	UINT64 ErrorCode;
	UINT64 RIP;
	UINT64 CS;
	UINT64 RFlags;
	UINT64 RSP;
	UINT64 SS;
} EXCEPTION_FRAME, * PEXCEPTION_FRAME;

#pragma pack(pop)

#define UINT64_MAX 0xFFFFFFFFFFFFFFFF

#define PAGE_SIZE (1 << 12)

#define PAGE_SIZE   4096
#define PAGE_MASK   0xFFF
#define PAGE_SHIFT  12

#define SIZE_TO_PAGES(a)  \
    ( ((a) >> PAGE_SHIFT) + ((a) & PAGE_MASK ? 1 : 0) )


//4mb reserved space
#define ReservedPageTablePages 512
#define ReservedPageTableSpace (ReservedPageTablePages * PAGE_SIZE)
#define ReservedPageTableSpaceMask (ReservedPageTableSpace - 1)

//User space starts at   0x00000000 00000000
//User space stops at    0x00007FFF FFFFFFFF
//Kernel space starts at 0xFFFF8000 00000000
//Kernel space stops at  0xFFFFFFFF FFFFFFFF

#define UserStop    0x00007FFFFFFFFFFF
#define KernelStart 0xFFFF800000000000

#define KernelBaseAddress (KernelStart + 0x1000000)//16 MB kernel
#define KernelPageTablesPoolAddress (KernelStart + 0x2000000)//16MB page pool (currently only 2mb is used - 512 * 4096)
#define KernelGraphicsDeviceAddress (KernelStart + 0x3000000)//16MB graphics device (Hyper-v device uses 8MB)
#define KernelStop UINT64_MAX

#define ISR_HANDLER(x) x ## _ISR_HANDLER
#define DEF_ISR_HANDLER(x) void ISR_HANDLER(x) ## ()
#define EXC_HANDLER(x) x ## _EXC_HANDLER
#define DEF_EXC_HANDLER(x) void EXC_HANDLER(x) ## ()

#define KERNEL_STACK_SIZE (1 << 20)

#define IDT_COUNT 256
#define IST_STACK_SIZE (1 << 12)
#define IST_DOUBLEFAULT_IDX 1
#define IST_NMI_IDX 2
#define IST_DEBUG_IDX 3
#define IST_MCE_IDX 4


#define PLACEHOLDER 0

#define KERNEL_GLOBAL_ALIGN __declspec(align(64))

//We should just change the base address of the kernel image
//#define KernelBaseAddress 0x100000

typedef unsigned __int64    uint64_t;
typedef __int64             int64_t;
typedef unsigned __int32    uint32_t;
typedef __int32             int32_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned char       uint8_t;
typedef char                int8_t;
typedef long long          intmax_t;
typedef unsigned long long uintmax_t;

//Types
typedef uint8_t UINT8;
typedef uint16_t UINT16;
typedef uint32_t UINT32;
typedef uint64_t UINT64;

typedef UINT16          CHAR16;
typedef UINT64          EFI_PHYSICAL_ADDRESS;

typedef struct
{
	EFI_PHYSICAL_ADDRESS FrameBufferBase;
	UINT32 FrameBufferSize;
	UINT32 HorizontalResolution;
	UINT32 VerticalResolution;
	UINT32 PixelsPerScanLine;
} GRAPHICS_DEVICE, * PGRAPHICS_DEVICE;

