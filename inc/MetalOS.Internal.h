#pragma once

//Shared header between loader and kernel

//This is defined in Kernel but kernel shouldn't be shared. TODO
#define UserStart   0x0
//#define UserStop    0x00007FFFFFFFFFFF
#define UserStop    0x0000800000000000
#define KernelStart 0xFFFF800000000000

#define RamDriveSize 0x4000000 //64MB ramdrive

//16MB region unused
#define KernelBaseAddress (KernelStart + 0x1000000)//16 MB kernel
#define KernelPageTablesPoolAddress (KernelStart + 0x2000000)//16MB page pool (currently only 2mb is used - 512 * 4096)
#define KernelGraphicsDeviceAddress (KernelStart + 0x3000000)//16MB graphics device (Hyper-v device uses 8MB)
#define KernelRamDriveAddress (KernelStart + 0x4000000)//64MB Ram Drive
#define KernelRuntimeAddress (KernelStart + 0x100000000000)//UEFI services needed to exist in runtime
#define KernelACPIAddress (KernelStart + 0x200000000000)//ACPI Request area. ACPI requests pages to be mapped so use this chunk
#define KernelDriverIOAddress (KernelStart + 0x300000000000)//Driver IO space
#define KernelRuntimeStart (KernelStart + 0x400000000000)//Connected to kernel virtual address space for runtime mappings
#define KernelRuntimeEnd (KernelStart + 0x500000000000)
#define KernelPfnDbStart (KernelStart + 0x500000000000)//PFNDatabase
#define KernelPdbStart (KernelStart + 0x600000000000)//PDB

#define PAGE_SHIFT  12
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PAGE_MASK   0xFFF

#define SIZE_TO_PAGES(a)  \
    ( ((a) >> PAGE_SHIFT) + ((a) & PAGE_MASK ? 1 : 0) )

#define MemoryMapReservedSize PAGE_SIZE

//4mb reserved space
#define BootloaderPagePoolCount 256
#define ReservedPageTablePages 512

enum class SystemCall : uint64_t
{
	GetSystemInfo = 1,
	GetProcessInfo,
	ExitProcess,
	ExitThread,
	CreateWindow,
	WaitForMessages,
	GetMessage,
	CreateFile,
	ReadFile,
	SetFilePosition,
	VirtualAlloc,
	DebugPrint,
	LoadLibrary,
	GetProcAddress
};





