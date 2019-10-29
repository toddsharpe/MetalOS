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

typedef struct _SegmentDescriptor
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
		UINT64 Available : 1; // Used for OS
		UINT64 L : 1; //1 for 64bit code (d must be cleared)
		UINT64 DB : 1;
		UINT64 Granulatiry : 1; // 0=1b-1mb, 1=4kb-4gb
		UINT64 BaseAddress3 : 8;
	};
} SegmentDescriptor, *PSegmentDescriptor;
static_assert(sizeof(SegmentDescriptor) == sizeof(PVOID), "Size mismatch, only 64-bit supported.");

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
#define KernelStop UINT64_MAX

//We should just change the base address of the kernel image
//#define KernelBaseAddress 0x100000




