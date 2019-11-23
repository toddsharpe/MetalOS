#include "EfiMain.h"
#include "BootLoader.h"
#include "EfiPrint.h"
#include "String.h"
#include "Device.h"
#include "EfiLoader.h"
#include "Error.h"
#include "Memory.h"
//#define _JMP_BUF_DEFINED
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

extern "C" EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_STATUS status;

	ST = SystemTable;
	BS = SystemTable->BootServices;
	RT = SystemTable->RuntimeServices;

	//Disable the stupid watchdog
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

	//Reserve space for loader block
	LOADER_PARAMS* params = nullptr;
	ReturnIfNotSuccess(BS->AllocatePool(AllocationType, sizeof(LOADER_PARAMS), (void**)&params));
	CRT::memset(params, 0, sizeof(LOADER_PARAMS));
	params->ConfigTables = ST->ConfigurationTable;
	params->ConfigTableSizes = ST->NumberOfTableEntries;

	//Allocate a page for the memory map
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, (EFI_PHYSICAL_ADDRESS*)&(params->MemoryMap)));
	Print(L"MemoryMap: %q\r\n", params->MemoryMap);

	//Allocate pages for our PageTablesPool
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, EfiLoaderData, ReservedPageTablePages, &(params->PageTablesPoolAddress)));
	Print(L"PageTablesPoolAddress: %q\r\n", params->PageTablesPoolAddress);
	params->PageTablesPoolPageCount = ReservedPageTablePages;

	//Initialize the page tables pool and allocate a root page
	PageTablesPool pageTablesPool(params->PageTablesPoolAddress, params->PageTablesPoolPageCount);
	EFI_PHYSICAL_ADDRESS ptsRoot;
	if (!pageTablesPool.AllocatePage(&ptsRoot))
	{
		ReturnIfNotSuccess(EFI_LOAD_ERROR);
	}

	//Copy current PT root to our new root
	CRT::memcpy((void*)ptsRoot, (void*)__readcr3(), EFI_PAGE_SIZE);

	//Make a new PT from this root and activate it. We do this because original PT is read-only to us.
	//We only need to map the kernel, then the kernel can arrange things in a clean PT.
	PageTables newPts(ptsRoot);
	newPts.SetPool(&pageTablesPool);
	__writecr3(ptsRoot);

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

	//Map kernel into memory
	UINT64 entryPoint;
	EFI_PHYSICAL_ADDRESS physicalImageBase;
	ReturnIfNotSuccess(EfiLoader::MapKernel(KernelFile, &params->BaseAddress, &params->ImageSize, &entryPoint, &physicalImageBase));

	//Map kernel into address space
	newPts.MapKernelPages(params->BaseAddress, physicalImageBase, EFI_SIZE_TO_PAGES(params->ImageSize));

	//Technically everything after allocating the loader block needs to free that memory before dying
	ReturnIfNotSuccess(InitializeGraphics(&params->Display));
	ReturnIfNotSuccess(DisplayLoaderParams(params));
	Keywait(L"Boot kernel?\r\n");

	//Determine size of map
	UINTN memoryMapKey = 0;
	status = BS->GetMemoryMap(&params->MemoryMapSize, params->MemoryMap, &memoryMapKey, &params->MemoryMapDescriptorSize, &params->MemoryMapVersion);
	if (status != EFI_BUFFER_TOO_SMALL)
	{
		ReturnIfNotSuccess(status);
	}
	else if (params->MemoryMapSize > EFI_PAGE_SIZE)
	{
		//We cant allocate a page after getting the memory map size (since that allocates a page) so instead we predict a page and make sure it isnt too small
		//We could do fancy math to know this ahead of time (if its within MemoryMapDescriptorSize or being over a page, etc)
		ReturnIfNotSuccess(EFI_BUFFER_TOO_SMALL);
	}

	//TODO: Free memory allocated above if this second call fails
	ReturnIfNotSuccess(BS->GetMemoryMap(&params->MemoryMapSize, params->MemoryMap, &memoryMapKey, &params->MemoryMapDescriptorSize, &params->MemoryMapVersion));
	//Get latest memory map, exit boot services
	ReturnIfNotSuccess(BS->ExitBootServices(ImageHandle, memoryMapKey));

	//Call into kernel
	KernelMain kernelMain = (KernelMain)(entryPoint);
	kernelMain(params);

	//Should never get here
	return EFI_ABORTED;
}

EFI_STATUS DisplayLoaderParams(LOADER_PARAMS* params)
{
	EFI_STATUS status;
	
	ReturnIfNotSuccess(Print(L"DisplayLoaderParams:\r\n"));
	ReturnIfNotSuccess(Print(L"\tBaseAddress: %q\r\n", params->BaseAddress));
	ReturnIfNotSuccess(Print(L"\tConfigTables: %q\r\n", params->ConfigTables));
	ReturnIfNotSuccess(Print(L"\tConfigTableSizes: %d\r\n", params->ConfigTableSizes));

	PrintGopMode(&params->Display);
	return status;
}

EFI_STATUS Keywait(const CHAR16* String)
{
	EFI_STATUS status;
	EFI_INPUT_KEY Key;

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
