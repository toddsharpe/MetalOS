#include "WinPE.h"
#include <cstring>

//Linker-provided base of this test executable, a loaded PE image to parse
extern "C" IMAGE_DOS_HEADER __ImageBase;

//Exported so this image has an export table, as kdcom.dll and the runtime DLL do
extern "C" __declspec(dllexport) int WinPE_test_export()
{
	return 42;
}

void WinPE_test()
{
	const void* const image = &__ImageBase;
	const IMAGE_NT_HEADERS64* const nt = MakePointer<const IMAGE_NT_HEADERS64*>(image, __ImageBase.e_lfanew);

	//Header fields
	AssertEqual(WinPE::GetEntryPoint(image), nt->OptionalHeader.AddressOfEntryPoint);
	AssertEqual(WinPE::GetSizeOfImage(image), nt->OptionalHeader.SizeOfImage);
	Assert(WinPE::IsConsole(image));

	//Exports resolve by name to the function's loaded address (how the kernel binds kdcom.dll); unknown names don't
	Assert(WinPE::GetProcAddress(image, "WinPE_test_export") == reinterpret_cast<void*>(&WinPE_test_export));
	Assert(WinPE::GetProcAddress(image, "WinPE_test_missing") == nullptr);

	//Sections by name and index
	const IMAGE_SECTION_HEADER* const text = WinPE::GetPESection(image, ".text");
	Assert(text);
	Assert(WinPE::GetPESection(image, (uint32_t)0) == IMAGE_FIRST_SECTION(nt));
	Assert(WinPE::GetPESection(image, ".nosuch") == nullptr);

	//The debug directory names this image's PDB (the stack walker finds symbols this way)
	const char* const pdb = WinPE::GetPdbName(image);
	Assert(pdb);
	const size_t length = strlen(pdb);
	Assert(length >= strlen("Test.pdb"));
	Assert(_stricmp(pdb + length - strlen("Test.pdb"), "Test.pdb") == 0);
}
