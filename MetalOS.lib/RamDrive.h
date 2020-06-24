#pragma once

#include <cstdint>
#include <MetalOS.Internal.h>

//Page0: Superblock
//Page1-Size<<PAGE_SHIFT: files
class RamDrive
{
public:
	static const size_t MAX_NAME = 28 - sizeof(size_t);
	static const size_t MAX_ENTRIES = 128;

	struct Entry
	{
		char Name[MAX_NAME];
		uint32_t PageNumber;
		size_t Length;
	};
	static_assert(sizeof(Entry) == 32, "Invalid Entry size");

	RamDrive(void* address, size_t size);
	void Clear();
	size_t FileCount();

	void* Allocate(const char* name, const size_t size);
	bool Open(const char* name, void*& address, size_t& length);

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

