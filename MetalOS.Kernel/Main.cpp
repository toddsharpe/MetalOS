#include "Main.h"
#include "msvc.h"
#include <efi.h>
#include "LoaderParams.h"
#include "Display.h"
#include "LoadingScreen.h"
#include "MetalOS.h"
#include "System.h"
#include "MemoryMap.h"
#include <intrin.h>

#include "PageTables.h"
#include "PageTablesPool.h"
#include "x64.h"
#include "KernelHeap.h"

#include <crt_stdio.h>

#include <vector>
#include "Bitvector.h"
#include "PageFrameAllocator.h"

extern "C"
{
#include <acpi.h>
}


const Color Red = { 0x00, 0x00, 0xFF, 0x00 };
const Color Black = { 0x00, 0x00, 0x00, 0x00 };

//Kernel members
Display* display;
LoadingScreen* loading;
PageTablesPool* pagePool;
MemoryMap* memoryMap;
PageFrameAllocator* frameAllocator;
uint32_t LastProcessId = 0;
std::vector<KERNEL_PROCESS> *processes;

//Queues
//std::queue<uint32_t> readyQueue;
//std::list<uint32_t> sleepQueue;

//Kernel Stack
KERNEL_PAGE_ALIGN volatile UINT8 KERNEL_STACK[KERNEL_STACK_SIZE] = { 0 };
extern "C" UINT64 KERNEL_STACK_STOP = (UINT64)&KERNEL_STACK[KERNEL_STACK_SIZE];

//Kernel Heap
KERNEL_PAGE_ALIGN static volatile UINT8 KERNEL_HEAP[KERNEL_HEAP_SIZE] = { 0 };
KERNEL_GLOBAL_ALIGN static KernelHeap heap((UINT64)KERNEL_HEAP, KERNEL_HEAP_SIZE);

KERNEL_GLOBAL_ALIGN static LOADER_PARAMS LoaderParams = { 0 };
KERNEL_GLOBAL_ALIGN static UINT8 EFI_MEMORY_MAP[MemoryMapReservedSize] = { 0 };

extern "C" void INTERRUPT_HANDLER(size_t vector, PINTERRUPT_FRAME pFrame)
{
	loading->WriteLineFormat("ISR: %d, Code: %d, RBP: 0x%16x, RIP: 0x%16x", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP);
	switch (vector)
	{
	case 14:
		loading->WriteLineFormat("CR2: 0x%16x", __readcr2());
		break;
	}
}

extern "C" void Print(const char* format, ...)
{
	va_list ap;
	va_start(ap, format);
	VPrint(format, ap);
	va_end(ap);
}

extern "C" void VPrint(const char* format, va_list Args)
{
	char buffer[80];

	int retval = crt_vsprintf(buffer, format, Args);
	buffer[retval] = '\0';

	loading->WriteLine(buffer);
}

void* operator new(size_t n)
{
	uintptr_t p = heap.Allocate(n);
	loading->WriteLineFormat("Allocation 0x%016x (0x%x)", p, n);
	return (void*)p;
}

void operator delete(void* p)
{
	loading->WriteLineFormat("Delete at 0x%16x", p);
	heap.Deallocate((UINT64)p);
}

void operator delete(void* p, size_t n)
{
	loading->WriteLineFormat("Delete at 0x%16x 0x%x", p, n);
	heap.Deallocate((UINT64)p);
}

//Copy loader params and all recursive structures to kernel memory
extern "C" void main_thunk(LOADER_PARAMS* loader)
{
	memcpy(&LoaderParams, loader, sizeof(LOADER_PARAMS));
	LoaderParams.MemoryMap = (EFI_MEMORY_DESCRIPTOR*)EFI_MEMORY_MAP;
	memcpy(EFI_MEMORY_MAP, loader->MemoryMap, loader->MemoryMapSize);

	main(&LoaderParams);
}

extern "C" void syscall()
{
	loading->WriteLineFormat("Syscall!");
}

//Need to get virtual pointers to acpi struct
//Find way to keep RuntimeServicesData/Code in kernel address space

