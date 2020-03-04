#include "Kernel.h"

#include <intrin.h>
#include <cstdarg>
#include "x64.h"
#include "x64_support.h"
#include "System.h"

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
	m_spinLocks(),
	m_semaphores(),
	m_pdata(nullptr)
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
	//__debugbreak();

	Print("MetalOS.Kernel - Base:0x%16x Size: 0x%x", m_physicalAddress, m_imageSize);
	//Print("ConfigTableSizes: %d", loader->ConfigTableSizes);
	Print("  PhysicalAddressSize: 0x%16x", m_pMemoryMap->GetPhysicalAddressSize());
	//Print("  PageTablesPool.AllocatedPageCount: 0x%8x", m_pPagePool->AllocatedPageCount());

	//Complete initialization
	m_processes = new std::list<KERNEL_PROCESS>();
	m_spinLocks = new std::list<PSPIN_LOCK>();
	m_semaphores = new std::list<PSEMAPHORE>();

	m_pdata = GetKernelSection(".pdata");

	Print("m_pdata: 0x%16x", (uintptr_t)m_pdata);

	m_pLoading->WriteText("Kernel Initialized");
}

void Kernel::HandleInterrupt(size_t vector, PINTERRUPT_FRAME pFrame)
{
	__halt();
	
	m_pLoading->WriteLineFormat("ISR: %d, Code: %d, RBP: 0x%16x, RIP: 0x%16x, RSP: 0x%16x", vector, pFrame->ErrorCode, pFrame->RBP, pFrame->RIP, pFrame->RSP);
	m_pLoading->WriteLineFormat("  RAX: 0x%16x, RBX: 0x%16x, RCX: 0x%16x, RDX: 0x%16x", pFrame->RAX, pFrame->RBX, pFrame->RCX, pFrame->RDX);
	switch (vector)
	{
	//Let debug continue (we use this to check ISRs on bootup)
	case 3:
		return;
	case 14:
		m_pLoading->WriteLineFormat("CR2: 0x%16x", __readcr2());
	}

	//shitty stalk walk
	uint64_t ip = pFrame->RIP;

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

PSPIN_LOCK Kernel::GetSpinLock(Handle id)
{
	for (const PSPIN_LOCK& item : *m_spinLocks)
	{
		if (item->Id == id)
			return item;
	}

	return nullptr;
}

Handle Kernel::CreateSpinlock()
{
	PSPIN_LOCK pLock = new SPIN_LOCK();
	Assert(pLock != nullptr);

	pLock->Id = m_objectId++;
	pLock->Value = 0;

	m_spinLocks->push_back(pLock);

	return pLock->Id;
}

cpu_flags_t Kernel::AcquireSpinlock(Handle id)
{
	PSPIN_LOCK pLock = GetSpinLock(id);
	Assert(pLock != nullptr);

	cpu_flags_t flags = x64_disable_interrupts();
	
	static_assert(std::numeric_limits<size_t>::digits == 64);
	while (_InterlockedCompareExchange64((volatile long long*)& pLock->Value, 0, 1) != 0)
		x64_pause();

	return flags;
}

void Kernel::ReleaseSpinlock(Handle id, cpu_flags_t flags)
{
	PSPIN_LOCK pLock = GetSpinLock(id);
	Assert(pLock != nullptr);

	pLock->Value = 0;

	x64_restore_flags(flags);
}

PSEMAPHORE Kernel::GetSemaphore(Handle id)
{
	for (const PSEMAPHORE& item : *m_semaphores)
	{
		if (item->Id == id)
			return item;
	}

	return nullptr;
}

Handle Kernel::CreateSemaphore(uint32_t initial, uint32_t maximum, const char* name)
{
	PSEMAPHORE pSemaphore = new SEMAPHORE();
	Assert(pSemaphore != nullptr);
	pSemaphore->Id = m_objectId++;
	pSemaphore->SpinLock = CreateSpinlock();
	pSemaphore->Value = initial;
	pSemaphore->Limit = maximum;
	pSemaphore->Name = name;

	m_semaphores->push_back(pSemaphore);

	return pSemaphore->Id;
}

//TODO: timeout
bool Kernel::WaitSemaphore(Handle id, size_t count, size_t timeout)
{
	//Acquire semaphore
	PSEMAPHORE semaphore = GetSemaphore(id);
	cpu_flags_t flags = AcquireSpinlock(semaphore->SpinLock);

	bool loop = semaphore->Value >= count;
	while (!loop)
	{
		ReleaseSpinlock(semaphore->SpinLock, flags);
		x64_pause();
		flags = AcquireSpinlock(semaphore->SpinLock);
		loop = semaphore->Value >= count;
	}

	semaphore->Value -= count;
	ReleaseSpinlock(semaphore->SpinLock, flags);

	return true;
}

void Kernel::SignalSemaphore(Handle id, size_t count)
{
	//Acquire semaphore
	PSEMAPHORE semaphore = GetSemaphore(id);
	cpu_flags_t flags = AcquireSpinlock(semaphore->SpinLock);

	semaphore->Value += count;

	ReleaseSpinlock(semaphore->SpinLock, flags);
}

PIMAGE_SECTION_HEADER Kernel::GetKernelSection(const std::string& name)
{
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)KernelBaseAddress;
	PIMAGE_NT_HEADERS64 pNtHeader = (PIMAGE_NT_HEADERS64)(KernelBaseAddress + dosHeader->e_lfanew);

	//Find section
	PIMAGE_SECTION_HEADER crtSection = nullptr;
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION_64(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		if ((char*)&section[i].Name == name)
		{
			return &section[i];
		}
	}

	Assert(false);
	return nullptr;
}

void* Kernel::GetAcpiRoot()
{
	return m_pConfigTables->GetAcpiTable();
}

//TODO: fix this mathy shit
uint64_t Kernel::GetAcpiTimer()
{
	EFI_TIME time;
	m_runtime.GetTime(&time, nullptr);
	uint64_t timer;
	uint16_t year = time.Year;
	uint8_t month = time.Month;
	uint8_t day = time.Day;
	timer = ((uint64_t)(year / 4 - year / 100 + year / 400 + 367 * month / 12 + day) +
		year * 365 - 719499);
	timer *= 24;
	timer += time.Hour;
	timer *= 60;
	timer += time.Minute;
	timer *= 60;
	timer += time.Second;
	timer *= 10000000;	//100 n intervals
	timer += time.Nanosecond / 100;
	return timer;
}
