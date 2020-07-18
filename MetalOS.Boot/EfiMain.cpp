#include "EfiMain.h"

#include <efilib.h>
#include <intrin.h>
#include <crt_string.h>
#include <crt_wchar.h>
#include <crt_stdlib.h>
#include <stdlib.h>
#include <Path.h>
#include <PageTables.h>
#include <PageTablesPool.h>
#include "Device.h"
#include "EfiLoader.h"
#include "Error.h"
#include <crt_stdio.h>
#include "Uart.h"
#include <RamDrive.h>

#define EFI_DEBUG 1
#define Kernel L"moskrnl.exe"
#define KernelPDB L"moskrnl.pdb"
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
		//We need an initial memory map to know physical address space for PFN DB
		LoaderParams.MemoryMapSize *= 2;//Increase allocation size to leave room for additional allocations
		ReturnIfNotSuccess(BS->AllocatePool(EfiBootServicesData, LoaderParams.MemoryMapSize, (void**)&LoaderParams.MemoryMap));
		ReturnIfNotSuccess(BS->GetMemoryMap(&LoaderParams.MemoryMapSize, LoaderParams.MemoryMap, &memoryMapKey, &LoaderParams.MemoryMapDescriptorSize, &LoaderParams.MemoryMapDescriptorVersion));
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

	//Allocate space for PFN DB
	UINTN address = GetPhysicalAddressSize(LoaderParams.MemoryMapSize, LoaderParams.MemoryMapDescriptorSize, LoaderParams.MemoryMap);
	Print(L"Physical Address Max: 0x%016x\r\n", address);

	LoaderParams.PfnDbSize = sizeof(PFN_ENTRY) * (address >> PAGE_SHIFT);
	size_t pfnDBPages = EFI_SIZE_TO_PAGES(LoaderParams.PfnDbSize);
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, AllocationType, pfnDBPages, &(LoaderParams.PfnDbAddress)));

	//Allocate space for RamDrive
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, AllocationType, SIZE_TO_PAGES(RamDriveSize), &(LoaderParams.RamDriveAddress)));
	RamDrive drive((void*)LoaderParams.RamDriveAddress, RamDriveSize);

	//Load kernel path
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fileSystem = nullptr;
	ReturnIfNotSuccess(ST->BootServices->OpenProtocol(LoadedImage->DeviceHandle, &FileSystemProtocol, (void**)&fileSystem, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	EFI_FILE* CurrentDriveRoot = nullptr;
	ReturnIfNotSuccess(fileSystem->OpenVolume(fileSystem, &CurrentDriveRoot));
	//PrintDirectory(fileSystem, CurrentDriveRoot);

	//Populate
	PopulateDrive(drive, fileSystem, CurrentDriveRoot);
	UartPrint("RamDrive:\n");
	for (const RamDrive::Entry& entry : drive)
	{
		if (*entry.Name == '\0')
			break;

		UartPrint("  File: %s PageNumber: 0x%x Length: 0x%x\n", entry.Name, entry.PageNumber, entry.Length);
	}

	//Build path to kernel
	CHAR16* KernelPath;
	ReturnIfNotSuccess(BS->AllocatePool(AllocationType, MaxKernelPath * sizeof(CHAR16), (void**)&KernelPath));
	memset((void*)KernelPath, 0, MaxKernelPath * sizeof(CHAR16));
	GetDirectoryName(BootFilePath, KernelPath);
	wcscpy(KernelPath + wcslen(KernelPath), Kernel);

	EFI_FILE* KernelFile = nullptr;
	Print(L"Loading: %s\r\n", KernelPath);
	ReturnIfNotSuccess(CurrentDriveRoot->Open(CurrentDriveRoot, &KernelFile, KernelPath, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY));
	ReturnIfNotSuccess(BS->FreePool(KernelPath));

	//Map kernel into memory. It will be relocated at KernelBaseAddress
	UINT64 entryPoint;
	ReturnIfNotSuccess(EfiLoader::MapKernel(KernelFile, &LoaderParams.KernelImageSize, &entryPoint, &LoaderParams.KernelAddress));

	//Build path to kernelpdb
	CHAR16* KernelPdbPath;
	ReturnIfNotSuccess(BS->AllocatePool(AllocationType, MaxKernelPath * sizeof(CHAR16), (void**)&KernelPdbPath));
	memset((void*)KernelPdbPath, 0, MaxKernelPath * sizeof(CHAR16));
	GetDirectoryName(BootFilePath, KernelPdbPath);
	wcscpy(KernelPdbPath + wcslen(KernelPdbPath), KernelPDB);

	//Load pdb file
	EFI_FILE* KernelPdbFile = nullptr;
	Print(L"Loading: %s\r\n", KernelPdbPath);
	ReturnIfNotSuccess(CurrentDriveRoot->Open(CurrentDriveRoot, &KernelPdbFile, KernelPdbPath, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY));
	ReturnIfNotSuccess(BS->FreePool(KernelPdbPath));

	//Map PDB into memory
	ReturnIfNotSuccess(EfiLoader::MapFile(KernelPdbFile, LoaderParams.PdbAddress, LoaderParams.PdbSize));
	Print(L"  Address: 0x%016x Size:0x%x\r\n", LoaderParams.PdbAddress, LoaderParams.PdbSize);

	//Initialize graphics
	ReturnIfNotSuccess(InitializeGraphics(&LoaderParams.Display));

	//Disable write protection, allowing current page tables to be modified
	__writecr0(__readcr0() & ~(1 << 16));

	//Map in Kernel Space
	PageTablesPool pageTablesPool(bootloaderPagePoolAddress, BootloaderPagePoolCount);
	PageTables::Pool = &pageTablesPool;
	PageTables currentPT(__readcr3());
	currentPT.MapKernelPages(KernelBaseAddress, LoaderParams.KernelAddress, EFI_SIZE_TO_PAGES(LoaderParams.KernelImageSize));
	currentPT.MapKernelPages(KernelPageTablesPoolAddress, LoaderParams.PageTablesPoolAddress, LoaderParams.PageTablesPoolPageCount);
	currentPT.MapKernelPages(KernelGraphicsDeviceAddress, LoaderParams.Display.FrameBufferBase, EFI_SIZE_TO_PAGES(LoaderParams.Display.FrameBufferSize));

	//Re-enable write protection
	__writecr0(__readcr0() | (1 << 16));

	//Finish kernel image initialization now that address space is constructed
	ReturnIfNotSuccess(EfiLoader::CrtInitialization(KernelBaseAddress));

	//Display graphics
	ReturnIfNotSuccess(DisplayLoaderParams(&LoaderParams));
	//DumpGopLocations();

	//Pause here before booting kernel to inspect bootloader outputs
	Keywait();

	//Retrieve map from UEFI
	//This could fail on EFI_BUFFER_TOO_SMALL
	LoaderParams.MemoryMapSize *= 2;
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

	//Output final map to uart
	DumpMemoryMap();

	//Call into kernel
	KernelMain kernelMain = (KernelMain)(entryPoint);
	kernelMain(&LoaderParams);

	//Should never get here
	return EFI_ABORTED;
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

	ReturnIfNotSuccess(Print(L"CPU Vendor: %s,", wideVendor));

	UINT64 cr0 = __readcr0();
	int paging = (cr0 & ((UINT32)1 << 31)) != 0;
	ReturnIfNotSuccess(Print(L" Paging: %d\r\n", (UINT32)paging));

	return status;
}

EFI_STATUS DisplayLoaderParams(LOADER_PARAMS* pParams)
{
	EFI_STATUS status;

	ReturnIfNotSuccess(Print(L"LoaderParams: 0x%016x, Pages: 0x%x\r\n", pParams, EFI_SIZE_TO_PAGES(sizeof(LOADER_PARAMS))));
	ReturnIfNotSuccess(Print(L"  Kernel-Address: 0x%016x, Pages: 0x%x\r\n", pParams->KernelAddress, EFI_SIZE_TO_PAGES(pParams->KernelImageSize)));
	ReturnIfNotSuccess(Print(L"  MemoryMap-Address: 0x%016x, Pages: 0x%x\r\n", pParams->MemoryMap, EFI_SIZE_TO_PAGES(pParams->MemoryMapSize)));
	ReturnIfNotSuccess(Print(L"  PageTablesPool-Address: 0x%016x, Pages: 0x%x\r\n", pParams->PageTablesPoolAddress, pParams->PageTablesPoolPageCount));
	ReturnIfNotSuccess(Print(L"  PFN Database-Address: 0x%016x, Size: 0x%x\r\n", pParams->PfnDbAddress, pParams->PfnDbSize));
	ReturnIfNotSuccess(Print(L"  ConfigTables-Address: 0x%016x, Count: 0x%x\r\n", pParams->ConfigTables, pParams->ConfigTableSizes));
	ReturnIfNotSuccess(Print(L"  RamDrive-Address: 0x%016x, Size: 0x%x\r\n", pParams->RamDriveAddress, RamDriveSize));
	ReturnIfNotSuccess(Print(L"  PDB-Address: 0x%016x, Size: 0x%x\r\n", pParams->PdbAddress, pParams->PdbSize));

	PrintGraphicsDevice(&pParams->Display);
	return status;
}

EFI_STATUS PrintGraphicsDevice(PEFI_GRAPHICS_DEVICE pDevice)
{
	EFI_STATUS status;

	ReturnIfNotSuccess(Print(L"Graphics:\r\n"));
	ReturnIfNotSuccess(Print(L"  FrameBuffer-Base 0x%016x, Size: 0x%08x\r\n", pDevice->FrameBufferBase, pDevice->FrameBufferSize));
	ReturnIfNotSuccess(Print(L"  Resulution 0x%04x (0x%04x) x 0x%04x\r\n", pDevice->HorizontalResolution, pDevice->PixelsPerScanLine, pDevice->VerticalResolution));

	return status;
}

EFI_STATUS DumpMemoryMap()
{
	const char mem_types[16][27] = {
	  "EfiReservedMemoryType     ",
	  "EfiLoaderCode             ",
	  "EfiLoaderData             ",
	  "EfiBootServicesCode       ",
	  "EfiBootServicesData       ",
	  "EfiRuntimeServicesCode    ",
	  "EfiRuntimeServicesData    ",
	  "EfiConventionalMemory     ",
	  "EfiUnusableMemory         ",
	  "EfiACPIReclaimMemory      ",
	  "EfiACPIMemoryNVS          ",
	  "EfiMemoryMappedIO         ",
	  "EfiMemoryMappedIOPortSpace",
	  "EfiPalCode                ",
	  "EfiPersistentMemory       ",
	  "EfiMaxMemoryType          "
	};

	UartPrint("MapSize: 0x%x, DescSize: 0x%x\n", LoaderParams.MemoryMap, LoaderParams.MemoryMapDescriptorSize);

	EFI_MEMORY_DESCRIPTOR* current;
	for (current = LoaderParams.MemoryMap;
		current < NextMemoryDescriptor(LoaderParams.MemoryMap, LoaderParams.MemoryMapSize);
		current = NextMemoryDescriptor(current, LoaderParams.MemoryMapDescriptorSize))
	{
		const bool runtime = (current->Attribute & EFI_MEMORY_RUNTIME) != 0;
		UartPrint("P: %016x V: %016x T:%s #: 0x%x A:0x%016x %c\n", current->PhysicalStart, current->VirtualStart, mem_types[current->Type], current->NumberOfPages, current->Attribute, runtime ? 'R' : ' ');
	}

	return EFI_SUCCESS;
}

EFI_STATUS PrintDirectory(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs, EFI_FILE* dir)
{
	EFI_STATUS status;

	const size_t MAX_FILE_INFO_SIZE = 1024;
	uint8_t buffer[MAX_FILE_INFO_SIZE] = { 0 };
	EFI_FILE_INFO* fileInfo = (EFI_FILE_INFO*)buffer;

	while (true)
	{
		UINTN size = MAX_FILE_INFO_SIZE;
		ReturnIfNotSuccess(status = dir->Read(dir, &size, buffer));
		if (size == 0)
			break; //No more directories

		if (wcscmp(fileInfo->FileName, L".") == 0 || wcscmp(fileInfo->FileName, L"..") == 0)
		{
			continue;
		}

		if (fileInfo->Attribute & EFI_FILE_DIRECTORY)
		{
			UartPrint(L"Dir: %s\n", fileInfo->FileName);

			//recurse
			EFI_FILE_HANDLE subDir;
			ReturnIfNotSuccess(dir->Open(dir, &subDir, fileInfo->FileName, EFI_FILE_MODE_READ, 0));
			subDir->SetPosition(subDir, 0);

			PrintDirectory(fs, subDir);

			dir->Close(subDir);
		}
		else
			UartPrint(L"File: %s\n", fileInfo->FileName);
	}

	return status;
}

EFI_STATUS PopulateDrive(RamDrive& drive, EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs, EFI_FILE* dir)
{
	EFI_STATUS status;

	const size_t MAX_FILE_INFO_SIZE = 1024;
	uint8_t buffer[MAX_FILE_INFO_SIZE] = { 0 };
	EFI_FILE_INFO* fileInfo = (EFI_FILE_INFO*)buffer;

	dir->SetPosition(dir, 0);

	while (true)
	{
		UINTN size = MAX_FILE_INFO_SIZE;
		ReturnIfNotSuccess(status = dir->Read(dir, &size, buffer));
		if (size == 0)
			break; //No more directories

		if (wcscmp(fileInfo->FileName, L".") == 0 || wcscmp(fileInfo->FileName, L"..") == 0)
		{
			continue;
		}

		if (fileInfo->Attribute & EFI_FILE_DIRECTORY)
		{
			//recurse
			EFI_FILE_HANDLE subDir;
			ReturnIfNotSuccess(dir->Open(dir, &subDir, fileInfo->FileName, EFI_FILE_MODE_READ, 0));
			subDir->SetPosition(subDir, 0);

			PopulateDrive(drive, fs, subDir);

			dir->Close(subDir);
		}
		else
		{
			//Convert to asci
			char buffer[32] = { 0 };
			wcstombs(buffer, fileInfo->FileName, sizeof(buffer));

			//Allocate in ram
			void* address = drive.Allocate(buffer, fileInfo->FileSize);

			//Open
			EFI_FILE_HANDLE file;
			ReturnIfNotSuccess(dir->Open(dir, &file, fileInfo->FileName, EFI_FILE_MODE_READ, 0));

			//Copy
			ReturnIfNotSuccess(file->Read(file, &fileInfo->FileSize, (void*)address));
		}
	}

	return status;
}

void Print(const char* format, ...)
{
	Uart uart(ComPort::Com1);

	va_list args;
	va_start(args, format);
	uart.Printf(format, args);
	va_end(args);
}

void Print(const char* format, va_list args)
{
	Uart uart(ComPort::Com1);

	uart.Printf(format, args);
}

#define MAXBUFFER 255
EFI_STATUS Print(const CHAR16* format, ...)
{
	va_list args;
	va_start(args, format);

	EFI_STATUS status;
	CHAR16 buffer[MAXBUFFER] = { 0 };
	vwprintf(buffer, format, args);
	va_end(args);

	//Write
	if (EFI_ERROR((status = ST->ConOut->OutputString(ST->ConOut, buffer))))
		return status;

	return EFI_SUCCESS;
}

void UartPrint(const char* format, ...)
{
	Uart uart(ComPort::Com1);

	va_list args;
	va_start(args, format);
	uart.Printf(format, args);
	va_end(args);
}

void UartPrint(const CHAR16* format, ...)
{
	Uart uart(ComPort::Com1);

	va_list args;
	va_start(args, format);
	CHAR16 wideBuffer[MAXBUFFER] = { 0 };
	vwprintf(wideBuffer, format, args);
	va_end(args);

	char buffer[MAXBUFFER / 2] = { 0 };
	wcstombs(buffer, wideBuffer, sizeof(buffer));
	uart.Printf(buffer);
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
