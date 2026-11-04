#include "kernel/Pdb/PdbFile.h"
#include <cstdio>
#include <cstdlib>
#include <intrin.h>

extern "C" IMAGE_DOS_HEADER __ImageBase;
extern "C" int WinPE_test_export();

//Thread storage puts S_GTHREAD32 and S_LTHREAD32 records in this image's symbol stream, which the publics parser once asserted on
thread_local int Pdb_test_tls;
static thread_local int s_tls;

//Data, for an IP outside .text
static int s_data = 1;

static uintptr_t s_returnAddress;

extern "C" __declspec(noinline) void Pdb_test_callee()
{
	s_returnAddress = (uintptr_t)_ReturnAddress();
}

//The return address is in the line after the call
extern "C" __declspec(noinline) int Pdb_test_middle_line()
{
	Pdb_test_callee();
	return s_data;
}
static constexpr size_t MiddleLine = __LINE__ - 2;

//The epilog, the function's last bytes, is the closing brace's line, the last in its line table
extern "C" __declspec(noinline) void Pdb_test_last_line()
{
	Pdb_test_callee();
}
static constexpr size_t LastLine = __LINE__ - 1;

//Internal linkage gets no public symbol, so these are named from their module's procedure records
static __declspec(noinline) int Pdb_test_static()
{
	return s_data + 1;
}

namespace
{
	__declspec(noinline) int Pdb_test_anonymous()
	{
		return s_data + 2;
	}
}

//Reads a whole file into memory, as KeLoadFile does for the kernel's stack walker
static void* LoadFile(const char* const path, size_t* const fileSize = nullptr)
{
	FILE* const file = fopen(path, "rb");
	Assert(file);
	fseek(file, 0, SEEK_END);
	const long size = ftell(file);
	fseek(file, 0, SEEK_SET);

	void* const buffer = malloc(size);
	Assert(buffer);
	Assert(fread(buffer, 1, size, file) == (size_t)size);
	fclose(file);
	if (fileSize)
		*fileSize = (size_t)size;
	return buffer;
}

static constexpr size_t TestBlockSize = 0x1000;

//Blocks of an in-memory stream are consecutive pages of one buffer
static void* ReadTestBlock(void* const context, const uint32_t blockNumber)
{
	return static_cast<uint8_t*>(context) + (size_t)blockNumber * TestBlockSize;
}

//A symbol record stream with more .text publics than the old fixed table held (4096), then one outside .text
static void PdbPublics_test(Arena& arena)
{
	static constexpr size_t Count = 5000;
	static constexpr size_t BlockCount = 32;
	uint8_t* const data = static_cast<uint8_t*>(calloc(BlockCount, TestBlockSize));
	Assert(data);

	size_t size = 0;
	for (size_t i = 0; i <= Count; i++)
	{
		char name[16];
		const size_t nameLength = (size_t)snprintf(name, sizeof(name), "f%zu", i);
		const size_t recordSize = (2 + 2 + 4 + 4 + 2 + nameLength + 1 + 3) & ~(size_t)3;
		Assert(size + recordSize <= BlockCount * TestBlockSize);

		uint8_t* const record = data + size;
		const uint16_t length = (uint16_t)(recordSize - 2);
		const uint16_t type = S_PUB32;
		const uint32_t flags = 0;
		const uint32_t offset = (uint32_t)(i * 0x10);
		const uint16_t section = (i == Count) ? 2 : 1;
		memcpy(record + 0, &length, sizeof(length));
		memcpy(record + 2, &type, sizeof(type));
		memcpy(record + 4, &flags, sizeof(flags));
		memcpy(record + 8, &offset, sizeof(offset));
		memcpy(record + 12, &section, sizeof(section));
		memcpy(record + 14, name, nameLength + 1);
		size += recordSize;
	}

	MsfStream stream(TestBlockSize, ReadTestBlock, data);
	stream.Size = (uint32_t)size;
	stream.BlockCount = (size + TestBlockSize - 1) / TestBlockSize;
	for (size_t i = 0; i < stream.BlockCount; i++)
		stream.BlockNumbers.Add((uint32_t)i);

	PdbPublicsStream publics;
	publics.Load(stream, arena);

	CString name;
	Assert(publics.GetFunction(0, name));
	Assert(name == "f0");
	Assert(publics.GetFunction((uint32_t)((Count - 1) * 0x10), name));
	Assert(name == "f4999");
	Assert(!publics.GetFunction((uint32_t)(Count * 0x10), name));

	free(data);
}

