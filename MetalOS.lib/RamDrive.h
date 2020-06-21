#pragma once

#include <cstdint>
#include <MetalOS.Internal.h>

//Page0: Superblock
//Page1-Size<<PAGE_SHIFT: files
class RamDrive
{
public:
	static const size_t MAX_NAME = 28;
	static const size_t MAX_ENTRIES = 128;

	struct Entry
	{
		char Name[MAX_NAME];
		uint32_t PageNumber;
	};

	RamDrive(void* address, size_t size);

	void* Allocate(const char* name, const size_t size);
	void* Open(const char* name);

	Entry* begin() const { return &m_superblock->Entries[0]; }
	Entry* end() const { return &m_superblock->Entries[MAX_ENTRIES - 1]; }

private:
	bool GetFileIndex(const char* name, size_t& index);

	struct Superblock
	{
		Entry Entries[128];
	};
	static_assert(sizeof(Superblock) == PAGE_SIZE, "Invalid superblock size");

	uintptr_t m_address;
	size_t m_size;

	Superblock* m_superblock;

	size_t m_pageWatermark;
};

