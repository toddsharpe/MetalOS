#include "Kernel.h"

#include <intrin.h>
#include <cstdarg>
#include "x64.h"

const Color Red = { 0x00, 0x00, 0xFF, 0x00 };
const Color Black = { 0x00, 0x00, 0x00, 0x00 };

Kernel::Kernel() :
	m_address(0),
	m_imageSize(0),
	m_runtime(),
	m_pPagePool(nullptr),
	m_pMemoryMap(nullptr),
	m_pLoading(nullptr),
	m_pDisplay(nullptr),
	m_lastProcessId(0),
	m_processes(nullptr),
	m_lastSemaphoreId(0),
	m_semaphores(nullptr)
{

}

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); }
void Kernel::Initialize(const PLOADER_PARAMS params)
{
	//Save Loader Params
	m_address = params->KernelAddress;
	m_imageSize = params->KernelImageSize;
	m_runtime = *params->Runtime;

	//Initialize Heap

	//Initialize Display
	m_pDisplay = new Display(params->Display, KernelGraphicsDeviceAddress);
	m_pDisplay->ColorScreen(Black);
	m_pLoading = new LoadingScreen(*m_pDisplay);

	//Initialize memory map
	m_pMemoryMap = new MemoryMap(params->MemoryMapSize, params->MemoryMapDescriptorSize, params->MemoryMapDescriptorVersion, params->MemoryMap);
	m_pMemoryMap->ReclaimBootPages();
	m_pMemoryMap->MergeConventionalPages();
	m_pMemoryMap->DumpMemoryMap();

	//Initialize page table pool
	m_pPagePool = new PageTablesPool(KernelPageTablesPoolAddress, params->PageTablesPoolAddress, params->PageTablesPoolPageCount);
	
	//Construct kernel page table
	UINT64 ptRoot;
	Assert(m_pPagePool->AllocatePage(&ptRoot));
	PageTables kernelPT(ptRoot);
	kernelPT.SetPool(m_pPagePool);
	kernelPT.MapKernelPages(KernelBaseAddress, params->KernelAddress, EFI_SIZE_TO_PAGES(params->KernelImageSize));
	kernelPT.MapKernelPages(KernelPageTablesPoolAddress, params->PageTablesPoolAddress, params->PageTablesPoolPageCount);
	kernelPT.MapKernelPages(KernelGraphicsDeviceAddress, params->Display.FrameBufferBase, EFI_SIZE_TO_PAGES(params->Display.FrameBufferSize));

	//Map in runtime addresses
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = params->MemoryMap;
		current < NextMemoryDescriptor(params->MemoryMap, params->MemoryMapSize);
		current = NextMemoryDescriptor(current, params->MemoryMapDescriptorSize))
	{
		if ((current->Attribute & EFI_MEMORY_RUNTIME) == 0)
			continue;

		Assert(kernelPT.MapKernelPages(current->VirtualStart, current->PhysicalStart, current->NumberOfPages));
	}

	//Use new Kernel PT
	//Identity mappings are not possible now, lost access to params
	//Alternatively - remove physical identity mappings from bootloader PT since entry in pt root can be cleared and subsequent pages are going to be eaten (since they are in boot memory)
	__writecr3(ptRoot);

	//Test UEFI runtime access
	EFI_TIME time;
	m_runtime.GetTime(&time, nullptr);
	Print("Date: %02d-%02d-%02d %02d:%02d:%02d\r\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

	//Test interrupts
	__debugbreak();
	__debugbreak();

	Print("MetalOS.Kernel - Base:0x%16x Size: 0x%x", m_address, m_imageSize);
	//Print("ConfigTableSizes: %d", loader->ConfigTableSizes);
	Print("  PhysicalAddressSize: 0x%16x", m_pMemoryMap->GetPhysicalAddressSize());
	//Print("  PageTablesPool.AllocatedPageCount: 0x%8x", m_pPagePool->AllocatedPageCount());

	m_pLoading->WriteText("Kernel Initialized");
}

void Kernel::HandleInterrupt(size_t vector, PINTERRUPT_FRAME pFrame)
{
	m_pLoading->WriteLineFormat("ISR: %d, Code: %d, RBP: 0x%16x, RIP: 0x%16x, Code: 0x%16x", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP, pFrame->ErrorCode);
	switch (vector)
	{
	case 14:
		m_pLoading->WriteLineFormat("CR2: 0x%16x", __readcr2());
		break;
	}
}

void Kernel::Bugcheck(const char* file, const char* line, const char* assert)
{
	//display.ColorScreen(Red);
	//TODO: color background function (pass lambda bool to colorscreen)

	//loading->ResetX();
	//loading->ResetY();
	m_pLoading->WriteLine(file);
	m_pLoading->WriteLine(line);
	m_pLoading->WriteLine(assert);

	__halt();
}

void Kernel::Print(const char* format, ...)
{
	va_list args;

	va_start(args, format);
	m_pLoading->WriteLineFormat(format, args);
	va_end(args);
}

void Kernel::Print(const char* format, va_list args)
{
	m_pLoading->WriteLineFormat(format, args);
}
