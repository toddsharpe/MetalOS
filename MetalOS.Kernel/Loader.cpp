#include "Loader.h"

//#include <crt_string.h>
#include <WindowsPE.h>
#include "Main.h"
#include "PortableExecutable.h"

Handle Loader::LoadKernelLibrary(const std::string& path)
{
	FileHandle* file = kernel.CreateFile(path, GenericAccess::Read);
	Assert(file);

	size_t read;

	//Dos header
	IMAGE_DOS_HEADER dosHeader;
	Assert(kernel.ReadFile(file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
	Assert(read == sizeof(IMAGE_DOS_HEADER));
	Assert(dosHeader.e_magic == IMAGE_DOS_SIGNATURE);

	//NT Header
	IMAGE_NT_HEADERS64 peHeader;
	Assert(kernel.SetFilePosition(file, dosHeader.e_lfanew));
	Assert(kernel.ReadFile(file, &peHeader, sizeof(IMAGE_NT_HEADERS64), &read));
	Assert(read == sizeof(IMAGE_NT_HEADERS64));

	//Verify image
	Assert(peHeader.Signature == IMAGE_NT_SIGNATURE);
	Assert(peHeader.FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64);
	Assert(peHeader.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC);

	//Verify base address is reasonable
	Assert(peHeader.OptionalHeader.ImageBase >= KernelAddress::KernelLibraryStart);
	Assert(peHeader.OptionalHeader.ImageBase < KernelAddress::KernelLibraryEnd);

	void* address = kernel.AllocateLibrary(peHeader.OptionalHeader.ImageBase, SIZE_TO_PAGES(peHeader.OptionalHeader.SizeOfImage));
	Assert(address);

	//Read headers
	Assert(kernel.SetFilePosition(file, 0));
	Assert(kernel.ReadFile(file, address, peHeader.OptionalHeader.SizeOfHeaders, &read));
	Assert(read == peHeader.OptionalHeader.SizeOfHeaders);

	PIMAGE_NT_HEADERS64 pNtHeader = MakePtr(PIMAGE_NT_HEADERS64, address, dosHeader.e_lfanew);

	//Write sections into memory
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION_64(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		uintptr_t destination = (uintptr_t)address + section[i].VirtualAddress;

		//If physical size is non-zero, read data to allocated address
		DWORD rawSize = section[i].SizeOfRawData;
		if (rawSize != 0)
		{
			Assert(kernel.SetFilePosition(file, section[i].PointerToRawData));
			Assert(kernel.ReadFile(file, (void*)destination, rawSize, &read));
			Assert(read == section[i].SizeOfRawData);
		}
	}

	//Kernel libraries should never be relocated

	//Kernel library dont have imports yet
	Assert(pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0);

	return address;
}

//TODO: this is very similar to KernelAPI's loader, as well as create process
Handle Loader::LoadLibrary(UserProcess& process, const char* path)
{
	FileHandle* file = kernel.CreateFile(std::string(path), GenericAccess::Read);
	Assert(file);

	size_t read;

	//Dos header
	IMAGE_DOS_HEADER dosHeader;
	Assert(kernel.ReadFile(file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
	Assert(read == sizeof(IMAGE_DOS_HEADER));
	Assert(dosHeader.e_magic == IMAGE_DOS_SIGNATURE);

	//NT Header
	IMAGE_NT_HEADERS64 peHeader;
	Assert(kernel.SetFilePosition(file, dosHeader.e_lfanew));
	Assert(kernel.ReadFile(file, &peHeader, sizeof(IMAGE_NT_HEADERS64), &read));
	Assert(read == sizeof(IMAGE_NT_HEADERS64));

	//Verify image
	if (peHeader.Signature != IMAGE_NT_SIGNATURE ||
		peHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
		peHeader.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		Assert(false);
		return nullptr;
	}

	void* address = kernel.VirtualAlloc(process, (void*)peHeader.OptionalHeader.ImageBase, peHeader.OptionalHeader.SizeOfImage, MemoryAllocationType::CommitReserve, MemoryProtection::PageReadWriteExecute);
	Assert(address);
	
	//Read headers
	Assert(kernel.SetFilePosition(file, 0));
	Assert(kernel.ReadFile(file, address, peHeader.OptionalHeader.SizeOfHeaders, &read));
	Assert(read == peHeader.OptionalHeader.SizeOfHeaders);

	Assert((uintptr_t)address == peHeader.OptionalHeader.ImageBase);

	PIMAGE_NT_HEADERS64 pNtHeader = MakePtr(PIMAGE_NT_HEADERS64, address, dosHeader.e_lfanew);

	//Write sections into memory
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION_64(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		uintptr_t destination = (uintptr_t)address + section[i].VirtualAddress;

		//If physical size is non-zero, read data to allocated address
		DWORD rawSize = section[i].SizeOfRawData;
		if (rawSize != 0)
		{
			Assert(kernel.SetFilePosition(file, section[i].PointerToRawData));
			Assert(kernel.ReadFile(file, (void*)destination, rawSize, &read));
			Assert(read == section[i].SizeOfRawData);
		}
	}

	//Imports are loaded from usermode, this is just for base kernelapi which shouldnt have any
	Assert(pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size == 0);

	//Add to process's load map
	process.AddModule(path, address);

	return address;
}

void Loader::KernelExports(void* address, void* kernelAddress)
{
	uintptr_t baseAddress = (uintptr_t)address;

	IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)baseAddress;
	IMAGE_NT_HEADERS64* ntHeader = (PIMAGE_NT_HEADERS64)(baseAddress + dosHeader->e_lfanew);
	
	IMAGE_DATA_DIRECTORY importDirectory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	Print("import size: 0x%016x\n", importDirectory.Size);
	if (importDirectory.Size)
	{
		PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, baseAddress, importDirectory.VirtualAddress);

		while (importDescriptor->Name)
		{
			char* module = MakePtr(char*, baseAddress, importDescriptor->Name);
			Print("    %s\n", module);
			if (stricmp(module, "mosrt.dll") == 0)
			{
				Handle hModule = kernelAddress;

				PIMAGE_THUNK_DATA pThunkData = MakePtr(PIMAGE_THUNK_DATA, baseAddress, importDescriptor->FirstThunk);
				while (pThunkData->u1.AddressOfData)
				{
					PIMAGE_IMPORT_BY_NAME pImportByName = MakePtr(PIMAGE_IMPORT_BY_NAME, baseAddress, pThunkData->u1.AddressOfData);

					pThunkData->u1.Function = PortableExecutable::GetProcAddress(hModule, (char*)pImportByName->Name);
					Print("Patched %s to 0x%016x\n", (char*)pImportByName->Name, pThunkData->u1.Function);
					pThunkData++;

				}
			}
			importDescriptor++;
		}
	}
}
