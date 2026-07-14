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
	/**
	 * Loads a PE image into the kernel address space (contiguous) for reading and patching. Install it
	 * into a process at UserBase (its ImageBase) with KeAliasInto.
	 */
	struct LoadedImage
	{
		void* UserBase;
		void* Alias;
		size_t Size;

		//Translate a pointer inside this image's kernel alias to the equivalent process (user) address.
		void* ToUser(const void* const aliasPtr) const
		{
			return reinterpret_cast<void*>((uintptr_t)UserBase + ((uintptr_t)aliasPtr - (uintptr_t)Alias));
		}
	};

	/**
	 * Loads a PE image into the kernel address space (contiguous) for reading and patching.
	 */
	static LoadedImage Load(const CString& path)
	{
		KFile file = {};
		if (!KeCreateFile(file, path.c_str(), KFileAccess::Read))
			return { nullptr, nullptr, 0 };
		size_t read = 0;

		//Headers go into kernel-local buffers - no process memory needed yet.
		IMAGE_DOS_HEADER dosHeader = {};
		Assert(KeReadFile(file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
		AssertEqual(read, sizeof(IMAGE_DOS_HEADER));
		AssertEqual(dosHeader.e_magic, IMAGE_DOS_SIGNATURE);

		IMAGE_NT_HEADERS64 peHeader = {};
		Assert(KeSetFilePosition(file, dosHeader.e_lfanew));
		Assert(KeReadFile(file, &peHeader, sizeof(IMAGE_NT_HEADERS), &read));
		AssertEqual(read, sizeof(IMAGE_NT_HEADERS));

		//Verify image
		AssertEqual(peHeader.Signature, IMAGE_NT_SIGNATURE);
		AssertEqual(peHeader.FileHeader.Machine, IMAGE_FILE_MACHINE_AMD64);
		AssertEqual(peHeader.OptionalHeader.Magic, IMAGE_NT_OPTIONAL_HDR64_MAGIC);

		const size_t imageSize = peHeader.OptionalHeader.SizeOfImage;
		void* const userBase = (void*)peHeader.OptionalHeader.ImageBase;

		//Allocate + load the image in the kernel address space (contiguous VA) so it can be loaded
		//and patched without switching CR3. It is installed into the target process (at userBase)
		//later via KeAliasInto.
		void* const alias = KeVirtualAlloc(imageSize);
		Assert(alias);

		//Load headers into the alias.
		Assert(KeSetFilePosition(file, 0));
		Assert(KeReadFile(file, alias, peHeader.OptionalHeader.SizeOfHeaders, &read));
		AssertEqual(read, peHeader.OptionalHeader.SizeOfHeaders);

		//Write sections into the alias.
		PIMAGE_NT_HEADERS64 pNtHeader = MakePointer<PIMAGE_NT_HEADERS64>(alias, dosHeader.e_lfanew);
		PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNtHeader);
		for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
		{
			void* destination = MakePointer<void*>(alias, section[i].VirtualAddress);

			//If physical size is non-zero, read data to the aliased address
			DWORD rawSize = section[i].SizeOfRawData;
			if (rawSize != 0)
			{
				Assert(KeSetFilePosition(file, section[i].PointerToRawData));
				Assert(KeReadFile(file, destination, rawSize, &read));
				AssertEqual(read, section[i].SizeOfRawData);
			}
		}

		return { userBase, alias, imageSize };
	}

	//Processes imports in the target image that resolve to the library. Operates entirely through
	//kernel aliases; the IAT is stamped with the library's user-space export addresses.
	static void ResolveImports(const LoadedImage& image, const LoadedImage& library, const CString& libraryName)
	{
		//Headers
		PIMAGE_DOS_HEADER dosHeader = static_cast<PIMAGE_DOS_HEADER>(image.Alias);
		AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

		PIMAGE_NT_HEADERS64 ntHeader = MakePointer<PIMAGE_NT_HEADERS64>(image.Alias, dosHeader->e_lfanew);
		AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

		IMAGE_DATA_DIRECTORY importDirectory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
		if (importDirectory.Size)
		{
			PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePointer<PIMAGE_IMPORT_DESCRIPTOR>(image.Alias, importDirectory.VirtualAddress);
			while (importDescriptor->Name)
			{
				const char* module = MakePointer<char*>(image.Alias, importDescriptor->Name);
				if (libraryName == CString(module))
				{
					PIMAGE_THUNK_DATA pThunkData = MakePointer<PIMAGE_THUNK_DATA>(image.Alias, importDescriptor->FirstThunk);
					while (pThunkData->u1.AddressOfData)
					{
						PIMAGE_IMPORT_BY_NAME pImportByName = MakePointer<PIMAGE_IMPORT_BY_NAME>(image.Alias, pThunkData->u1.AddressOfData);

						//Resolve against the library's alias, then store its USER address (the process
						//will call through this IAT entry).
						void* const funcAlias = WinPE::GetProcAddress(library.Alias, CString((char*)pImportByName->Name));
						pThunkData->u1.Function = (ULONGLONG)library.ToUser(funcAlias);
						pThunkData++;
					}
				}
				importDescriptor++;
			}
		}
	}
}
