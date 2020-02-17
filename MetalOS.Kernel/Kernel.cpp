#include "Kernel.h"

#include <intrin.h>
#include <cstdarg>
#include "x64.h"

const Color Red = { 0x00, 0x00, 0xFF, 0x00 };
const Color Black = { 0x00, 0x00, 0x00, 0x00 };

Kernel::Kernel() :
	m_address(0),
	m_imageSize(0),
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

	//Initialize Heap

	//Initialize Display
	m_pDisplay = new Display(params->Display);
	m_pDisplay->ColorScreen(Black);
	m_pLoading = new LoadingScreen(*m_pDisplay);

	//Initialize page tables
	m_pPagePool = new PageTablesPool(params->PageTablesPoolAddress, params->PageTablesPoolPageCount);
	m_pPagePool->SetVirtualAddress(KernelPageTablesPoolAddress);
	
	//Access current PT
	PageTables kernelPT(__readcr3());
	kernelPT.SetPool(m_pPagePool);
	kernelPT.SetVirtualOffset(KernelPageTablesPoolAddress - params->PageTablesPoolAddress);
	
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = params->MemoryMap;
		current < NextMemoryDescriptor(params->MemoryMap, params->MemoryMapSize);
		current = NextMemoryDescriptor(current, params->MemoryMapDescriptorSize))
	{
		if ((current->Attribute & EFI_MEMORY_RUNTIME) == 0)
			continue;

		Assert(kernelPT.MapKernelPages(current->VirtualStart, current->PhysicalStart, current->NumberOfPages));
	}

	//Initialize memory map
	m_pMemoryMap = new MemoryMap(params->MemoryMapSize, params->MemoryMapDescriptorSize, params->MemoryMapDescriptorVersion, params->MemoryMap);
	EFI_TIME time;
	//params->Runtime->GetTime(&time, nullptr);
	Print("Date: %02d-%02d-%02d %02d:%02d:%02d\r\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

	//s = params->Runtime->ConvertPointer(0, (void**)&params->Runtime->ResetSystem);
	//m_pLoading->WriteLineFormat("R %d", s);

//	m_pMemoryMap->ReclaimBootPages();
//	m_pMemoryMap->MergeConventionalPages();
	m_pMemoryMap->DumpMemoryMap();



	//Test interrupts
	__debugbreak();
	__debugbreak();

	m_pLoading->WriteText("Kernel Initialized");
	m_pLoading->WriteLineFormat("CRC: 0x%08x", params->Runtime->Hdr.CRC32);
	m_pLoading->WriteLineFormat("GetTime: 0x%16x", params->Runtime->GetTime);
	m_pLoading->WriteLineFormat("CP: 0x%16x", params->Runtime->ConvertPointer);

}

void Kernel::HandleInterrupt(size_t vector, PINTERRUPT_FRAME pFrame)
{
	m_pLoading->WriteLineFormat("ISR: %d, Code: %d, RBP: 0x%16x, RIP: 0x%16x", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP);
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
