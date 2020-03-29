#include "Kernel.h"

typedef EFI_GUID GUID;
#define PACKED
#include <PlatformAcpi.h>
#include <intrin.h>
#include <cstdarg>
#include "x64.h"
#include "x64_support.h"
#include "System.h"
#include "Main.h"
#include <functional>
#include "AcpiDevice.h"
#include "UartDriver.h"
#include "RuntimeSupport.h"

const Color Red = { 0x00, 0x00, 0xFF, 0x00 };
const Color Black = { 0x00, 0x00, 0x00, 0x00 };

Kernel::Kernel() :
	m_physicalAddress(0),
	m_imageSize(0),
	m_runtime(),
	m_pPagePool(nullptr),
	m_pMemoryMap(nullptr),
	m_pConfigTables(nullptr),
	m_pageTables(nullptr),
	m_pDisplay(nullptr),
	m_pLoading(nullptr),
	m_lastProcessId(0),
	m_processes(nullptr),
	m_objectId(0),
	m_deviceTree()
{

}

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { Bugcheck("File: " __FILE__, "Line: " STR(__LINE__),  #x); }
#define Trace() Print(__FILE__ "-" STR(__LINE__));
void Kernel::Initialize(const PLOADER_PARAMS params)
{
	//Save Loader Params
	m_physicalAddress = params->KernelAddress;
	m_imageSize = params->KernelImageSize;
	m_runtime = *params->Runtime;//UEFI has rewritten these pointers, now we copy them locally

	//Initialize Heap

	//Initialize Display
	m_pDisplay = new Display(params->Display, KernelGraphicsDeviceAddress);
	m_pDisplay->ColorScreen(Black);
	m_pLoading = new LoadingScreen(*m_pDisplay);

	//Initialize memory map
	m_pMemoryMap = new MemoryMap(params->MemoryMapSize, params->MemoryMapDescriptorSize, params->MemoryMapDescriptorVersion, params->MemoryMap);
	m_pMemoryMap->ReclaimBootPages();
	m_pMemoryMap->MergeConventionalPages();
	//m_pMemoryMap->DumpMemoryMap();

	//Config Tables
	m_pConfigTables = new ConfigTables(params->ConfigTables, params->ConfigTableSizes);
	//m_pConfigTables->Dump();

	//Initialize page table pool
	m_pPagePool = new PageTablesPool(KernelPageTablesPoolAddress, params->PageTablesPoolAddress, params->PageTablesPoolPageCount);

	//Construct kernel page table
	UINT64 ptRoot;
	Assert(m_pPagePool->AllocatePage(&ptRoot));
	m_pageTables = new PageTables(ptRoot);
	m_pageTables->SetPool(m_pPagePool);
	m_pageTables->MapKernelPages(KernelBaseAddress, params->KernelAddress, EFI_SIZE_TO_PAGES(params->KernelImageSize));
	m_pageTables->MapKernelPages(KernelPageTablesPoolAddress, params->PageTablesPoolAddress, params->PageTablesPoolPageCount);
	m_pageTables->MapKernelPages(KernelGraphicsDeviceAddress, params->Display.FrameBufferBase, EFI_SIZE_TO_PAGES(params->Display.FrameBufferSize));

	//Map in runtime addresses
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = params->MemoryMap;
		current < NextMemoryDescriptor(params->MemoryMap, params->MemoryMapSize);
		current = NextMemoryDescriptor(current, params->MemoryMapDescriptorSize))
	{
		if ((current->Attribute & EFI_MEMORY_RUNTIME) == 0)
			continue;

		Assert(m_pageTables->MapKernelPages(current->VirtualStart, current->PhysicalStart, current->NumberOfPages));
	}

	//Use new Kernel PT
	//Identity mappings are not possible now, lost access to params
	//Alternatively - remove physical identity mappings from bootloader PT since entry in pt root can be cleared and subsequent pages are going to be eaten (since they are in boot memory)
	//This requires copying at least the root
	__writecr3(ptRoot);

	//Test UEFI runtime access
	EFI_TIME time;
	m_runtime.GetTime(&time, nullptr);
	Print("Date: %02d-%02d-%02d %02d:%02d:%02d\r\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

	//Test interrupts
	//__debugbreak();

	Print("MetalOS.Kernel - Base:0x%16x Size: 0x%x\n", m_physicalAddress, m_imageSize);
	//Print("ConfigTableSizes: %d", loader->ConfigTableSizes);
	Print("  PhysicalAddressSize: 0x%16x\n", m_pMemoryMap->GetPhysicalAddressSize());
	//Print("  PageTablesPool.AllocatedPageCount: 0x%8x", m_pPagePool->AllocatedPageCount());

	//Complete initialization
	m_processes = new std::list<KERNEL_PROCESS>();

	//Initialized IO
	this->InitializeAcpi();

	//Devices
	m_deviceTree.Populate();
	m_deviceTree.PrintTree();

	//Save reference to Com1 for debugprint
	Assert(m_deviceTree.GetDeviceByName("COM1", &m_com1));
	m_com1->GetDriver()->Write("Com1 initialized\n");

	//Done
	Print("Kernel Initialized\n");
}

void Kernel::HandleInterrupt(size_t vector, PINTERRUPT_FRAME pFrame)
{
	m_pLoading->WriteLine("ISR: %d, Code: %d, RBP: 0x%16x, RIP: 0x%16x, RSP: 0x%16x\n", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP, pFrame->RSP);
	m_pLoading->WriteLine("  RAX: 0x%16x, RBX: 0x%16x, RCX: 0x%16x, RDX: 0x%16x\n", pFrame->RAX, pFrame->RBX, pFrame->RCX, pFrame->RDX);
	switch (vector)
	{
	//Let debug continue (we use this to check ISRs on bootup)
	case 3:
		return;
	case 14:
		m_pLoading->WriteLine("CR2: 0x%16x\n", __readcr2());
		if (__readcr2() == 0)
			m_pLoading->WriteLine("Null pointer dereference\n", __readcr2());
	}

	//shitty stalk walk
	__halt();

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
	m_pLoading->Write(format, args);
	va_end(args);
}

void Kernel::Print(const char* format, va_list args)
{
	m_pLoading->Write(format, args);
}

void Kernel::PrintArray(char* buffer, size_t length)
{
	for (size_t i = 0; i < length; i++)
	{
		if (i != 0 && i % 32 == 0)
			m_pLoading->Write("\n");

		m_pLoading->Write("%02x ", (unsigned char)buffer[i]);
	}
	m_pLoading->Write("\n");
}

void Kernel::InitializeAcpi()
{
	ACPI_STATUS Status;
	Status = AcpiInitializeSubsystem();
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiInitializeSubsystem: %d\n", Status);
		__halt();
	}

	Status = AcpiInitializeTables(nullptr, 16, FALSE);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiInitializeTables: %d\n", Status);
		__halt();
	}

	//TODO: notify handlers

	/* Install the default address space handlers. */
	Status = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_SYSTEM_MEMORY, ACPI_DEFAULT_HANDLER, NULL, NULL);
	if (ACPI_FAILURE(Status))
	{
		Print("ACPI", "Could not initialise SystemMemory handler, %s!", AcpiFormatException(Status));
		__halt();
	}

	Status = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_SYSTEM_IO, ACPI_DEFAULT_HANDLER, NULL, NULL);
	if (ACPI_FAILURE(Status))
	{
		Print("ACPI", "Could not initialise SystemIO handler, %s!", AcpiFormatException(Status));
		__halt();
	}

	Status = AcpiInstallAddressSpaceHandler(ACPI_ROOT_OBJECT, ACPI_ADR_SPACE_PCI_CONFIG, ACPI_DEFAULT_HANDLER, NULL, NULL);
	if (ACPI_FAILURE(Status))
	{
		Print("ACPI", "Could not initialise PciConfig handler, %s!", AcpiFormatException(Status));
		__halt();
	}

	Status = AcpiLoadTables();
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiLoadTables: %d\n", Status);
		__halt();
	}

	//Local handlers should be installed here

	Status = AcpiEnableSubsystem(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiEnableSubsystem: %d\n", Status);
		__halt();
	}

	Status = AcpiInitializeObjects(ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE(Status))
	{
		Print("Could not AcpiInitializeObjects: %d\n", Status);
		__halt();
	}

	Print("ACPI Finished\n");
}
