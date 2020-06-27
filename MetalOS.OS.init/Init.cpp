#include <cstdint>
#include <WindowsPE.h>
#include <MetalOS.h>

#pragma comment(linker, "/entry:InitProcess") //I dont think this works

extern "C" int main(int argc, char** argv);

extern "C" void InitProcess(uintptr_t baseAddress)
{
	DebugPrint("InitProcess");
	
	//Perform library loading
	IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)baseAddress;
	IMAGE_NT_HEADERS64* ntHeader = (PIMAGE_NT_HEADERS64)(baseAddress + dosHeader->e_lfanew);

	IMAGE_DATA_DIRECTORY importDirectory = ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (importDirectory.Size)
	{
		PIMAGE_IMPORT_DESCRIPTOR importDescriptor = MakePtr(PIMAGE_IMPORT_DESCRIPTOR, baseAddress, importDirectory.VirtualAddress);

		while (importDescriptor->Name)
		{
			char* module = MakePtr(char*, baseAddress, importDescriptor->Name);
			Handle hModule = LoadLibrary(module);

			PIMAGE_THUNK_DATA pThunkData = MakePtr(PIMAGE_THUNK_DATA, baseAddress, importDescriptor->FirstThunk);
			while (pThunkData->u1.AddressOfData)
			{
				PIMAGE_IMPORT_BY_NAME pImportByName = MakePtr(PIMAGE_IMPORT_BY_NAME, baseAddress, pThunkData->u1.AddressOfData);

				pThunkData->u1.Function = GetProcAddress(hModule, (char*)pImportByName->Name);
				pThunkData++;
			}
			importDescriptor++;
		}
	}

	//Call entry
	main(0, nullptr);

	//Kill process
	ExitProcess(0);
}

extern "C" void InitThread(uintptr_t threadStart)
{

}