//Open takes only the PDB the image was linked with, going by the GUID and age in the image's CodeView record
static void PdbMatch_test(const void* const pdb, const size_t pdbSize, Arena& arena)
{
	const void* const image = &__ImageBase;
	const WinPE::OMFSignatureRSDS* const codeView = WinPE::GetCodeView(image, WinPE::GetSizeOfImage(image));
	Assert(codeView);

	//A copy of this image's headers (sections included) with a debug directory of its own, so the CodeView record can be rewritten
	static constexpr DWORD DirectoryRva = 0x800;
	static constexpr DWORD CodeViewRva = 0x900;
	static uint8_t headers[0x1000];
	const DWORD headerSize = WinPE::GetNtHeaders(image)->OptionalHeader.SizeOfHeaders;
	Assert(headerSize <= DirectoryRva);
	memcpy(headers, image, headerSize);
	IMAGE_DATA_DIRECTORY& debug = MakePointer<IMAGE_NT_HEADERS64*>(headers, __ImageBase.e_lfanew)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
	IMAGE_DEBUG_DIRECTORY* const entry = MakePointer<IMAGE_DEBUG_DIRECTORY*>(headers, DirectoryRva);
	*entry = {};
	entry->Type = IMAGE_DEBUG_TYPE_CODEVIEW;
	entry->AddressOfRawData = CodeViewRva;
	WinPE::OMFSignatureRSDS* const rsds = MakePointer<WinPE::OMFSignatureRSDS*>(headers, CodeViewRva);
	memcpy(rsds, codeView, sizeof(*rsds));
	debug = { DirectoryRva, (DWORD)sizeof(IMAGE_DEBUG_DIRECTORY) };

	//The same GUID and age
	arena.Reset();
	Assert(PdbFile().Open(pdb, headers, sizeof(headers), arena));

	//Another build's GUID
	rsds->guid.Data4[7] ^= 1;
	arena.Reset();
	Assert(!PdbFile().Open(pdb, headers, sizeof(headers), arena));
	rsds->guid.Data4[7] ^= 1;

	//A later link than the PDB's (relinking keeps the GUID and bumps the age, so this is a stale PDB)
	rsds->age++;
	arena.Reset();
	Assert(!PdbFile().Open(pdb, headers, sizeof(headers), arena));
	rsds->age--;

	//An earlier link than the PDB's, which the DBI stream's age rules out
	Assert(rsds->age > 0);
	rsds->age--;
	arena.Reset();
	Assert(!PdbFile().Open(pdb, headers, sizeof(headers), arena));
	rsds->age++;

	//An image size its headers and CodeView record don't fit in, as for a process that scribbled on its own
	arena.Reset();
	Assert(!PdbFile().Open(pdb, headers, CodeViewRva, arena));
	arena.Reset();
	Assert(!PdbFile().Open(pdb, headers, __ImageBase.e_lfanew + sizeof(IMAGE_NT_HEADERS64) - 1, arena));

	//No CodeView record
	debug = {};
	arena.Reset();
	Assert(!PdbFile().Open(pdb, headers, sizeof(headers), arena));

	//The PDB side: a copy of it with its GUID corrupted wherever it appears
	uint8_t* const corrupt = static_cast<uint8_t*>(malloc(pdbSize));
	Assert(corrupt);
	memcpy(corrupt, pdb, pdbSize);
	size_t corrupted = 0;
	for (size_t i = 0; i + sizeof(codeView->guid) <= pdbSize; i++)
	{
		if (memcmp(corrupt + i, &codeView->guid, sizeof(codeView->guid)) != 0)
			continue;

		corrupt[i] ^= 1;
		corrupted++;
	}
	Assert(corrupted != 0);
	arena.Reset();
	Assert(!PdbFile().Open(corrupt, image, WinPE::GetSizeOfImage(image), arena));
	free(corrupt);
}

