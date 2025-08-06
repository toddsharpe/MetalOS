#pragma once

#include "Kernel.h"
#include "windows/types.h"
#include "windows/winnt.h"
#include "Assert.h"
#include "kernel/KProcess.h"
#include "Lib/System.h"

//Used by:
// - Kernel to load an exe into a fresh process - just want to process RT imports
// - Kernel to load RT into every process - cant have imports
// - Kernel to load a DLL into its own process - might have imports from kernel/other libraries
namespace Loader
{
	//This is called from userpath (create process) so bulletproof
	static void* Load(KProcess& process, const CString& path)
	{
		Printf("kernel load: %s\n", path.c_str());
		
		KFile file = {};
		if (!KeCreateFile(file, path, KFileAccess::Read))
			return nullptr;

		size_t read = 0;

		//Dos header
		IMAGE_DOS_HEADER dosHeader = {};
		Assert(KeReadFile(file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
		AssertEqual(read, sizeof(IMAGE_DOS_HEADER));
		AssertEqual(dosHeader.e_magic, IMAGE_DOS_SIGNATURE);

		//NT Header
		IMAGE_NT_HEADERS64 peHeader = {};
		Assert(KeSetFilePosition(file, dosHeader.e_lfanew));
		Assert(KeReadFile(file, &peHeader, sizeof(IMAGE_NT_HEADERS), &read));
		AssertEqual(read, sizeof(IMAGE_NT_HEADERS));

		//Verify image
		AssertEqual(peHeader.Signature, IMAGE_NT_SIGNATURE);
		AssertEqual(peHeader.FileHeader.Machine, IMAGE_FILE_MACHINE_AMD64);
		AssertEqual(peHeader.OptionalHeader.Magic, IMAGE_NT_OPTIONAL_HDR64_MAGIC);

		//Allocate space for image in new address space
		void* const address = KeVirtualAlloc(process, (void*)peHeader.OptionalHeader.ImageBase, peHeader.OptionalHeader.SizeOfImage);
		AssertEqual(address, (void*)peHeader.OptionalHeader.ImageBase);

		//Load image into memory
		Assert(KeSetFilePosition(file, 0));
		Assert(KeReadFile(file, address, peHeader.OptionalHeader.SizeOfHeaders, &read));
		AssertEqual(read, peHeader.OptionalHeader.SizeOfHeaders);

		PIMAGE_NT_HEADERS64 pNtHeader = MakePointer<PIMAGE_NT_HEADERS64>(address, dosHeader.e_lfanew);

		//Write sections into memory
		PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNtHeader);
		for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
		{
			void* destination = MakePointer<void*>(address, section[i].VirtualAddress);

			//If physical size is non-zero, read data to allocated address
			DWORD rawSize = section[i].SizeOfRawData;
			if (rawSize != 0)
			{
				Assert(KeSetFilePosition(file, section[i].PointerToRawData));
				Assert(KeReadFile(file, destination, rawSize, &read));
				AssertEqual(read, section[i].SizeOfRawData);
			}
		}

		return address;
	}

	//Processes imports in the target image that resolve to the module
	static void ResolveImports(void* const image, void* const library, const CString& libraryName)
	{
		//Headers
		PIMAGE_DOS_HEADER dosHeader = static_cast<PIMAGE_DOS_HEADER>(image);
		AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

		PIMAGE_NT_HEADERS64 ntHeader = MakePointer<PIMAGE_NT_HEADERS64>(image, dosHeader->e_lfanew);
		AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);
		
		IMAGE_DATA_DIRECTORY importDirectory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		//Printf("import size: 0x%016x\n", importDirectory.Size);
		if (importDirectory.Size)
		{
			PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePointer<PIMAGE_IMPORT_DESCRIPTOR>(image, importDirectory.VirtualAddress);
			while (importDescriptor->Name)
			{
				const char* module = MakePointer<char*>(image, importDescriptor->Name);
				if (libraryName == CString(module))
				{
					PIMAGE_THUNK_DATA pThunkData = MakePointer<PIMAGE_THUNK_DATA>(image, importDescriptor->FirstThunk);
					while (pThunkData->u1.AddressOfData)
					{
						PIMAGE_IMPORT_BY_NAME pImportByName = MakePointer<PIMAGE_IMPORT_BY_NAME>(image, pThunkData->u1.AddressOfData);

						pThunkData->u1.Function = (ULONGLONG)WinPE::GetProcAddress(library, CString((char*)pImportByName->Name));
						//Printf("Patched %s to 0x%016x\n", (char*)pImportByName->Name, pThunkData->u1.Function);
						pThunkData++;

					}
				}
				importDescriptor++;
			}
		}
	}
}
