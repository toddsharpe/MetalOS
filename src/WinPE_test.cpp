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
	Assert(WinPE::GetNtHeaders(image) == nt);
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

	//A copy of this image's headers, to rewrite its debug directory
	static constexpr DWORD DirectoryRva = 0x800;
	static constexpr DWORD CodeViewRva = 0x900;
	static uint8_t headers[0x1000];
	const size_t headerSize = __ImageBase.e_lfanew + sizeof(IMAGE_NT_HEADERS64);
	Assert(headerSize <= DirectoryRva);
	memcpy(headers, image, headerSize);
	IMAGE_DATA_DIRECTORY& debug = MakePointer<IMAGE_NT_HEADERS64*>(headers, __ImageBase.e_lfanew)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];

	//No debug directory: no PDB name, rather than an assert (the stack walker then prints frames without symbols)
	debug = {};
	Assert(WinPE::GetPdbName(headers) == nullptr);

	//CodeView found after another entry type
	IMAGE_DEBUG_DIRECTORY* const entries = MakePointer<IMAGE_DEBUG_DIRECTORY*>(headers, DirectoryRva);
	entries[0] = {};
	entries[0].Type = IMAGE_DEBUG_TYPE_MISC;
	entries[1] = {};
	entries[1].Type = IMAGE_DEBUG_TYPE_CODEVIEW;
	entries[1].AddressOfRawData = CodeViewRva;
	WinPE::OMFSignatureRSDS* const rsds = MakePointer<WinPE::OMFSignatureRSDS*>(headers, CodeViewRva);
	memcpy(rsds->Signature, "RSDS", sizeof(rsds->Signature));
	memcpy(rsds->name, "late.pdb", sizeof("late.pdb"));
	debug = { DirectoryRva, (DWORD)(2 * sizeof(IMAGE_DEBUG_DIRECTORY)) };
	const char* const late = WinPE::GetPdbName(headers);
	Assert(late && strcmp(late, "late.pdb") == 0);

	//Only a non-CodeView entry: no PDB name
	debug = { DirectoryRva, (DWORD)sizeof(IMAGE_DEBUG_DIRECTORY) };
	Assert(WinPE::GetPdbName(headers) == nullptr);
}
