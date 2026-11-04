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

	uint32_t blocks[BlockCount];
	for (size_t i = 0; i < BlockCount; i++)
		blocks[i] = (uint32_t)i;

	MsfStream stream(TestBlockSize, ReadTestBlock, data);
	stream.Size = (uint32_t)size;
	stream.BlockCount = (size + TestBlockSize - 1) / TestBlockSize;
	stream.SetBlocks(blocks);

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

//An MSF file with two 600-block streams (past the 512 a stream once held), out of order on disk, and a directory split across two blocks that aren't adjacent
static void MsfStreams_test(Arena& arena)
{
	static constexpr char Magic[0x1e] = "Microsoft C/C++ MSF 7.00\r\n\x1a\x44\x53";
	static constexpr uint32_t StreamCount = 2;
	static constexpr uint32_t StreamBlocks = 600;
	static constexpr uint32_t BlockMap = 1;
	static constexpr uint32_t DirectoryBlocks[] = { 2, 4 };
	static constexpr uint32_t BlockCount = 4 + StreamCount * StreamBlocks;
	static constexpr uint32_t DirectoryBytes = (1 + StreamCount + StreamCount * StreamBlocks) * sizeof(uint32_t);
	static_assert(DirectoryBytes > TestBlockSize && DirectoryBytes <= 2 * TestBlockSize, "Directory should take two blocks");

	uint8_t* const image = static_cast<uint8_t*>(calloc(BlockCount, TestBlockSize));
	Assert(image);
	uint32_t* const directory = static_cast<uint32_t*>(calloc(2, TestBlockSize));
	Assert(directory);

	//Every block but the superblock, block map and directory holds data, handed out last to first
	uint32_t dataBlocks[BlockCount];
	uint32_t dataCount = 0;
	for (uint32_t block = 0; block < BlockCount; block++)
	{
		if (block != 0 && block != BlockMap && block != DirectoryBlocks[0] && block != DirectoryBlocks[1])
			dataBlocks[dataCount++] = block;
	}
	AssertEqual(dataCount, StreamCount * StreamBlocks);

	//Each data block starts with its stream and index and ends with their complement
	size_t entry = 0;
	directory[entry++] = StreamCount;
	for (uint32_t s = 0; s < StreamCount; s++)
		directory[entry++] = StreamBlocks * TestBlockSize;
	for (uint32_t s = 0; s < StreamCount; s++)
	{
		for (uint32_t i = 0; i < StreamBlocks; i++)
		{
			const uint32_t block = dataBlocks[--dataCount];
			const uint32_t marker = (s << 16) | i;
			uint8_t* const data = image + (size_t)block * TestBlockSize;
			memcpy(data, &marker, sizeof(marker));
			const uint32_t complement = ~marker;
			memcpy(data + TestBlockSize - sizeof(complement), &complement, sizeof(complement));
			directory[entry++] = block;
		}
	}

	memcpy(image + (size_t)DirectoryBlocks[0] * TestBlockSize, directory, TestBlockSize);
	memcpy(image + (size_t)DirectoryBlocks[1] * TestBlockSize, (uint8_t*)directory + TestBlockSize, DirectoryBytes - TestBlockSize);
	memcpy(image + (size_t)BlockMap * TestBlockSize, DirectoryBlocks, sizeof(DirectoryBlocks));

	//Superblock: magic, BlockSize, FreeBlockMapBlock, NumBlocks, NumDirectoryBytes, Unknown, BlockMapAddr
	const uint32_t header[] = { TestBlockSize, 0, BlockCount, DirectoryBytes, 0, BlockMap };
	memcpy(image, Magic, sizeof(Magic));
	memcpy(image + 0x20, header, sizeof(header));

	MsfFile file(image);
	file.Initialize(arena);
	for (uint32_t s = 0; s < StreamCount; s++)
	{
		MsfStream& stream = file.GetStream(s);
		AssertEqual(stream.BlockCount, StreamBlocks);
		for (uint32_t i = 0; i < StreamBlocks; i++)
		{
			const uint32_t marker = (s << 16) | i;
			stream.Seek(i * TestBlockSize);
			AssertEqual(stream.Read<uint32_t>(), marker);
			stream.Seek((i + 1) * TestBlockSize - sizeof(uint32_t));
			AssertEqual(stream.Read<uint32_t>(), ~marker);
		}
	}

	free(directory);
	free(image);
}

//The MSF superblock's fields after its magic, as PdbOpen_test corrupts them
struct PdbOpen_testHeader
{
	uint32_t BlockSize;
	uint32_t FreeBlockMapBlock;
	uint32_t NumBlocks;
	uint32_t NumDirectoryBytes;
	uint32_t Unknown;
	uint32_t BlockMapAddr;
};

//Open on a corrupted copy of pdb fails instead of asserting; corrupt changes the copy (and may change the image it's opened with)
template <typename Corrupt>
static bool PdbOpen_test_open(const void* const pdb, const size_t pdbSize, Arena& arena, Corrupt corrupt)
{
	uint8_t* const copy = static_cast<uint8_t*>(malloc(pdbSize));
	Assert(copy);
	memcpy(copy, pdb, pdbSize);

	//A copy of this whole image, so its headers can be corrupted too
	const size_t imageSize = WinPE::GetSizeOfImage(&__ImageBase);
	uint8_t* const image = static_cast<uint8_t*>(malloc(imageSize));
	Assert(image);
	memcpy(image, &__ImageBase, imageSize);

	corrupt(copy, MakePointer<PdbOpen_testHeader*>(copy, 0x20), MakePointer<IMAGE_NT_HEADERS64*>(image, __ImageBase.e_lfanew));

	arena.Reset();
	PdbFile file;
	const bool opened = file.Open(copy, image, imageSize, arena);
	PdbLookup lookup = {};
	Assert(opened || !file.Resolve((uint32_t)((uintptr_t)&WinPE_test_export - (uintptr_t)&__ImageBase), lookup));

	free(image);
	free(copy);
	return opened;
}

