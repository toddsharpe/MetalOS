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

	//Reads exactly length bytes at offset, failing on a short (truncated) file
	static bool ReadAt(KFile& file, const size_t offset, void* const buffer, const size_t length)
	{
		size_t read = 0;
		return KeSetFilePosition(file, offset) && KeReadFile(file, buffer, length, &read) && read == length;
	}

	//Copies the headers and each section's raw data into the alias, bounds-checked against the image
	static bool LoadSections(KFile& file, void* const alias, const IMAGE_DOS_HEADER& dosHeader, const IMAGE_NT_HEADERS64& peHeader)
	{
		const size_t imageSize = peHeader.OptionalHeader.SizeOfImage;
		const size_t headersSize = peHeader.OptionalHeader.SizeOfHeaders;
		const size_t sectionsEnd = (size_t)dosHeader.e_lfanew + offsetof(IMAGE_NT_HEADERS64, OptionalHeader) +
			peHeader.FileHeader.SizeOfOptionalHeader + (size_t)peHeader.FileHeader.NumberOfSections * sizeof(IMAGE_SECTION_HEADER);
		if (headersSize > imageSize || sectionsEnd > headersSize)
			return false;

		if (!ReadAt(file, 0, alias, headersSize))
			return false;

		PIMAGE_NT_HEADERS64 pNtHeader = MakePointer<PIMAGE_NT_HEADERS64>(alias, dosHeader.e_lfanew);
		PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNtHeader);
		for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
		{
			const size_t rawSize = section[i].SizeOfRawData;
			if (rawSize == 0)
				continue;
			if ((size_t)section[i].VirtualAddress + rawSize > imageSize)
				return false;

			void* const destination = MakePointer<void*>(alias, section[i].VirtualAddress);
			if (!ReadAt(file, section[i].PointerToRawData, destination, rawSize))
				return false;
		}
		return true;
	}

	/**
	 * Loads a PE image into the kernel address space (contiguous) for reading and patching.
	 */
	static LoadedImage Load(const CString& path)
	{
		//A missing, truncated, or non-x64 PE file returns a null UserBase
		KFile file = {};
		if (!KeCreateFile(file, path.c_str(), KFileAccess::Read))
			return { nullptr, nullptr, 0 };

		//Headers go into kernel-local buffers - no process memory needed yet.
		IMAGE_DOS_HEADER dosHeader = {};
		if (!ReadAt(file, 0, &dosHeader, sizeof(IMAGE_DOS_HEADER)) || dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
			return { nullptr, nullptr, 0 };

		IMAGE_NT_HEADERS64 peHeader = {};
		if (dosHeader.e_lfanew < 0 || !ReadAt(file, dosHeader.e_lfanew, &peHeader, sizeof(IMAGE_NT_HEADERS64)))
			return { nullptr, nullptr, 0 };

		//Verify image
		if (peHeader.Signature != IMAGE_NT_SIGNATURE ||
			peHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
			peHeader.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC ||
			peHeader.OptionalHeader.SizeOfImage == 0)
			return { nullptr, nullptr, 0 };

		const size_t imageSize = peHeader.OptionalHeader.SizeOfImage;
		void* const userBase = (void*)peHeader.OptionalHeader.ImageBase;

		//Allocate + load the image in the kernel address space (contiguous VA) so it can be loaded
		//and patched without switching CR3. It is installed into the target process (at userBase)
		//later via KeAliasInto.
		void* const alias = KeVirtualAlloc(imageSize);
		Assert(alias);

		if (!LoadSections(file, alias, dosHeader, peHeader))
		{
			KeVirtualFree(alias);
			return { nullptr, nullptr, 0 };
		}

		return { userBase, alias, imageSize };
	}

	//Processes imports in the target image that resolve to the library. Operates entirely through
	//kernel aliases; the IAT is stamped with the library's user-space export addresses.
	static bool ResolveImports(const LoadedImage& image, const LoadedImage& library, const CString& libraryName)
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
						if (!funcAlias)
						{
							Printf("Loader: %s does not export %s\n", libraryName.c_str(), (char*)pImportByName->Name);
							return false;
						}
						pThunkData->u1.Function = (ULONGLONG)library.ToUser(funcAlias);
						pThunkData++;
					}
				}
				importDescriptor++;
			}
		}
		return true;
	}
}
