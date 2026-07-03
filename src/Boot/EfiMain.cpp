//Include main header
#include "Boot/EfiMain.h"

//Core crt
#include "core_crt/core_crt.c"

//Architecture specific
#include "x64/CpuId.cpp"

//Include sources
#include "Boot/EfiDevice.cpp"
#include "Boot/EfiDump.cpp"
#include "Boot/EfiError.cpp"
#include "Boot/EfiLoader.cpp"
#include "Boot/EfiMemory.cpp"
#include "Boot/EfiPrint.cpp"

static constexpr wchar_t Kernel[] = L"moskrnl.exe";
static constexpr wchar_t KernelPDB[] = L"moskrnl.pdb";
static constexpr size_t MaxKernelPath = 64;

EFI_SYSTEM_TABLE* ST;
EFI_RUNTIME_SERVICES* RT;
EFI_BOOT_SERVICES* BS;

//EFI protocol GUIDs (declared extern in Efi.h)
EFI_GUID gEfiLoadedImageProtocolGuid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
EFI_GUID gEfiSimpleFileSystemProtocolGuid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
EFI_GUID gEfiGraphicsOutputProtocolGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
EFI_GUID gEfiFileInfoGuid = EFI_FILE_INFO_ID;

static_assert(x64::PageShift == EFI_PAGE_SHIFT, "PageShift mismatch");
static_assert(x64::PageSize == EFI_PAGE_SIZE, "PageSize mismatch");

static constexpr PageTablesOps PtOps =
{
	.Resolve = [](const paddr_t address) -> void*
	{
		//UEFI uses identity paging
		return reinterpret_cast<void*>(address);
	},
	.PhyAlloc = [](paddr_t &address) -> bool
	{
		//EfiLoaderData is how kernel image is mapped
		EFI_PHYSICAL_ADDRESS addr = {};
		if (EFI_ERROR(BS->AllocatePages(AllocateAnyPages, EfiLoaderData, 1, &addr)))
			return false;
		address = static_cast<paddr_t>(addr);

		//Zero out the page
		memset(reinterpret_cast<void*>(address), 0, x64::PageSize);
		return true;
	}
};
using BootloaderPageTables = PageTabes<PtOps>;

static EFI_STATUS PrintCpuDetails()
{
	CHAR16 wideVendor[13] = { };
	const char* vendor = CpuId::GetVendor();
	mbstowcs(wideVendor, vendor, sizeof(wideVendor) / sizeof(CHAR16));

	ReturnIfNotSuccess(Print(L"CPU Vendor: %s,", wideVendor));

	const UINT64 cr0 = __readcr0();
	const int paging = (cr0 & ((UINT32)1 << 31)) != 0;
	ReturnIfNotSuccess(Print(L" Paging: %d\r\n", (UINT32)paging));

	return EFI_SUCCESS;
}

