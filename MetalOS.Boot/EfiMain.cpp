#include "EfiMain.h"
#include "Kernel.h"
#include "BootLoader.h"
#include "EfiPrint.h"
#include "String.h"
#include "Device.h"
#include "EfiLoader.h"
#include "Error.h"
#include "Memory.h"
#include <intrin.h>
#include "CRT.h"

#include <PageTables.h>
#include <PageTablesPool.h>

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

	//Disable the stupid watchdog - TODO: why doesnt it go away on its own? Maybe because i didnt call SetVirtualAddressMap?
	ReturnIfNotSuccess(BS->SetWatchdogTimer(0, 0, 0, nullptr));

	EFI_LOADED_IMAGE* LoadedImage;
	ReturnIfNotSuccess(BS->OpenProtocol(ImageHandle, &LoadedImageProtocol, (void**)&LoadedImage, NULL, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	AllocationType = LoadedImage->ImageDataType;
	const CHAR16* BootFilePath = ((FILEPATH_DEVICE_PATH*)LoadedImage->FilePath)->PathName;

	//Display some splash info
	ReturnIfNotSuccess(Print(L"MetalOS.BootLoader\n\r"));
	ReturnIfNotSuccess(Print(L"  Firmware Vendor: %S, Revision: %d\n\r", ST->FirmwareVendor, ST->FirmwareRevision));
	ReturnIfNotSuccess(Print(L"  Bootloader: %S\r\n", BootFilePath));
	ReturnIfNotSuccess(Print(L"  ImageHandle: %q\r\n", ImageHandle));
	ReturnIfNotSuccess(PrintCpuDetails());

	//Display time
	EFI_TIME time;
	ReturnIfNotSuccess(RT->GetTime(&time, nullptr));
	Print(L"Date: %d-%d-%d %d:%d:%d\r\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

	//Determine size of memory map, allocate it in Boot Services Data so Kernel cleans it up
	UINTN memoryMapKey = 0;
	status = BS->GetMemoryMap(&LoaderParams.MemoryMapSize, LoaderParams.MemoryMap, &memoryMapKey, &LoaderParams.MemoryMapDescriptorSize, &LoaderParams.MemoryMapVersion);
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
	CRT::memset((void*)KernelPath, 0, MaxKernelPath * sizeof(CHAR16));
	CRT::GetDirectoryName(BootFilePath, KernelPath);
	CRT::strcpy(KernelPath + CRT::strlen(KernelPath), Kernel);

	//Load kernel path
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fileSystem = nullptr;
	ReturnIfNotSuccess(ST->BootServices->OpenProtocol(LoadedImage->DeviceHandle, &FileSystemProtocol, (void**)&fileSystem, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	EFI_FILE* CurrentDriveRoot = nullptr;
	ReturnIfNotSuccess(fileSystem->OpenVolume(fileSystem, &CurrentDriveRoot));
	EFI_FILE* KernelFile = nullptr;
	Print(L"Loading: %S\r\n", KernelPath);
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
	currentPT.MapKernelPages(KernelPageTablesPoolAddress, LoaderParams.PageTablesPoolAddress, LoaderParams.PageTablesPoolPageCount);

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
	ReturnIfNotSuccess(BS->GetMemoryMap(&LoaderParams.MemoryMapSize, LoaderParams.MemoryMap, &memoryMapKey, &LoaderParams.MemoryMapDescriptorSize, &LoaderParams.MemoryMapVersion));
	
	//After ExitBootServices we can no longer use the BS handle (no print, memory, etc)
	ReturnIfNotSuccess(BS->ExitBootServices(ImageHandle, memoryMapKey));

	//Call into kernel
	KernelMain kernelMain = (KernelMain)(entryPoint);
	kernelMain(&LoaderParams);

	//Should never get here
	return EFI_ABORTED;
}

EFI_STATUS DisplayLoaderParams(LOADER_PARAMS* pParams)
{
	EFI_STATUS status;
	
	ReturnIfNotSuccess(Print(L"LoaderParams: %q, Pages: 0x%2x\r\n", pParams, EFI_SIZE_TO_PAGES(sizeof(LOADER_PARAMS))));
	ReturnIfNotSuccess(Print(L"  Kernel-Address: %q, Pages: 0x%2x\r\n", pParams->KernelAddress, EFI_SIZE_TO_PAGES(pParams->KernelImageSize)));
	ReturnIfNotSuccess(Print(L"  MemoryMap-Address: %q, Pages: 0x%2x\r\n", pParams->MemoryMap, EFI_SIZE_TO_PAGES(pParams->MemoryMapSize)));
	ReturnIfNotSuccess(Print(L"  PageTablesPool-Address: %q, Pages: 0x%4x\r\n", pParams->PageTablesPoolAddress, pParams->PageTablesPoolPageCount));
	ReturnIfNotSuccess(Print(L"  ConfigTables-Address: %q, Count: 0x%2x\r\n", pParams->ConfigTables, pParams->ConfigTableSizes));

	PrintGraphicsDevice(&pParams->Display);
	return status;
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

EFI_STATUS DumpGDT()
{
	EFI_STATUS status;
	
	DESCRIPTOR_TABLE gdt = { 0 };
	_sgdt(&gdt);

	ReturnIfNotSuccess(Print(L"GDT-Limit: %w\r\n", gdt.Limit));
	ReturnIfNotSuccess(Print(L"GDT-Address: %q\r\n", gdt.BaseAddress));

	PSEGMENT_DESCRIPTOR pSegDesc = (PSEGMENT_DESCRIPTOR)gdt.BaseAddress;
	while ((UINT64)pSegDesc < gdt.BaseAddress + gdt.Limit)
	{
		ReturnIfNotSuccess(Print(L"  SegDesc: %q\r\n", pSegDesc->Value));
		pSegDesc++;
	}

	return status;
}

EFI_STATUS PrintCpuDetails()
{
	EFI_STATUS status;
	
	int registers[4] = { -1 };
	char vendor[13] = { 0 };

	__cpuid(registers, 0);
	*((UINT32*)vendor) = (UINT32)registers[1];
	*((UINT32*)(vendor + 4)) = (UINT32)registers[3];
	*((UINT32*)(vendor + 8)) = (UINT32)registers[2];

	ReturnIfNotSuccess(Print(L"CPU vendor: %s\r\n", vendor));

	__cpuid(registers, 0x80000001);
	//This means its supported, may not be active
	int x64 = (registers[3] & (1 << 29)) != 0;
	ReturnIfNotSuccess(Print(L"  CPU x64 Mode: %d\r\n", x64));

	UINT64 cr0 = __readcr0();
	int paging = (cr0 & ((UINT32)1 << 31)) != 0;
	ReturnIfNotSuccess(Print(L"  Paging: %d\r\n", (UINT32)paging));

	return status;
}