//Every way Open can fail on a bad or too large PDB ends in false, never in an assert
static void PdbOpen_test(const void* const pdb, uint8_t* const storage, const size_t storageSize)
{
	const PdbOpen_testHeader& header = *MakePointer<const PdbOpen_testHeader*>(pdb, 0x20);
	const size_t pdbSize = (size_t)header.NumBlocks * header.BlockSize;
	Arena arena(storage, storageSize);

	//Untouched, as a control
	Assert(PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t*, PdbOpen_testHeader*, IMAGE_NT_HEADERS64*) {}));

	//Not an MSF file
	Assert(!PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t* copy, PdbOpen_testHeader*, IMAGE_NT_HEADERS64*) { copy[0] ^= 1; }));

	//A block size of zero (which once divided by zero), and one that isn't a power of two
	Assert(!PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t*, PdbOpen_testHeader* msf, IMAGE_NT_HEADERS64*) { msf->BlockSize = 0; }));
	Assert(!PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t*, PdbOpen_testHeader* msf, IMAGE_NT_HEADERS64*) { msf->BlockSize = 0x600; }));

	//A directory too small for its stream count, too big for the block map, or (from a bad block map) read from past the file's blocks
	Assert(!PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t*, PdbOpen_testHeader* msf, IMAGE_NT_HEADERS64*) { msf->NumDirectoryBytes = 2; }));
	Assert(!PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t*, PdbOpen_testHeader* msf, IMAGE_NT_HEADERS64*) { msf->NumDirectoryBytes = msf->BlockSize * msf->BlockSize; }));
	Assert(!PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t*, PdbOpen_testHeader* msf, IMAGE_NT_HEADERS64*) { msf->BlockMapAddr = msf->NumBlocks; }));

	//No streams, more streams than the directory holds, and too few for a DBI stream
	static constexpr auto SetStreamCount = [](uint8_t* copy, const PdbOpen_testHeader* msf, const uint32_t count)
	{
		const uint32_t directoryBlock = *MakePointer<const uint32_t*>(copy, (size_t)msf->BlockMapAddr * msf->BlockSize);
		*MakePointer<uint32_t*>(copy, (size_t)directoryBlock * msf->BlockSize) = count;
	};
	Assert(!PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t* copy, PdbOpen_testHeader* msf, IMAGE_NT_HEADERS64*) { SetStreamCount(copy, msf, 0); }));
	Assert(!PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t* copy, PdbOpen_testHeader* msf, IMAGE_NT_HEADERS64*) { SetStreamCount(copy, msf, 0x7FFFFFFF); }));
	Assert(!PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t* copy, PdbOpen_testHeader* msf, IMAGE_NT_HEADERS64*) { SetStreamCount(copy, msf, 3); }));

	//A DBI stream of another version (its header's second dword) or machine (at 58)
	static constexpr auto CorruptDbi = [](uint8_t* copy, const size_t offset)
	{
		static uint8_t streams[1 << 20];
		Arena streamArena(streams, sizeof(streams));
		MsfFile file(copy);
		Assert(file.Initialize(streamArena));
		const uint32_t block = file.GetStream(3).GetBlockNumber(0);
		static_cast<uint8_t*>(file.GetBlock(block))[offset] ^= 1;
	};
	Assert(!PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t* copy, PdbOpen_testHeader*, IMAGE_NT_HEADERS64*) { CorruptDbi(copy, 4); }));
	Assert(!PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t* copy, PdbOpen_testHeader*, IMAGE_NT_HEADERS64*) { CorruptDbi(copy, 58); }));

	//An image whose first section isn't .text, or that has none
	Assert(!PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t*, PdbOpen_testHeader*, IMAGE_NT_HEADERS64* nt) { IMAGE_FIRST_SECTION(nt)->Name[1] = 'T'; }));
	Assert(!PdbOpen_test_open(pdb, pdbSize, arena, [](uint8_t*, PdbOpen_testHeader*, IMAGE_NT_HEADERS64* nt) { nt->FileHeader.NumberOfSections = 0; }));

	//Arenas too small for the PDB, running out at each stage of Open (its streams, the DBI, the publics table, the names), down to one allocation short
	arena.Reset();
	Assert(PdbFile().Open(pdb, &__ImageBase, WinPE::GetSizeOfImage(&__ImageBase), arena));
	const size_t needed = (size_t)(static_cast<uint8_t*>(arena.Allocate(1)) - storage);
	for (size_t size = 0; size < needed; size = (size < 0x1000) ? size + 0x10 : size + needed / 64)
	{
		Arena small(storage, size);
		Assert(!PdbFile().Open(pdb, &__ImageBase, WinPE::GetSizeOfImage(&__ImageBase), small));
	}
	Arena exact(storage, needed);
	Assert(PdbFile().Open(pdb, &__ImageBase, WinPE::GetSizeOfImage(&__ImageBase), exact));
	Arena tight(storage, needed - 0x10);
	Assert(!PdbFile().Open(pdb, &__ImageBase, WinPE::GetSizeOfImage(&__ImageBase), tight));
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

	arena.Reset();
	MsfStreams_test(arena);

	PdbOpen_test(pdb, storage, ArenaSize);

	free(storage);
	free(pdb);
}
