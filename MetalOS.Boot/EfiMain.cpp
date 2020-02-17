#include "EfiMain.h"

#include <intrin.h>
#include <crt_string.h>
#include <crt_wchar.h>
#include <crt_stdlib.h>
#include <Path.h>
#include <PageTables.h>
#include <PageTablesPool.h>
#include "MetalOS.Boot.h"
#include "Device.h"
#include "EfiLoader.h"
#include "Error.h"
#include "Memory.h"

#define EFI_DEBUG 1
#define Kernel L"moskrnl.exe"
#define MaxKernelPath 64

EFI_SYSTEM_TABLE* ST;
EFI_RUNTIME_SERVICES* RT;
EFI_BOOT_SERVICES* BS;
EFI_MEMORY_TYPE AllocationType;

LOADER_PARAMS LoaderParams = { 0 };

extern "C" EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_STATUS status;

	//Save UEFI environment
	ST = SystemTable;
	BS = SystemTable->BootServices;
	RT = SystemTable->RuntimeServices;

	//Populate params
	LoaderParams.ConfigTables = ST->ConfigurationTable;
	LoaderParams.ConfigTableSizes = ST->NumberOfTableEntries;
	LoaderParams.Runtime = RT;

	ReturnIfNotSuccess(ST->ConOut->ClearScreen(ST->ConOut));

	//Disable the stupid watchdog - TODO: why doesnt it go away on its own? Maybe because i didnt call SetVirtualAddressMap?
	ReturnIfNotSuccess(BS->SetWatchdogTimer(0, 0, 0, nullptr));

	EFI_LOADED_IMAGE* LoadedImage;
	ReturnIfNotSuccess(BS->OpenProtocol(ImageHandle, &LoadedImageProtocol, (void**)&LoadedImage, NULL, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	AllocationType = LoadedImage->ImageDataType;
	CHAR16* BootFilePath = ((FILEPATH_DEVICE_PATH*)LoadedImage->FilePath)->PathName;

	//Display some splash info
	ReturnIfNotSuccess(Print(L"MetalOS.BootLoader\r\n"));
	ReturnIfNotSuccess(Print(L"  Firmware Vendor: %s, Revision: %d\r\n", ST->FirmwareVendor, ST->FirmwareRevision));
	ReturnIfNotSuccess(Print(L"  Bootloader: %s\r\n", BootFilePath));
	ReturnIfNotSuccess(Print(L"  ImageHandle: 0x%016x\r\n", ImageHandle));
	ReturnIfNotSuccess(PrintCpuDetails());

	//Display time
	EFI_TIME time;
	ReturnIfNotSuccess(RT->GetTime(&time, nullptr));
	Print(L"Date: %02d-%02d-%02d %02d:%02d:%02d\r\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

	//Determine size of memory map, allocate it in Boot Services Data so Kernel cleans it up
	UINTN memoryMapKey = 0;
	status = BS->GetMemoryMap(&LoaderParams.MemoryMapSize, LoaderParams.MemoryMap, &memoryMapKey, &LoaderParams.MemoryMapDescriptorSize, &LoaderParams.MemoryMapDescriptorVersion);
	if (status == EFI_BUFFER_TOO_SMALL)
	{
		ReturnIfNotSuccess(BS->AllocatePool(EfiBootServicesData, LoaderParams.MemoryMapSize, (void**)&LoaderParams.MemoryMap));
	}
	else
	{
		ReturnIfNotSuccess(status);
	}

	if (LoaderParams.MemoryMapSize > MemoryMapReservedSize)
	{
		ReturnIfNotSuccess(EFI_BUFFER_TOO_SMALL);
	}

	//Allocate pages for Bootloader PageTablesPool in BootServicesData
	//Pages from this pool will be used to bootstrap the kernel
	//Boot PT is allocated in boot services data so kernel knows it can be cleared
	EFI_PHYSICAL_ADDRESS bootloaderPagePoolAddress;
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, EfiBootServicesData, BootloaderPagePoolCount, &bootloaderPagePoolAddress));

	//Allocate pages for our Kernel's page pools
	//This pool will be kept for the kernel to use during runtime
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, AllocationType, ReservedPageTablePages, &(LoaderParams.PageTablesPoolAddress)));
	LoaderParams.PageTablesPoolPageCount = ReservedPageTablePages;

	//Build path to kernel
	CHAR16* KernelPath;
	ReturnIfNotSuccess(BS->AllocatePool(AllocationType, MaxKernelPath * sizeof(CHAR16), (void**)&KernelPath));
	memset((void*)KernelPath, 0, MaxKernelPath * sizeof(CHAR16));
	GetDirectoryName(BootFilePath, KernelPath);
	wcscpy(KernelPath + wcslen(KernelPath), Kernel);

	//Load kernel path
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fileSystem = nullptr;
	ReturnIfNotSuccess(ST->BootServices->OpenProtocol(LoadedImage->DeviceHandle, &FileSystemProtocol, (void**)&fileSystem, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	EFI_FILE* CurrentDriveRoot = nullptr;
	ReturnIfNotSuccess(fileSystem->OpenVolume(fileSystem, &CurrentDriveRoot));
	EFI_FILE* KernelFile = nullptr;
	Print(L"Loading: %s\r\n", KernelPath);
	ReturnIfNotSuccess(CurrentDriveRoot->Open(CurrentDriveRoot, &KernelFile, KernelPath, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY));

	//Map kernel into memory. It will be relocated at KernelBaseAddress
	UINT64 entryPoint;
	ReturnIfNotSuccess(EfiLoader::MapKernel(KernelFile, &LoaderParams.KernelImageSize, &entryPoint, &LoaderParams.KernelAddress));

	//Disable write protection, allowing current page tables to be modified
	__writecr0(__readcr0() & ~(1 << 16));

	//Expand current page tables to include Kernel address space and pagetables
	PageTablesPool pageTablesPool(bootloaderPagePoolAddress, BootloaderPagePoolCount);
	PageTables currentPT(__readcr3());
	currentPT.SetPool(&pageTablesPool);
	currentPT.MapKernelPages(KernelBaseAddress, LoaderParams.KernelAddress, EFI_SIZE_TO_PAGES(LoaderParams.KernelImageSize));
	Print(L"PageTablesPool.AllocatedPageCount: %x\r\n", pageTablesPool.AllocatedPageCount());
	currentPT.MapKernelPages(KernelPageTablesPoolAddress, LoaderParams.PageTablesPoolAddress, LoaderParams.PageTablesPoolPageCount);
	Print(L"PageTablesPool.AllocatedPageCount: %x\r\n", pageTablesPool.AllocatedPageCount());

	//Re-enable write protection
	__writecr0(__readcr0() | (1 << 16));

	//Finish kernel image initialization now that address space is constructed
	ReturnIfNotSuccess(EfiLoader::CrtInitialization(KernelBaseAddress));

	//Technically everything after allocating the loader block needs to free that memory before dying
	ReturnIfNotSuccess(InitializeGraphics(&LoaderParams.Display));
	ReturnIfNotSuccess(DisplayLoaderParams(&LoaderParams));

	//Pause here before booting kernel to inspect bootloader outputs
	Keywait();

	//Retrieve map from UEFI
	//This could fail on EFI_BUFFER_TOO_SMALL
	ReturnIfNotSuccess(BS->GetMemoryMap(&LoaderParams.MemoryMapSize, LoaderParams.MemoryMap, &memoryMapKey, &LoaderParams.MemoryMapDescriptorSize, &LoaderParams.MemoryMapDescriptorVersion));

	//After ExitBootServices we can no longer use the BS handle (no print, memory, etc)
	ReturnIfNotSuccess(BS->ExitBootServices(ImageHandle, memoryMapKey));

	//Assign virtual mappings for runtime sections
	for (EFI_MEMORY_DESCRIPTOR* current = LoaderParams.MemoryMap;
		current < NextMemoryDescriptor(LoaderParams.MemoryMap, LoaderParams.MemoryMapSize);
		current = NextMemoryDescriptor(current, LoaderParams.MemoryMapDescriptorSize))
	{
		if ((current->Attribute & EFI_MEMORY_RUNTIME) == 0)
			continue;

		current->VirtualStart = current->PhysicalStart + KernelRuntimeAddress;
	}

	//Update UEFI virtual address map
	ReturnIfNotSuccessNoDisplay(RT->SetVirtualAddressMap(LoaderParams.MemoryMapSize, LoaderParams.MemoryMapDescriptorSize, LoaderParams.MemoryMapDescriptorVersion, LoaderParams.MemoryMap));

	//Call into kernel
	KernelMain kernelMain = (KernelMain)(entryPoint);
	kernelMain(&LoaderParams);

	//Should never get here
	return EFI_ABORTED;
}

EFI_STATUS DisplayLoaderParams(LOADER_PARAMS* pParams)
{
	EFI_STATUS status;
	
	ReturnIfNotSuccess(Print(L"LoaderParams: 0x%016x, Pages: 0x%x\r\n", pParams, EFI_SIZE_TO_PAGES(sizeof(LOADER_PARAMS))));
	ReturnIfNotSuccess(Print(L"  Kernel-Address: 0x%016x, Pages: 0x%x\r\n", pParams->KernelAddress, EFI_SIZE_TO_PAGES(pParams->KernelImageSize)));
	ReturnIfNotSuccess(Print(L"  MemoryMap-Address: 0x%016x, Pages: 0x%x\r\n", pParams->MemoryMap, EFI_SIZE_TO_PAGES(pParams->MemoryMapSize)));
	ReturnIfNotSuccess(Print(L"  PageTablesPool-Address: 0x%016x, Pages: 0x%x\r\n", pParams->PageTablesPoolAddress, pParams->PageTablesPoolPageCount));
	ReturnIfNotSuccess(Print(L"  ConfigTables-Address: 0x%016x, Count: 0x%x\r\n", pParams->ConfigTables, pParams->ConfigTableSizes));

	PrintGraphicsDevice(&pParams->Display);
	return status;
}

#define MAXBUFFER 255
EFI_STATUS Print(const CHAR16* format, ...)
{
	va_list args;
	va_start(args, format);

	EFI_STATUS status;
	CHAR16 buffer[MAXBUFFER] = { 0 };
	vwprintf(buffer, format, args);

	//Write
	if (EFI_ERROR((status = ST->ConOut->OutputString(ST->ConOut, buffer))))
		return status;

	return EFI_SUCCESS;
}

EFI_STATUS Keywait(const CHAR16* String)
{
	EFI_STATUS status = EFI_SUCCESS;
	EFI_INPUT_KEY Key;

	if (String != nullptr)
		ReturnIfNotSuccess(Print(String));
	ReturnIfNotSuccess(Print(L"Press any key to continue..."));
	ReturnIfNotSuccess(ST->ConIn->Reset(ST->ConIn, FALSE));

	// Poll for key
	while ((status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY);

	// Clear keystroke buffer (this is just a pause)
	ReturnIfNotSuccess(ST->ConIn->Reset(ST->ConIn, FALSE));

	Print(L"\r\n");
	return status;
}

EFI_STATUS PrintCpuDetails()
{
	EFI_STATUS status;
	
	int registers[4] = { -1 };
	char vendor[13] = { 0 };
	CHAR16 wideVendor[13] = { 0 };

	__cpuid(registers, 0);
	*((UINT32*)vendor) = (UINT32)registers[1];
	*((UINT32*)(vendor + 4)) = (UINT32)registers[3];
	*((UINT32*)(vendor + 8)) = (UINT32)registers[2];

	mbstowcs(wideVendor, vendor, sizeof(wideVendor));

	ReturnIfNotSuccess(Print(L"CPU vendor: %s\r\n", wideVendor));

	__cpuid(registers, 0x80000001);
	//This means its supported, may not be active
	int x64 = (registers[3] & (1 << 29)) != 0;
	ReturnIfNotSuccess(Print(L"  CPU x64 Mode: %d\r\n", x64));

	UINT64 cr0 = __readcr0();
	int paging = (cr0 & ((UINT32)1 << 31)) != 0;
	ReturnIfNotSuccess(Print(L"  Paging: %d\r\n", (UINT32)paging));

	return status;
}

UINTN GetPhysicalAddressSize(UINTN MemoryMapSize, UINTN MemoryMapDescriptorSize, EFI_MEMORY_DESCRIPTOR* MemoryMap)
{
	UINTN highest = 0;

	EFI_MEMORY_DESCRIPTOR* current;
	for (current = MemoryMap; current < NextMemoryDescriptor(MemoryMap, MemoryMapSize); current = NextMemoryDescriptor(current, MemoryMapDescriptorSize))
	{
		uintptr_t address = current->PhysicalStart + (current->NumberOfPages << EFI_PAGE_SHIFT);
		if (address > highest)
			highest = address;
	}

	return highest;
}
