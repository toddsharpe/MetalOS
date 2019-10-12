#include "EfiMain.h"
#include "Common.h"
#include "Print.h"
#include "String.h"
#include "Device.h"
#include "Loader.h"
#include "Path.h"
#include "Error.h"
#include "Memory.h"
//#define _JMP_BUF_DEFINED
#include <intrin.h>

#define EFI_DEBUG 1
#define Kernel L"moskrnl.exe"
#define MaxKernelPath 64

EFI_SYSTEM_TABLE *ST;
EFI_RUNTIME_SERVICES* RT;
EFI_BOOT_SERVICES* BS;
EFI_MEMORY_TYPE AllocationType;

EFI_STATUS EfiMain(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable)
{
	EFI_STATUS status;

	ST = SystemTable;
	BS = SystemTable->BootServices;
	RT = SystemTable->RuntimeServices;

	EFI_LOADED_IMAGE* LoadedImage;
	ReturnIfNotSuccess(BS->OpenProtocol(ImageHandle, &LoadedImageProtocol, &LoadedImage, NULL, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));
	AllocationType = LoadedImage->ImageDataType;
	CHAR16* BootFilePath = ((FILEPATH_DEVICE_PATH*)LoadedImage->FilePath)->PathName;

	ReturnIfNotSuccess(Print(L"MetalOS.BootLoader\n\r"));
	ReturnIfNotSuccess(Print(L"  Firmware Vendor: %S, Revision: %d\n\r", ST->FirmwareVendor, ST->FirmwareRevision));
	ReturnIfNotSuccess(Print(L"  Bootloader: %S\r\n", BootFilePath));

	//Detect CPU vendor and features
	int registers[4] = { -1 };
	char vendor[13] = { 0 };

	__cpuid(&registers, 0);
	*((UINT32*)vendor) = (UINT32)registers[1];
	*((UINT32*)(vendor + 4)) = (UINT32)registers[3];
	*((UINT32*)(vendor + 8)) = (UINT32)registers[2];

	ReturnIfNotSuccess(Print(L"  CPU vendor: %s\r\n", vendor));

	//Detect CPU
	__cpuid(&registers, 0x80000001);

	//This means its supported, may not be active
	int x64 = (registers[3] & (1 << 29)) != 0;
	ReturnIfNotSuccess(Print(L"  CPU x64 Mode: %d\r\n", x64));

	UINT64 efer = __readmsr(0xC0000080);
	int lme = (efer & (1 << 8)) != 0;
	int lma = (efer & (1 << 10)) != 0;
	ReturnIfNotSuccess(Print(L"  LME: %b LMA: %b\r\n", lme, lma));

	UINT64 cr0 = __readcr0();
	ReturnIfNotSuccess(Print(L"CR0 - %u\r\n", (UINT32)cr0));
	int paging = (cr0 & ((UINT32)1 << 31)) != 0;
	ReturnIfNotSuccess(Print(L"  Paging: %d\r\n", (UINT32)paging));

	UINT64 cr3 = __readcr3();
	ReturnIfNotSuccess(Print(L"CR3 - %q\r\n", (UINT64)cr3));

	UINT64 cr4 = __readcr4();
	ReturnIfNotSuccess(Print(L"CR4 - %q\r\n", (UINT64)cr4));
	int pse = (cr4 & (1 << 4)) != 0;
	int pae = (cr4 & (1 << 5)) != 0;
	int pcide = (cr4 & (1 << 17)) != 0;
	ReturnIfNotSuccess(Print(L"  PSE: %b PAE: %b PCIDE: %b\r\n", pse, pae, pcide));

	//Detect CPU topology - might not be working?
	/*
	int numProcessors;
	__cpuidex(&registers, 0xB, 0);
	numProcessors = registers[1];//EBX

	int numCoresPer;
	__cpuidex(&registers, 0xB, 1);
	numCoresPer = registers[1];//EBX
	ReturnIfNotSuccess(Print(L"  CPU Cores: %d, Logical Processors: %d\r\n", numProcessors, numProcessors * numCoresPer));
	*/

	EFI_TIME time;
	ReturnIfNotSuccess(RT->GetTime(&time, NULL));
	Print(L"  Date: %d-%d-%d %d:%d:%d\r\n", time.Month, time.Day, time.Year, time.Hour, time.Minute, time.Second);

	//Build path to kernel
	CHAR16* KernelPath;
	ReturnIfNotSuccess(BS->AllocatePool(AllocationType, MaxKernelPath * sizeof(CHAR16), &KernelPath));
	efi_memset((void*)KernelPath, 0, MaxKernelPath * sizeof(CHAR16));
	PathCombine(BootFilePath, KernelPath, Kernel); // THis function needs to be renamed/split

	//Load kernel
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fileSystem;
	ReturnIfNotSuccess(ST->BootServices->OpenProtocol(LoadedImage->DeviceHandle, &FileSystemProtocol, &fileSystem, ImageHandle, NULL, EFI_OPEN_PROTOCOL_GET_PROTOCOL));

	EFI_FILE* CurrentDriveRoot;
	ReturnIfNotSuccess(fileSystem->OpenVolume(fileSystem, &CurrentDriveRoot));

	EFI_FILE* KernelFile = NULL;
	Print(L"Loading: %S\r\n", KernelPath);
	ReturnIfNotSuccess(CurrentDriveRoot->Open(CurrentDriveRoot, &KernelFile, KernelPath, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY));

	//Reserve space for loader block
	LOADER_PARAMS* params = NULL;
	ReturnIfNotSuccess(BS->AllocatePool(AllocationType, sizeof(LOADER_PARAMS), &params));
	efi_memset(params, 0, sizeof(LOADER_PARAMS));
	params->ConfigTables = ST->ConfigurationTable;
	params->ConfigTableSizes = ST->NumberOfTableEntries;

	//Map file, get base and entry point
	EFI_PHYSICAL_ADDRESS entry;
	ReturnIfNotSuccess(MapFile(KernelFile, &params->BaseAddress, &entry));

	//Technically everything after allocationg the loader block needs to free that memory before dying

	ReturnIfNotSuccess(InitializeGraphics(&params->Display));
	ReturnIfNotSuccess(DisplayLoaderParams(params));

	//Brief pause before launching kernel (for any output to be read)
	Keywait(L"Continue to boot kernel\r\n");

	//Determine size of map
	UINTN memoryMapKey = 0;
	status = BS->GetMemoryMap(&params->MemoryMapSize, params->MemoryMap, &memoryMapKey, &params->MemoryMapDescriptorSize, &params->MemoryMapVersion);
	if (status != EFI_BUFFER_TOO_SMALL)
	{
		ReturnIfNotSuccess(status);
	}

	ReturnIfNotSuccess(BS->AllocatePool(AllocationType, params->MemoryMapSize, &params->MemoryMap));
	//TODO: Free memory allocated above if this second call fails
	ReturnIfNotSuccess(BS->GetMemoryMap(&params->MemoryMapSize, params->MemoryMap, memoryMapKey, &params->MemoryMapDescriptorSize, &params->MemoryMapVersion));
	//Get latest memory map, exit boot services
	ReturnIfNotSuccess(BS->ExitBootServices(ImageHandle, memoryMapKey));

	//Call into kernel
	//This call or doing getmemory map and exitbootservices is enough to break us
	//Has to be some sort of stack bullshit
	KernelMain kernelMain = (KernelMain)(entry);
	kernelMain(params);

	Keywait(L"asd\r\n");

	//Should never get here
	return EFI_ABORTED;
}

EFI_STATUS DisplayLoaderParams(LOADER_PARAMS* params)
{
	EFI_STATUS status;
	
	ReturnIfNotSuccess(Print(L"DisplayLoaderParams:\r\n"));
	ReturnIfNotSuccess(Print(L"\tBaseAddress: %u\r\n", params->BaseAddress));
	ReturnIfNotSuccess(Print(L"\tConfigTables: %q\r\n", params->ConfigTables));
	ReturnIfNotSuccess(Print(L"\tConfigTableSizes: %d\r\n", params->ConfigTableSizes));

	PrintGopMode(&params->Display);
	return status;
}

EFI_STATUS Keywait(CHAR16* String)
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
