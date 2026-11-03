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

	free(storage);
	free(pdb);
}
