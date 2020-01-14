#pragma once

#include <cstdint>
#include <time.h>

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define NO_COPY_OR_ASSIGN(X) X(const X&) = delete; X& operator = (const X&) = delete;
#define Assert(x) if (!(x)) { KernelBugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); }
#define Fatal(x) KernelBugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); 

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

#define UserStop    0x00007FFFFFFFFFFF
#define KernelStart 0xFFFF800000000000

//16MB region unused
#define KernelBaseAddress (KernelStart + 0x1000000)//16 MB kernel
#define KernelPageTablesPoolAddress (KernelStart + 0x2000000)//16MB page pool (currently only 2mb is used - 512 * 4096)
#define KernelGraphicsDeviceAddress (KernelStart + 0x3000000)//16MB graphics device (Hyper-v device uses 8MB)
#define KernelRuntimeAddress (KernelStart + 0x100000000000)//

#define KernelHeapSize 0x1000000

#define MemoryMapReservedSize PAGE_SIZE

#define KernelStop UINT64_MAX

#define ISR_HANDLER(x) x64_interrupt_handler_ ## x
#define DEF_ISR_HANDLER(x) void ISR_HANDLER(x) ## ()

#define KERNEL_STACK_SIZE (1 << 20)
#define KERNEL_HEAP_SIZE (1 << 20)

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

//We should just change the base address of the kernel image
//#define KernelBaseAddress 0x100000

typedef struct
{
	uintptr_t FrameBufferBase;
	uint32_t FrameBufferSize;
	uint32_t HorizontalResolution;
	uint32_t VerticalResolution;
	uint32_t PixelsPerScanLine;
} GRAPHICS_DEVICE, * PGRAPHICS_DEVICE;

typedef struct
{
	uint32_t Id;
	time_t CreateTime;
	time_t ExitTime;
} KERNEL_PROCESS, * PKERNEL_PROCESS;
