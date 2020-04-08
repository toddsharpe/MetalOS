#pragma once

//Shared header between loader and kernel

//This is defined in Kernel but kernel shouldn't be shared. TODO
#define UserStop    0x00007FFFFFFFFFFF
#define KernelStart 0xFFFF800000000000

//16MB region unused
#define KernelBaseAddress (KernelStart + 0x1000000)//16 MB kernel
#define KernelPageTablesPoolAddress (KernelStart + 0x2000000)//16MB page pool (currently only 2mb is used - 512 * 4096)
#define KernelGraphicsDeviceAddress (KernelStart + 0x3000000)//16MB graphics device (Hyper-v device uses 8MB)
#define KernelRuntimeAddress (KernelStart + 0x100000000000)//UEFI services needed to exist in runtime
#define KernelACPIAddress (KernelStart + 0x200000000000)//ACPI Request area. ACPI requests pages to be mapped so use this chunk
#define KernerDriverIOAddress (KernelStart + 0x300000000000)//Driver IO space

#define PAGE_SHIFT  12
#define PAGE_SIZE (1 << PAGE_SHIFT)
#define PAGE_MASK   0xFFF

#define MemoryMapReservedSize PAGE_SIZE

//4mb reserved space
#define BootloaderPagePoolCount 256
#define ReservedPageTablePages 512

