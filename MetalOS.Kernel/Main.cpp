#include "Main.h"

#include "msvc.h"
#include <efi.h>
#include <LoaderParams.h>
#include <crt_stdio.h>
#include <vector>
#include <intrin.h>
#include "MetalOS.Kernel.h"
#include "MetalOS.h"
#include "System.h"
#include "MemoryMap.h"
#include "PageTables.h"
#include "PageTablesPool.h"
#include "x64.h"
#include "KernelHeap.h"
#include "Bitvector.h"
#include "PageFrameAllocator.h"

extern "C"
{
#include <acpi.h>
}

//Kernel members
PageFrameAllocator* frameAllocator;

Kernel kernel;

//Kernel Stack
KERNEL_PAGE_ALIGN volatile UINT8 KERNEL_STACK[KERNEL_STACK_SIZE] = { 0 };
extern "C" UINT64 KERNEL_STACK_STOP = (UINT64)&KERNEL_STACK[KERNEL_STACK_SIZE];

//Kernel Heap
KERNEL_PAGE_ALIGN static volatile UINT8 KERNEL_HEAP[KERNEL_HEAP_SIZE] = { 0 };
KERNEL_GLOBAL_ALIGN static KernelHeap heap((UINT64)KERNEL_HEAP, KERNEL_HEAP_SIZE);

extern "C" void INTERRUPT_HANDLER(size_t vector, PINTERRUPT_FRAME pFrame)
{
	kernel.HandleInterrupt(vector, pFrame);
}

extern "C" void Print(const char* format, ...)
{
	va_list args;

	va_start(args, format);
	kernel.Print(format, args);
	va_end(args);
}

extern "C" void VPrint(const char* format, va_list args)
{
	kernel.Print(format, args);
}

void* operator new(size_t n)
{
	uintptr_t p = heap.Allocate(n);
	Print("Allocation 0x%016x (0x%x)", p, n);
	return (void*)p;
}

void operator delete(void* p)
{
	Print("Delete at 0x%16x", p);
	heap.Deallocate((UINT64)p);
}

void operator delete(void* p, size_t n)
{
	Print("Delete at 0x%16x 0x%x", p, n);
	heap.Deallocate((UINT64)p);
}

extern "C" void syscall()
{
	Print("Syscall!");
}

//Need to get virtual pointers to acpi struct
//Find way to keep RuntimeServicesData/Code in kernel address space

void main(LOADER_PARAMS* loader)
{
	kernel.Initialize(loader);


	//Initialize memorymap. Call SetVirtualAddressMap, then modify - TODO
	
	//memoryMap->SetVirtualOffset(KernelPhysicalMemoryAddress);
	//Assert(loader->Runtime->SetVirtualAddressMap(loader->MemoryMapSize, loader->MemoryMapDescriptorSize, loader->MemoryMapDescriptorVersion, loader->MemoryMap) == EFI_SUCCESS);
	//loader->Runtime = MakePtr(EFI_RUNTIME_SERVICES*, loader->Runtime, KernelPhysicalMemoryAddress);
	//loading->WriteLineFormat("runtime 0x%16x time: 0x%16x", loader->Runtime, loader->Runtime->GetTime);
	//__halt();

	//EFI_TIME time;
	//loader->Runtime->GetTime(&time, nullptr);
	//loading->WriteLineFormat("print 0x%d", time.Hour);
	//__halt();

	//Initialize Frame Allocator
	//frameAllocator = new PageFrameAllocator(*memoryMap);

	//x64 Initialization



	//ACPI
	//ACPI_STATUS Status;
	//Status = AcpiInitializeSubsystem();
	//if (ACPI_FAILURE(Status))
	//{
	//	Print("Could not initialize ACPI: %d\n", Status);
	//	__halt();
	//}
	//__halt();
	//AcpiInitializeTables(0, 0, FALSE);

	//System system(loader->ConfigTables, loader->ConfigTableSizes);
	//system.GetInstalledSystemRam();
	//system.DisplayTableIds();
	//system.DisplayAcpi2();


	//Map in kernel to new PT. PageTablesPool has been mapped in by bootloader
	//UINT64 ptRoot;
	//Assert(pagePool->AllocatePage(&ptRoot));
	//PageTables kernelPT(ptRoot);
	//kernelPT.SetPool(pagePool);
	//kernelPT.SetVirtualOffset(KernelPageTablesPoolAddress - LoaderParams.PageTablesPoolAddress);
	//kernelPT.MapKernelPages(KernelBaseAddress, loader->KernelAddress, EFI_SIZE_TO_PAGES(loader->KernelImageSize));
	//kernelPT.MapKernelPages(KernelPageTablesPoolAddress, loader->PageTablesPoolAddress, loader->PageTablesPoolPageCount);
	//kernelPT.MapKernelPages(KernelGraphicsDeviceAddress, loader->Display.FrameBufferBase, EFI_SIZE_TO_PAGES(loader->Display.FrameBufferSize));
	//loader->Display.FrameBufferBase = KernelGraphicsDeviceAddress;
	//__writecr3(ptRoot);

	//Print("MetalOS.Kernel - Base:0x%16x Size: 0x%x", LoaderParams.KernelAddress, LoaderParams.KernelImageSize);
	//Print("LOADER_PARAMS: 0x%16x", loader);
	//Print("ConfigTableSizes: %d", loader->ConfigTableSizes);
	//Print("MemoryMap: 0x%16x, PhysicalAddressSize: 0x%16x", loader->MemoryMap, memoryMap->GetPhysicalAddressSize());
	//Print("Display.FrameBufferBase: 0x%16x", loader->Display.FrameBufferBase);
	//Print("PageTablesPool.AllocatedPageCount: 0x%8x", pagePool->AllocatedPageCount());


	//Access current EFI memory map
	//Its on its own page so we are fine with resizing

	__halt();
}