static EFI_STATUS PopulateDrive(RamDrive& drive, EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs, EFI_FILE* dir)
{
	EFI_STATUS status;

	const size_t MAX_FILE_INFO_SIZE = 1024;
	uint8_t buffer[MAX_FILE_INFO_SIZE] = { 0 };
	EFI_FILE_INFO* fileInfo = (EFI_FILE_INFO*)buffer;

	dir->SetPosition(dir, 0);

	while (true)
	{
		UINTN size = MAX_FILE_INFO_SIZE;
		ReturnIfNotSuccess(dir->Read(dir, &size, buffer));
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
			char buffer[32] = {};
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

	return EFI_SUCCESS;
}

static EFI_STATUS DisplayLoaderParams(const LoaderParams& params)
{
	ReturnIfNotSuccess(Print(L"LoaderParams: 0x%016x, Pages: 0x%x\r\n", &params, x64::SizeToPages(sizeof(LoaderParams))));
	ReturnIfNotSuccess(Print(L"  Kernel-Address: 0x%016x, Pages: 0x%x\r\n", params.KernelAddress, x64::SizeToPages(params.KernelImageSize)));
	ReturnIfNotSuccess(Print(L"  FreeRanges: 0x%016x, RuntimeRanges: 0x%016x\r\n", params.FreeRanges, params.RuntimeRanges));
	ReturnIfNotSuccess(Print(L"  PFN Database-Address: 0x%016x, Count: 0x%x\r\n", params.PageFrameAddr, params.PageFrameCount));
	ReturnIfNotSuccess(Print(L"  ConfigTables-Address: 0x%016x, Count: 0x%x\r\n", params.ConfigTables, params.ConfigTablesCount));
	ReturnIfNotSuccess(Print(L"  RamDrive-Address: 0x%016x, Size: 0x%x\r\n", params.RamDriveAddress, RamDriveSize));
	ReturnIfNotSuccess(Print(L"  PDB-Address: 0x%016x, Size: 0x%x\r\n", params.PdbAddress, params.PdbSize));

	ReturnIfNotSuccess(Print(L"Graphics:\r\n"));
	ReturnIfNotSuccess(Print(L"  FrameBuffer-Base 0x%016x, Size: 0x%08x\r\n", params.Display.FrameBufferBase, params.Display.FrameBufferSize));
	ReturnIfNotSuccess(Print(L"  Resulution 0x%04x (0x%04x) x 0x%04x\r\n", params.Display.HorizontalResolution, params.Display.PixelsPerScanLine, params.Display.VerticalResolution));
	
	return EFI_SUCCESS;
}

extern "C" EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{

	//Save UEFI environment
	ST = SystemTable;
	BS = SystemTable->BootServices;
	RT = SystemTable->RuntimeServices;

	//Boot Params for kernel
	LoaderParams params = {};
	params.ConfigTables = ST->ConfigurationTable;
	params.ConfigTablesCount = ST->NumberOfTableEntries;
	params.Runtime = RT;

	EFI_STATUS status;
	ReturnIfNotSuccess(ST->ConOut->ClearScreen(ST->ConOut));

	//Get handle to bootloader.
	EFI_LOADED_IMAGE* LoadedImage = nullptr;
	ReturnIfNotSuccess(BS->OpenProtocol(ImageHandle, &LoadedImageProtocol, (void**)&LoadedImage, NULL, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	const CHAR16* BootFilePath = ((FILEPATH_DEVICE_PATH*)LoadedImage->FilePath)->PathName;

	//Display some splash info
	ReturnIfNotSuccess(Print(L"MetalOS.BootLoader\r\n"));
	ReturnIfNotSuccess(Print(L"  Firmware Vendor: %s, Revision: %d\r\n", ST->FirmwareVendor, ST->FirmwareRevision));
	ReturnIfNotSuccess(Print(L"  Bootloader: %s\r\n", BootFilePath));
	ReturnIfNotSuccess(Print(L"  ImageHandle: 0x%016x\r\n", ImageHandle));
	ReturnIfNotSuccess(PrintCpuDetails());

	//Display time
	EFI_TIME time = {};
	ReturnIfNotSuccess(RT->GetTime(&time, nullptr));
	Print(L"Date: %02d-%02d-%02d %02d:%02d:%02d\r\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

	//Read the memory map. Needed up front to size the PFN DB.
	EFI_MEMORY_MAP map = {};
	UINTN memoryMapKey = 0;
	UINT32 descriptorVersion = 0;
	ReturnIfNotSuccess(EfiMemory::GetMemoryMap(map, memoryMapKey, descriptorVersion));

	//Allocate space for Page Frame DB
	const UINTN address = EfiMemory::GetPhysicalAddressSize(map);
	Print(L"Physical Address Max: 0x%016x\r\n", address);

	const size_t pageCount = address >> x64::PageShift;
	params.PageFrameCount = pageCount;
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, EfiLoaderData, x64::SizeToPages(pageCount * sizeof(PageFrame)), &params.PageFrameAddr));

	//Allocate space for RamDrive
	ReturnIfNotSuccess(BS->AllocatePages(AllocateAnyPages, EfiLoaderData, x64::SizeToPages(RamDriveSize), &params.RamDriveAddress));
	RamDrive drive((void*)params.RamDriveAddress, RamDriveSize);

	//Load kernel path
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fileSystem = nullptr;
	ReturnIfNotSuccess(ST->BootServices->OpenProtocol(LoadedImage->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&fileSystem, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	EFI_FILE* CurrentDriveRoot = nullptr;
	ReturnIfNotSuccess(fileSystem->OpenVolume(fileSystem, &CurrentDriveRoot));

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
	CHAR16 KernelPath[MaxKernelPath] = {};
	GetDirectoryName(BootFilePath, KernelPath);
	wcscpy(KernelPath + wcslen(KernelPath), Kernel);

	EFI_FILE* KernelFile = nullptr;
	Print(L"Loading: %s\r\n", KernelPath);
	ReturnIfNotSuccess(CurrentDriveRoot->Open(CurrentDriveRoot, &KernelFile, KernelPath, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY));

	//Map kernel into memory. It will be relocated at KernelBase
	UINT64 entryPoint;
	ReturnIfNotSuccess(EfiLoader::MapKernel(KernelFile, params.KernelImageSize, entryPoint, params.KernelAddress));

	LoaderParams* kParams = (LoaderParams*)EfiLoader::GetProcAddress((void*)params.KernelAddress, "BootParams");
	Assert(kParams);
	Print(L"  Params: 0x%016x\r\n", kParams);

	//Build path to kernelpdb
	CHAR16 KernelPdbPath[MaxKernelPath] = {};
	GetDirectoryName(BootFilePath, KernelPdbPath);
	wcscpy(KernelPdbPath + wcslen(KernelPdbPath), KernelPDB);

	//Load pdb file
	EFI_FILE* KernelPdbFile = nullptr;
	Print(L"Loading: %s\r\n", KernelPdbPath);
	ReturnIfNotSuccess(CurrentDriveRoot->Open(CurrentDriveRoot, &KernelPdbFile, KernelPdbPath, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY));

	//Map PDB into memory
	ReturnIfNotSuccess(EfiLoader::MapFile(KernelPdbFile, EfiLoaderData, params.PdbAddress, params.PdbSize));
	Print(L"  Address: 0x%016x Size:0x%x\r\n", params.PdbAddress, params.PdbSize);

	//Initialize graphics
	ReturnIfNotSuccess(EfiDevice::InitializeGraphics(params.Display));

	//Disable write protection, allowing current page tables to be modified
	__writecr0(__readcr0() & ~(1 << 16));

	//Map in Kernel Space
	BootloaderPageTables tables = BootloaderPageTables::Current();
	
	//Some platforms have Kernel addresses already mapped (B550i/5950x). Clear them.
	//The physical page tables will be reclaimed by Kernel, so just remove pointers.
	tables.ClearKernelEntries();
	Assert(tables.MapPages(KernelBase, params.KernelAddress, x64::SizeToPages(params.KernelImageSize), KernelAll));
	Assert(tables.MapPages(KernelGraphicsDevice, params.Display.FrameBufferBase, x64::SizeToPages(params.Display.FrameBufferSize), KernelAll));
	Assert(tables.MapPages(KernelPageFrameDBStart, params.PageFrameAddr, x64::SizeToPages(params.PageFrameCount * sizeof(PageFrame)), KernelAll));
	Assert(tables.MapPages(KernelPhysicalStart, 0, pageCount, KernelAll));

	//Re-enable write protection
	__writecr0(__readcr0() | (1 << 16));

	//Display graphics
	ReturnIfNotSuccess(DisplayLoaderParams(params));

	//Refresh the map after our allocations
	ReturnIfNotSuccess(EfiMemory::GetMemoryMap(map, memoryMapKey, descriptorVersion));

	//After ExitBootServices we can no longer use the BS handle (no print, memory, etc)
	ReturnIfNotSuccess(BS->ExitBootServices(ImageHandle, memoryMapKey));

	//Pre-process the final map into kernel ranges and assign runtime virtual addresses
	EfiMemory::BuildRanges(map, params);

	//Update UEFI virtual address map
	ReturnIfNotSuccessNoDisplay(RT->SetVirtualAddressMap(map.Size, map.DescriptorSize, descriptorVersion, map.Table));

	//Output final map to uart
	EfiMemory::DumpMemoryMap(map);

	/*
	 * Call to kernel.
	 * - Populate loader params first.
	 * - Finish CRT init.
	 * - Call kernel main
	 */
	const KernelMain kernelMain = (KernelMain)(entryPoint);
	*kParams = params;
	ReturnIfNotSuccess(EfiLoader::CrtInitialization((void*)KernelBase));
	kernelMain();

	//Should never get here
	return EFI_ABORTED;
}
