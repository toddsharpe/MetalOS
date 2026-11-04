#include "kernel/Pdb/PdbFile.h"
#include <cstdio>
#include <cstdlib>

extern "C" IMAGE_DOS_HEADER __ImageBase;
extern "C" int WinPE_test_export();

//Reads a whole file into memory, as KeLoadFile does for the kernel's stack walker
static void* LoadFile(const char* const path)
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

void Pdb_test()
{
	//Symbolize a function in this test executable from its own PDB, the same path the kernel's stack walker takes
	const void* const image = &__ImageBase;
	const char* const path = WinPE::GetPdbName(image);
	Assert(path);
	void* const pdb = LoadFile(path);

	static constexpr size_t ArenaSize = 16 << 20;
	uint8_t* const storage = static_cast<uint8_t*>(malloc(ArenaSize));
	Assert(storage);
	Arena arena(storage, ArenaSize);

	PdbFile file;
	file.Open(pdb, image, arena);

	const uint32_t rva = (uint32_t)((uintptr_t)&WinPE_test_export - (uintptr_t)image);
	PdbLookup lookup = {};
	Assert(file.Resolve(rva, lookup));
	Assert(lookup.Function == "WinPE_test_export");
	Assert(lookup.Line != 0);

	arena.Reset();
	PdbPublics_test(arena);

	free(storage);
	free(pdb);
}
