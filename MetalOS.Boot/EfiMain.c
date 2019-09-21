#include "EfiMain.h"
#include "Common.h"
#include "Print.h"
#include "String.h"
#include "Device.h"
#include "Loader.h"
#include "Path.h"
#include "Error.h"
#include "Memory.h"

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

	//Map file, get base and entry point
	EFI_PHYSICAL_ADDRESS entry;
	ReturnIfNotSuccess(MapFile(KernelFile, &params->BaseAddress, &entry));

	//Technically everything after allocationg the loader block needs to free that memory before dying

	ReturnIfNotSuccess(InitializeGraphics(&params->Display));
	ReturnIfNotSuccess(DisplayLoaderParams(params));

	//Determine size of map
	UINTN mapSize = 0;
	status = BS->GetMemoryMap(&mapSize, params->MemoryMap, &params->MemoryMapKey, &params->MemoryMapDescriptorSize, &params->MemoryMapVersion);
	if (status != EFI_BUFFER_TOO_SMALL)
	{
		ReturnIfNotSuccess(status);
	}

	ReturnIfNotSuccess(BS->AllocatePool(AllocationType, mapSize, &params->MemoryMap));
	//TODO: Free memory allocated above if this second call fails
	ReturnIfNotSuccess(BS->GetMemoryMap(&mapSize, params->MemoryMap, &params->MemoryMapKey, &params->MemoryMapDescriptorSize, &params->MemoryMapVersion));

	//Get latest memory map, exit boot services
	ReturnIfNotSuccess(BS->ExitBootServices(ImageHandle, params->MemoryMapKey));

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
	PrintGopMode(&params->Display);
	return status;
}

EFI_STATUS Keywait(CHAR16* String)
{
	EFI_STATUS status;
	EFI_INPUT_KEY Key;

	Print(String);
	ReturnIfNotSuccess(Print(L"Press any key to continue..."));
	ReturnIfNotSuccess(ST->ConIn->Reset(ST->ConIn, FALSE));

	// Poll for key
	while ((status = ST->ConIn->ReadKeyStroke(ST->ConIn, &Key)) == EFI_NOT_READY);

	// Clear keystroke buffer (this is just a pause)
	ReturnIfNotSuccess(ST->ConIn->Reset(ST->ConIn, FALSE));

	Print(L"\r\n");
	return status;
}
