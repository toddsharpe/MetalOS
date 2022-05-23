#include "Kernel.h"
#include "Assert.h"
#include "Loader.h"
#include <windows/types.h>
#include <windows/winnt.h>

#include "PortableExecutable.h"

void* Loader::LoadKernelLibrary(const std::string& path)
{
	KFile* file = kernel.KeCreateFile(path, GenericAccess::Read);
	Assert(file);

	size_t read;

	//Dos header
	IMAGE_DOS_HEADER dosHeader;
	Assert(kernel.KeReadFile(*file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
	AssertEqual(read, sizeof(IMAGE_DOS_HEADER));
	AssertEqual(dosHeader.e_magic, IMAGE_DOS_SIGNATURE);

	//NT Header
	IMAGE_NT_HEADERS peHeader;
	Assert(kernel.KeSetFilePosition(*file, dosHeader.e_lfanew));
	Assert(kernel.KeReadFile(*file, &peHeader, sizeof(IMAGE_NT_HEADERS), &read));
	AssertEqual(read, sizeof(IMAGE_NT_HEADERS));

	//Verify image
	AssertEqual(peHeader.Signature, IMAGE_NT_SIGNATURE);
	AssertEqual(peHeader.FileHeader.Machine, IMAGE_FILE_MACHINE_AMD64);
	AssertEqual(peHeader.OptionalHeader.Magic, IMAGE_NT_OPTIONAL_HDR64_MAGIC);

	//Verify base address is reasonable
	AssertOp(peHeader.OptionalHeader.ImageBase, >=, KernelAddress::KernelLibraryStart);
	AssertOp(peHeader.OptionalHeader.ImageBase, <, KernelAddress::KernelLibraryEnd);

	void* address = kernel.AllocateLibrary(peHeader.OptionalHeader.ImageBase, SIZE_TO_PAGES(peHeader.OptionalHeader.SizeOfImage));
	Assert(address);

	//Read headers
	Assert(kernel.KeSetFilePosition(*file, 0));
	Assert(kernel.KeReadFile(*file, address, peHeader.OptionalHeader.SizeOfHeaders, &read));
	AssertEqual(read, peHeader.OptionalHeader.SizeOfHeaders);

	PIMAGE_NT_HEADERS pNtHeader = MakePointer<PIMAGE_NT_HEADERS>(address, dosHeader.e_lfanew);

	//Write sections into memory
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		//If physical size is non-zero, read data to allocated address
		DWORD rawSize = section[i].SizeOfRawData;
		if (rawSize == 0)
			continue;
		
		void* destination = MakePointer<void*>(address, section[i].VirtualAddress);
		Assert(kernel.KeSetFilePosition(*file, section[i].PointerToRawData));
		Assert(kernel.KeReadFile(*file, destination, rawSize, &read));
		AssertEqual(read, section[i].SizeOfRawData);
	}

	//Kernel libraries should never be relocated

	//Kernel library dont have imports yet
	AssertEqual(pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size, 0);

	return address;
}

//TODO: this is very similar to KernelAPI's loader, as well as create process
Handle Loader::LoadLibrary(UserProcess& process, const char* path)
{
	KFile* file = kernel.KeCreateFile(std::string(path), GenericAccess::Read);
	Assert(file);

	size_t read;

	//Dos header
	IMAGE_DOS_HEADER dosHeader;
	Assert(kernel.KeReadFile(*file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
	AssertEqual(read, sizeof(IMAGE_DOS_HEADER));
	AssertEqual(dosHeader.e_magic, IMAGE_DOS_SIGNATURE);

	//NT Header
	IMAGE_NT_HEADERS peHeader;
	Assert(kernel.KeSetFilePosition(*file, dosHeader.e_lfanew));
	Assert(kernel.KeReadFile(*file, &peHeader, sizeof(IMAGE_NT_HEADERS), &read));
	AssertEqual(read, sizeof(IMAGE_NT_HEADERS));

	//Verify image
	AssertEqual(peHeader.Signature, IMAGE_NT_SIGNATURE);
	AssertEqual(peHeader.FileHeader.Machine, IMAGE_FILE_MACHINE_AMD64);
	AssertEqual(peHeader.OptionalHeader.Magic, IMAGE_NT_OPTIONAL_HDR64_MAGIC);

	void* address = kernel.VirtualAlloc(process, (void*)peHeader.OptionalHeader.ImageBase, peHeader.OptionalHeader.SizeOfImage, MemoryAllocationType::CommitReserve, MemoryProtection::PageReadWriteExecute);
	Assert(address);
	
	//Read headers
	Assert(kernel.KeSetFilePosition(*file, 0));
	Assert(kernel.KeReadFile(*file, address, peHeader.OptionalHeader.SizeOfHeaders, &read));
	AssertEqual(read, peHeader.OptionalHeader.SizeOfHeaders);
	AssertEqual((ULONGLONG)address, peHeader.OptionalHeader.ImageBase);

	PIMAGE_NT_HEADERS pNtHeader = MakePointer<PIMAGE_NT_HEADERS>(address, dosHeader.e_lfanew);

	//Write sections into memory
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		//If physical size is non-zero, read data to allocated address
		DWORD rawSize = section[i].SizeOfRawData;
		if (rawSize == 0)
			continue;

		void* destination = MakePointer<void*>(address, section[i].VirtualAddress);
		Assert(kernel.KeSetFilePosition(*file, section[i].PointerToRawData));
		Assert(kernel.KeReadFile(*file, destination, rawSize, &read));
		AssertEqual(read, section[i].SizeOfRawData);
	}

	//Imports are loaded from usermode, this is just for base kernelapi which shouldnt have any
	AssertEqual(pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size, 0);

	//Add to process's load map
	process.AddModule(path, address);

	return address;
}

void Loader::KernelExports(void* address, const Handle importLibrary, const std::string& libraryName)
{
	//Headers
	PIMAGE_DOS_HEADER dosHeader = static_cast<PIMAGE_DOS_HEADER>(address);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

	PIMAGE_NT_HEADERS64 ntHeader = MakePointer<PIMAGE_NT_HEADERS64>(address, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);
	
	IMAGE_DATA_DIRECTORY importDirectory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	kernel.Printf("import size: 0x%016x\n", importDirectory.Size);
	if (importDirectory.Size)
	{
		PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePointer<PIMAGE_IMPORT_DESCRIPTOR>(address, importDirectory.VirtualAddress);
		while (importDescriptor->Name)
		{
			const char* module = MakePointer<char*>(address, importDescriptor->Name);
			kernel.Printf("    %s\n", module);
			if (libraryName == module)
			{
				PIMAGE_THUNK_DATA pThunkData = MakePointer<PIMAGE_THUNK_DATA>(address, importDescriptor->FirstThunk);
				while (pThunkData->u1.AddressOfData)
				{
					PIMAGE_IMPORT_BY_NAME pImportByName = MakePointer<PIMAGE_IMPORT_BY_NAME>(address, pThunkData->u1.AddressOfData);

					pThunkData->u1.Function = (ULONGLONG)PortableExecutable::GetProcAddress(importLibrary, (char*)pImportByName->Name);
					kernel.Printf("Patched %s to 0x%016x\n", (char*)pImportByName->Name, pThunkData->u1.Function);
					pThunkData++;

				}
			}
			importDescriptor++;
		}
	}
}
