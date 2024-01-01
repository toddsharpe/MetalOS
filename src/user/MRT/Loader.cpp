#include "Loader.h"
#include "PortableExecutable.h"
#include "user/MetalOS.Types.h"
#include "MetalOS.h"
#include "Runtime.h"
#include "user/Assert.h"

#define ReturnNullIfNot(x) if (!(x)) return nullptr;
#define RetNullIfFailed(x) if ((x) != SystemCallResult::Success) return nullptr;

void* Loader::LoadLibrary(const char* const moduleName)
{
	DebugPrintf("LoadLibrary: %s\n", moduleName);

	//Check if module is already loaded
	ProcessEnvironmentBlock* peb = Runtime::GetPEB();
	Module* module = Runtime::GetLoadedModule(moduleName);
	if (module != nullptr)
	{
		DebugPrintf("  Loaded at: 0x%016x\n", module);
		return module->Address;
	}

	//Load it
	const HFile file = CreateFile(moduleName, GenericAccess::Read);
	Assert(file);

	size_t read;

	//Dos header
	IMAGE_DOS_HEADER dosHeader;
	RetNullIfFailed(ReadFile(file, &dosHeader, sizeof(IMAGE_DOS_HEADER), &read));
	ReturnNullIfNot(read == sizeof(IMAGE_DOS_HEADER));
	ReturnNullIfNot(dosHeader.e_magic == IMAGE_DOS_SIGNATURE);

	//NT Header
	IMAGE_NT_HEADERS64 peHeader;
	RetNullIfFailed(SetFilePointer(file, dosHeader.e_lfanew, FilePointerMove::Begin, nullptr));
	RetNullIfFailed(ReadFile(file, &peHeader, sizeof(IMAGE_NT_HEADERS64), &read));
	ReturnNullIfNot(read == sizeof(IMAGE_NT_HEADERS64));

	//Verify image
	if (peHeader.Signature != IMAGE_NT_SIGNATURE ||
		peHeader.FileHeader.Machine != IMAGE_FILE_MACHINE_AMD64 ||
		peHeader.OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
	{
		ReturnNullIfNot(false);
	}

	//Reserve space for image
	Handle moduleBase = VirtualAlloc((void*)peHeader.OptionalHeader.ImageBase, peHeader.OptionalHeader.SizeOfImage);
	//TODO(tsharpe): Implement relocations (below). Until then assert address is the one in PE header.
	ReturnNullIfNot(moduleBase)
		DebugPrintf("  Loading at 0x%016x\n", moduleBase);

	//Read in headers
	RetNullIfFailed(SetFilePointer(file, 0, FilePointerMove::Begin, nullptr));
	RetNullIfFailed(ReadFile(file, moduleBase, peHeader.OptionalHeader.SizeOfHeaders, &read));
	ReturnNullIfNot(read == peHeader.OptionalHeader.SizeOfHeaders);

	//Update pointer to loaded module
	PIMAGE_NT_HEADERS64 pNtHeader = MakePointer<PIMAGE_NT_HEADERS64>(moduleBase, dosHeader.e_lfanew);

	//Write sections into memory
	PIMAGE_SECTION_HEADER section = IMAGE_FIRST_SECTION(pNtHeader);
	for (WORD i = 0; i < pNtHeader->FileHeader.NumberOfSections; i++)
	{
		uintptr_t destination = (uintptr_t)moduleBase + section[i].VirtualAddress;

		//If physical size is non-zero, read data to allocated address
		DWORD rawSize = section[i].SizeOfRawData;
		if (rawSize != 0)
		{
			RetNullIfFailed(SetFilePointer(file, section[i].PointerToRawData, FilePointerMove::Begin, nullptr));
			RetNullIfFailed(ReadFile(file, (void*)destination, rawSize, &read));
			ReturnNullIfNot(read == section[i].SizeOfRawData);
		}
	}

	//Process imports
	Loader::ProcessImports(moduleBase, false);

	//Relocations
	//IMAGE_DATA_DIRECTORY relocationDirectory = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
	//if (((uintptr_t)moduleBase != pNtHeader->OptionalHeader.ImageBase) && (relocationDirectory.Size))
	//{
	//	PIMAGE_BASE_RELOCATION pBaseRelocation = MakePtr(PIMAGE_BASE_RELOCATION, moduleBase, relocationDirectory.VirtualAddress);

	//	//Calculate relative shift
	//	uintptr_t delta = (uintptr_t)moduleBase - (uintptr_t)pNtHeader->OptionalHeader.ImageBase;

	//	DWORD bytes = 0;
	//	while (bytes < relocationDirectory.Size)
	//	{
	//		PDWORD locationBase = MakePtr(PDWORD, moduleBase, pBaseRelocation->VirtualAddress);
	//		PWORD locationData = MakePtr(PWORD, pBaseRelocation, sizeof(IMAGE_BASE_RELOCATION));

	//		DWORD count = (pBaseRelocation->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
	//		for (DWORD i = 0; i < count; i++)
	//		{
	//			int type = (*locationData >> 12);
	//			int value = (*locationData & 0x0FFF);

	//			switch (type)
	//			{
	//			case IMAGE_REL_BASED_ABSOLUTE:
	//				break;

	//			case IMAGE_REL_BASED_HIGHLOW:
	//				*MakePtr(PDWORD, locationBase, value) += delta;
	//				break;
	//			}

	//			locationData++;
	//		}

	//		bytes += pBaseRelocation->SizeOfBlock;
	//		pBaseRelocation = (PIMAGE_BASE_RELOCATION)locationData;
	//	}
	//}

	//Run static initializers
	Loader::CrtInit(moduleBase);

	//TODO: TLS

	//Execute entry point
	DllMainCall main = MakePointer<DllMainCall>(PortableExecutable::GetEntryPoint(moduleBase));
	main(moduleBase, DllEntryReason::ProcessAttach);

	return moduleBase;
}

//Kernel does RT imports. Due to the PE unions, imports can't be done more than once.
void Loader::ProcessImports(const void* const imageBase, const bool skipRT)
{
	//Headers
	const IMAGE_DOS_HEADER* dosHeader = static_cast<const IMAGE_DOS_HEADER*>(imageBase);
	AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);
	const IMAGE_NT_HEADERS64* ntHeader = MakePointer<const IMAGE_NT_HEADERS64*>(imageBase, dosHeader->e_lfanew);
	AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

	//Imports
	const IMAGE_DATA_DIRECTORY* importDirectory = &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (!importDirectory->Size)
		return;

	const IMAGE_IMPORT_DESCRIPTOR* importDescriptor = MakePointer<const IMAGE_IMPORT_DESCRIPTOR*>(imageBase, importDirectory->VirtualAddress);

	while (importDescriptor->Name)
	{
		const char* module = MakePointer<char*>(imageBase, importDescriptor->Name);
		Assert(module);
		const void* moduleAddr = Loader::LoadLibrary(module);
		Assert(moduleAddr);

		IMAGE_THUNK_DATA* pThunkData = MakePointer<IMAGE_THUNK_DATA*>(imageBase, importDescriptor->FirstThunk);
		importDescriptor++;
		
		if ((skipRT) && (_stricmp(module, "mosrt.dll") == 0))
			continue;
		
		while (pThunkData->u1.AddressOfData)
		{
			PIMAGE_IMPORT_BY_NAME pImportByName = MakePointer<PIMAGE_IMPORT_BY_NAME>(imageBase, pThunkData->u1.AddressOfData);

			pThunkData->u1.Function = (uintptr_t)PortableExecutable::GetProcAddress(moduleAddr, pImportByName->Name);
			DebugPrintf("  Patched %s to 0x%016x\n", (char*)pImportByName->Name, pThunkData->u1.Function);
			pThunkData++;
		}
	}
}

typedef void (*CrtInitializer)();
//Initializes C++ statics
void Loader::CrtInit(const void* imageBase)
{
	const IMAGE_SECTION_HEADER* crtSection = PortableExecutable::GetPESection(imageBase, ".CRT");
	if (crtSection != nullptr)
	{
		CrtInitializer* initializer = MakePointer<CrtInitializer*>(imageBase, crtSection->VirtualAddress);
		while (*initializer)
		{
			(*initializer)();
			initializer++;
		}
	}
}