void Pdb_test()
{
	//Symbolize a function in this test executable from its own PDB, the same path the kernel's stack walker takes
	const void* const image = &__ImageBase;
	const char* const path = WinPE::GetPdbName(image, WinPE::GetSizeOfImage(image));
	Assert(path);
	size_t pdbSize = 0;
	void* const pdb = LoadFile(path, &pdbSize);

	static constexpr size_t ArenaSize = 16 << 20;
	uint8_t* const storage = static_cast<uint8_t*>(malloc(ArenaSize));
	Assert(storage);
	Arena arena(storage, ArenaSize);

	//Referenced, so the linker keeps the thread storage (and its records in the PDB)
	Pdb_test_tls++;
	s_tls++;

	PdbFile file;
	Assert(file.Open(pdb, image, WinPE::GetSizeOfImage(image), arena));

	const uint32_t rva = (uint32_t)((uintptr_t)&WinPE_test_export - (uintptr_t)image);
	PdbLookup lookup = {};
	Assert(file.Resolve(rva, lookup));
	Assert(lookup.Function == "WinPE_test_export");
	Assert(lookup.Line != 0);

	//An IP in data or the headers (a call through a bad pointer) is in no module's code, so it fails to resolve instead of asserting
	Assert(!file.Resolve((uint32_t)((uintptr_t)&s_data - (uintptr_t)image), lookup));
	Assert(!file.Resolve(0, lookup));

	//Every IP in .text resolves or fails cleanly, including a function's last line, after which the line walk used to read mid-subsection
	const IMAGE_SECTION_HEADER* const text = WinPE::GetPESection(image, ".text");
	Assert(text);
	size_t resolved = 0;
	for (uint32_t offset = 0; offset < text->Misc.VirtualSize; offset += 16)
	{
		if (file.Resolve(text->VirtualAddress + offset, lookup))
			resolved++;
	}
	Assert(resolved != 0);

	//A return address resolves to the line after the call
	Pdb_test_middle_line();
	Assert(file.Resolve((uint32_t)(s_returnAddress - (uintptr_t)image), lookup));
	Assert(lookup.Function == "Pdb_test_middle_line");
	AssertEqual(lookup.Line, MiddleLine);

	//Any function with line info resolves at its last byte (the ret, in its last line) to the same name as at its first
	const IMAGE_DATA_DIRECTORY& exceptions = WinPE::GetNtHeaders(image)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
	const RUNTIME_FUNCTION* const functions = MakePointer<const RUNTIME_FUNCTION*>(image, exceptions.VirtualAddress);
	const uint32_t lastLineRva = (uint32_t)((uintptr_t)&Pdb_test_last_line - (uintptr_t)image);
	size_t checked = 0;
	bool sawLastLine = false;
	for (size_t i = 0; i < exceptions.Size / sizeof(RUNTIME_FUNCTION); i++)
	{
		PdbLookup first = {};
		if (!file.Resolve(functions[i].BeginAddress, first))
			continue;

		PdbLookup last = {};
		Assert(file.Resolve(functions[i].EndAddress - 1, last));
		Assert(last.Function == first.Function);
		Assert(first.Function != "<not found>");
		checked++;

		if (functions[i].BeginAddress == lastLineRva)
		{
			Assert(last.Function == "Pdb_test_last_line");
			AssertEqual(last.Line, LastLine);
			sawLastLine = true;
		}
	}
	Assert(checked != 0);
	Assert(sawLastLine);

	//A static function and an anonymous-namespace one have lines but no public, so their names come from their procedure records
	AssertEqual(Pdb_test_static() + Pdb_test_anonymous(), 2 * s_data + 3);
	Assert(file.Resolve((uint32_t)((uintptr_t)&Pdb_test_static - (uintptr_t)image), lookup));
	Assert(lookup.Function == "Pdb_test_static");
	Assert(file.Resolve((uint32_t)((uintptr_t)&Pdb_test_anonymous - (uintptr_t)image), lookup));
	Assert(lookup.Function == "`anonymous namespace'::Pdb_test_anonymous");

	arena.Reset();
	PdbPublics_test(arena);

	PdbMatch_test(pdb, pdbSize, arena);

	free(storage);
	free(pdb);
}