void main(LOADER_PARAMS* loader)
{
	//Immediately set up graphics device so we can bugcheck gracefully
	display = new Display(&loader->Display);
	display->ColorScreen(Black);
	loading = new LoadingScreen(*display);

	//loading->WriteLineFormat("Syscall!");
	//__halt();

	//Initialize page tables
	pagePool = new PageTablesPool(loader->PageTablesPoolAddress, loader->PageTablesPoolPageCount);
	pagePool->SetVirtualAddress(KernelPageTablesPoolAddress);

	//Initialize memorymap. Call SetVirtualAddressMap, then modify - TODO
	memoryMap = new MemoryMap(loader->MemoryMapSize, loader->MemoryMapDescriptorSize, loader->MemoryMapDescriptorVersion, loader->MemoryMap, PAGE_SIZE);
	//memoryMap->SetVirtualOffset(KernelPhysicalMemoryAddress);
	//Assert(loader->Runtime->SetVirtualAddressMap(loader->MemoryMapSize, loader->MemoryMapDescriptorSize, loader->MemoryMapDescriptorVersion, loader->MemoryMap) == EFI_SUCCESS);
	//loader->Runtime = MakePtr(EFI_RUNTIME_SERVICES*, loader->Runtime, KernelPhysicalMemoryAddress);
	//loading->WriteLineFormat("runtime 0x%16x time: 0x%16x", loader->Runtime, loader->Runtime->GetTime);
	//__halt();

	//EFI_TIME time;
	//loader->Runtime->GetTime(&time, nullptr);
	//loading->WriteLineFormat("print 0x%d", time.Hour);
	//__halt();

	memoryMap->ReclaimBootPages();
	memoryMap->MergeConventionalPages();
	memoryMap->DumpMemoryMap();

	//Initialize Frame Allocator
	frameAllocator = new PageFrameAllocator(*memoryMap);

	//x64 Initialization
	x64::Initialize();

	//Test interrupts
	__debugbreak();
	__debugbreak();

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
	UINT64 ptRoot;
	Assert(pagePool->AllocatePage(&ptRoot));
	PageTables kernelPT(ptRoot);
	kernelPT.SetPool(pagePool);
	kernelPT.SetVirtualOffset(KernelPageTablesPoolAddress - LoaderParams.PageTablesPoolAddress);
	kernelPT.MapKernelPages(KernelBaseAddress, loader->KernelAddress, EFI_SIZE_TO_PAGES(loader->KernelImageSize));
	kernelPT.MapKernelPages(KernelPageTablesPoolAddress, loader->PageTablesPoolAddress, loader->PageTablesPoolPageCount);
	kernelPT.MapKernelPages(KernelGraphicsDeviceAddress, loader->Display.FrameBufferBase, EFI_SIZE_TO_PAGES(loader->Display.FrameBufferSize));
	loader->Display.FrameBufferBase = KernelGraphicsDeviceAddress;
	__writecr3(ptRoot);

	loading->WriteLineFormat("MetalOS.Kernel - Base:0x%16x Size: 0x%x", LoaderParams.KernelAddress, LoaderParams.KernelImageSize);
	loading->WriteLineFormat("LOADER_PARAMS: 0x%16x", loader);
	loading->WriteLineFormat("ConfigTableSizes: %d", loader->ConfigTableSizes);
	loading->WriteLineFormat("MemoryMap: 0x%16x, PhysicalAddressSize: 0x%16x", loader->MemoryMap, memoryMap->GetPhysicalAddressSize());
	loading->WriteLineFormat("Display.FrameBufferBase: 0x%16x", loader->Display.FrameBufferBase);
	loading->WriteLineFormat("PageTablesPool.AllocatedPageCount: 0x%8x", pagePool->AllocatedPageCount());


	//Access current EFI memory map
	//Its on its own page so we are fine with resizing

	__halt();
}

//TODO: fix when merging loading screen and display
void KernelBugcheck(const char* file, const char* line, const char* assert)
{
	//display.ColorScreen(Red);
	
	//loading->ResetX();
	//loading->ResetY();
	loading->WriteLine(file);
	loading->WriteLine(line);
	loading->WriteLine(assert);

	//halt
	__halt();
}

