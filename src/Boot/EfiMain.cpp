#include "EfiMain.h"

#include <efilib.h>
#include <intrin.h>
#include <stdlib.h>
#include "Device.h"
#include "EfiLoader.h"
#include "Error.h"
#include "Uart.h"
#include "LoaderParams.h"
#include "RamDrive.h"
#include "Path.h"
#include "PageTablesPool.h"
#include "PageTables.h"
#include <stdlib.h>
#include "Output.h"
#include <Assert.h>
#include "Cpuid.h"
#include <MetalOS.System.h>
#include <MetalOS.Internal.h>

static constexpr wchar_t Kernel[] = L"moskrnl.exe";
static constexpr wchar_t KernelPDB[] = L"moskrnl.pdb";
static constexpr size_t MaxKernelPath = 64;
static constexpr size_t BootloaderPagePoolCount = 256; //1Mb
static constexpr size_t ReservedPageTablePages = 512; //2Mb

EFI_SYSTEM_TABLE* ST;
EFI_RUNTIME_SERVICES* RT;
EFI_BOOT_SERVICES* BS;
EFI_MEMORY_TYPE AllocationType;

extern "C" EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	static_assert(PageShift == EFI_PAGE_SHIFT, "PageShift mismatch");
	static_assert(PageSize == EFI_PAGE_SIZE, "PageSize mismatch");

	//Save UEFI environment
	ST = SystemTable;
	BS = SystemTable->BootServices;
	RT = SystemTable->RuntimeServices;

	//Boot Params for kernel
	LoaderParams params;
	params.ConfigTables = ST->ConfigurationTable;
	params.ConfigTablesCount = ST->NumberOfTableEntries;
	params.Runtime = RT;

	EFI_STATUS status;
	ReturnIfNotSuccess(ST->ConOut->ClearScreen(ST->ConOut));

	//Get handle to bootloader.
	EFI_LOADED_IMAGE* LoadedImage = nullptr;
	ReturnIfNotSuccess(BS->OpenProtocol(ImageHandle, &LoadedImageProtocol, (void**)&LoadedImage, NULL, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	AllocationType = LoadedImage->ImageDataType;
	const CHAR16* BootFilePath = ((FILEPATH_DEVICE_PATH*)LoadedImage->FilePath)->PathName;

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
	UINT32 descriptorVersion = 0;
	status = BS->GetMemoryMap(&params.MemoryMap.Size, params.MemoryMap.Table, &memoryMapKey, &params.MemoryMap.DescriptorSize, &descriptorVersion);
	if (status != EFI_BUFFER_TOO_SMALL)
		ReturnIfNotSuccess(status);

	//We need an initial memory map to know physical address space for PFN DB
	params.MemoryMap.Size *= 2;//Increase allocation size to leave room for additional allocations
	ReturnIfNotSuccess(BS->AllocatePool(EfiBootServicesData, params.MemoryMap.Size, (void**)&params.MemoryMap.Table));
	ReturnIfNotSuccess(BS->GetMemoryMap(&params.MemoryMap.Size, params.MemoryMap.Table, &memoryMapKey, &params.MemoryMap.DescriptorSize, &descriptorVersion));
	DumpMemoryMap(params.MemoryMap);

	//Allocate pages for Bootloader PageTablesPool in BootServicesData
	//Pages from this pool will be used to bootstrap the kernel
	//Boot PT is allocated in boot services data so kernel knows it can be cleared
	EFI_PHYSICAL_ADDRESS bootloaderPagePoolAddress;
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, EfiBootServicesData, BootloaderPagePoolCount, &bootloaderPagePoolAddress));

	//Allocate pages for our Kernel's page pools
	//This pool will be kept for the kernel to use during runtime
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, AllocationType, ReservedPageTablePages, &params.PageTablesPoolAddress));
	params.PageTablesPoolPageCount = ReservedPageTablePages;

	//Allocate space for Page Frame DB
	const UINTN address = GetPhysicalAddressSize(params.MemoryMap);
	Print(L"Physical Address Max: 0x%016x\r\n", address);

	const size_t pageCount = address >> PageShift;
	params.PageFrameCount = pageCount;
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, AllocationType, SizeToPages(pageCount * sizeof(PageFrame)), &params.PageFrameAddr));

	//Allocate space for RamDrive
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, AllocationType, SizeToPages(RamDriveSize), &params.RamDriveAddress));
	RamDrive drive((void*)params.RamDriveAddress, RamDriveSize);

	//Load kernel path
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fileSystem = nullptr;
	ReturnIfNotSuccess(ST->BootServices->OpenProtocol(LoadedImage->DeviceHandle, &FileSystemProtocol, (void**)&fileSystem, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	EFI_FILE* CurrentDriveRoot = nullptr;
	ReturnIfNotSuccess(fileSystem->OpenVolume(fileSystem, &CurrentDriveRoot));
	//PrintDirectory(fileSystem, CurrentDriveRoot);

	//Populate
	PopulateDrive(drive, fileSystem, CurrentDriveRoot);
	UartPrintf(L"RamDrive:\n");
	for (const RamDrive::Entry& entry : drive)
	{
		if (*entry.Name == '\0')
			break;

		CHAR16 buffer[RamDrive::EntrySize] = {};
		mbstowcs(buffer, entry.Name, RamDrive::EntrySize);

		UartPrintf(L"  File: %s PageNumber: 0x%x Length: 0x%x\n", buffer, entry.PageNumber, entry.Length);
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
	ReturnIfNotSuccess(EfiLoader::MapKernel(KernelFile, params.KernelImageSize, entryPoint, params.KernelAddress));

	LoaderParams* kParams = (LoaderParams*)EfiLoader::GetProcAddress((void*)params.KernelAddress, "BootParams");
	Assert(kParams);
	Print(L"  Params: 0x%016x\r\n", kParams);

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
	ReturnIfNotSuccess(EfiLoader::MapFile(KernelPdbFile, params.PdbAddress, params.PdbSize));
	Print(L"  Address: 0x%016x Size:0x%x\r\n", params.PdbAddress, params.PdbSize);

	//Initialize graphics
	ReturnIfNotSuccess(InitializeGraphics(params.Display));

	//Disable write protection, allowing current page tables to be modified
	__writecr0(__readcr0() & ~(1 << 16));

	//Map in Kernel Space
	PageTablesPool pageTablesPool((void*)bootloaderPagePoolAddress, bootloaderPagePoolAddress, BootloaderPagePoolCount);
	pageTablesPool.Initialize();
	PageTables::Pool = &pageTablesPool;

	PageTables currentPT;
	currentPT.OpenCurrent();
	
	//Some platforms have Kernel addresses already mapped (B550i/5950x). Clear them.
	//The physical page tables will be reclaimed by Kernel, so just remove pointers.
	currentPT.ClearKernelEntries();
	Assert(currentPT.MapPages(KernelBaseAddress, params.KernelAddress, SizeToPages(params.KernelImageSize), true));
	Assert(currentPT.MapPages(KernelPageTablesPool, params.PageTablesPoolAddress, params.PageTablesPoolPageCount, true));
	Assert(currentPT.MapPages(KernelGraphicsDevice, params.Display.FrameBufferBase, SizeToPages(params.Display.FrameBufferSize), true));
	Assert(currentPT.MapPages(KernelPageFrameDBStart, params.PageFrameAddr, SizeToPages(params.PageFrameCount * sizeof(PageFrame)), true));

	//Re-enable write protection
	__writecr0(__readcr0() | (1 << 16));

	//Display graphics
	ReturnIfNotSuccess(DisplayLoaderParams(params));

	//Pause here before booting kernel to inspect bootloader outputs
	Keywait();

	//Retrieve map from UEFI
	//This could fail on EFI_BUFFER_TOO_SMALL
	params.MemoryMap.Size *= 2;
	ReturnIfNotSuccess(BS->GetMemoryMap(&params.MemoryMap.Size, params.MemoryMap.Table, &memoryMapKey, &params.MemoryMap.DescriptorSize, &descriptorVersion));

	//After ExitBootServices we can no longer use the BS handle (no print, memory, etc)
	ReturnIfNotSuccess(BS->ExitBootServices(ImageHandle, memoryMapKey));

	//Assign virtual mappings for runtime sections
	for (EFI_MEMORY_DESCRIPTOR* current = params.MemoryMap.Table;
		current < NextMemoryDescriptor(params.MemoryMap.Table, params.MemoryMap.Size);
		current = NextMemoryDescriptor(current, params.MemoryMap.DescriptorSize))
	{
		if ((current->Attribute & EFI_MEMORY_RUNTIME) == 0)
			continue;

		current->VirtualStart = current->PhysicalStart + KernelUefiStart;
	}

	//Update UEFI virtual address map
	ReturnIfNotSuccessNoDisplay(RT->SetVirtualAddressMap(params.MemoryMap.Size, params.MemoryMap.DescriptorSize, descriptorVersion, params.MemoryMap.Table));

	//Output final map to uart
	DumpMemoryMap(params.MemoryMap);

	//Call into kernel
	const KernelMain kernelMain = (KernelMain)(entryPoint);
	*kParams = params;

	//Finish kernel image initialization now that address space is constructed
	ReturnIfNotSuccess(EfiLoader::CrtInitialization(KernelBaseAddress));
	kernelMain();

	//Should never get here
	return EFI_ABORTED;
}

EFI_STATUS PrintCpuDetails()
{
	EFI_STATUS status;

	std::string vendor;
	Cpuid::GetVendor(vendor);
	CHAR16 wideVendor[13] = { };
	mbstowcs(wideVendor, vendor.c_str(), sizeof(wideVendor) / sizeof(CHAR16));

	ReturnIfNotSuccess(Print(L"CPU Vendor: %s,", wideVendor));

	UINT64 cr0 = __readcr0();
	int paging = (cr0 & ((UINT32)1 << 31)) != 0;
	ReturnIfNotSuccess(Print(L" Paging: %d\r\n", (UINT32)paging));

	return status;
}

EFI_STATUS DisplayLoaderParams(const LoaderParams& params)
{
	EFI_STATUS status;

	ReturnIfNotSuccess(Print(L"LoaderParams: 0x%016x, Pages: 0x%x\r\n", &params, SizeToPages(sizeof(LoaderParams))));
	ReturnIfNotSuccess(Print(L"  Kernel-Address: 0x%016x, Pages: 0x%x\r\n", params.KernelAddress, SizeToPages(params.KernelImageSize)));
	ReturnIfNotSuccess(Print(L"  MemoryMap-Address: 0x%016x, Pages: 0x%x\r\n", params.MemoryMap.Table, SizeToPages(params.MemoryMap.Size)));
	ReturnIfNotSuccess(Print(L"  PageTablesPool-Address: 0x%016x, Pages: 0x%x\r\n", params.PageTablesPoolAddress, params.PageTablesPoolPageCount));
	ReturnIfNotSuccess(Print(L"  PFN Database-Address: 0x%016x, Count: 0x%x\r\n", params.PageFrameAddr, params.PageFrameCount));
	ReturnIfNotSuccess(Print(L"  ConfigTables-Address: 0x%016x, Count: 0x%x\r\n", params.ConfigTables, params.ConfigTablesCount));
	ReturnIfNotSuccess(Print(L"  RamDrive-Address: 0x%016x, Size: 0x%x\r\n", params.RamDriveAddress, RamDriveSize));
	ReturnIfNotSuccess(Print(L"  PDB-Address: 0x%016x, Size: 0x%x\r\n", params.PdbAddress, params.PdbSize));

	ReturnIfNotSuccess(Print(L"Graphics:\r\n"));
	ReturnIfNotSuccess(Print(L"  FrameBuffer-Base 0x%016x, Size: 0x%08x\r\n", params.Display.FrameBufferBase, params.Display.FrameBufferSize));
	ReturnIfNotSuccess(Print(L"  Resulution 0x%04x (0x%04x) x 0x%04x\r\n", params.Display.HorizontalResolution, params.Display.PixelsPerScanLine, params.Display.VerticalResolution));
	return status;
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
			UartPrintf(L"Dir: %s\n", fileInfo->FileName);

			//recurse
			EFI_FILE_HANDLE subDir;
			ReturnIfNotSuccess(dir->Open(dir, &subDir, fileInfo->FileName, EFI_FILE_MODE_READ, 0));
			subDir->SetPosition(subDir, 0);

			PrintDirectory(fs, subDir);

			dir->Close(subDir);
		}
		else
			UartPrintf(L"File: %s\n", fileInfo->FileName);
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
			continue;

		if (fileInfo->Attribute & EFI_FILE_DIRECTORY)
		{
			//Recurse
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

UINTN GetPhysicalAddressSize(const EFI_MEMORY_MAP& map)
{
	UINTN highest = 0;

	for (const EFI_MEMORY_DESCRIPTOR* current = map.Table;
		current < NextMemoryDescriptor(map.Table, map.Size);
		current = NextMemoryDescriptor(current, map.DescriptorSize))
	{
		const uintptr_t address = current->PhysicalStart + (current->NumberOfPages << PageShift);
		if (address > highest)
			highest = address;
	}

	return highest;
}

EFI_STATUS DumpMemoryMap(const EFI_MEMORY_MAP& map)
{
	const CHAR16 mem_types[16][27] = {
	  L"EfiReservedMemoryType     ",
	  L"EfiLoaderCode             ",
	  L"EfiLoaderData             ",
	  L"EfiBootServicesCode       ",
	  L"EfiBootServicesData       ",
	  L"EfiRuntimeServicesCode    ",
	  L"EfiRuntimeServicesData    ",
	  L"EfiConventionalMemory     ",
	  L"EfiUnusableMemory         ",
	  L"EfiACPIReclaimMemory      ",
	  L"EfiACPIMemoryNVS          ",
	  L"EfiMemoryMappedIO         ",
	  L"EfiMemoryMappedIOPortSpace",
	  L"EfiPalCode                ",
	  L"EfiPersistentMemory       ",
	  L"EfiMaxMemoryType          "
	};

	UartPrintf(L"MapSize: 0x%016X, Size: 0x%x, DescSize: 0x%x\n", map.Table, map.Size, map.DescriptorSize);

	for (EFI_MEMORY_DESCRIPTOR* current = map.Table; current < NextMemoryDescriptor(map.Table, map.Size); current = NextMemoryDescriptor(current, map.DescriptorSize))
	{
		const bool runtime = (current->Attribute & EFI_MEMORY_RUNTIME) != 0;
		UartPrintf(L"P: %016x V: %016x T:%s #: 0x%x A:0x%016x %c\n", current->PhysicalStart, current->VirtualStart, mem_types[current->Type], current->NumberOfPages, current->Attribute, runtime ? 'R' : ' ');
	}

	return EFI_SUCCESS;
}
