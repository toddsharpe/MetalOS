#pragma once

#include <stdint.h>
#include <kernel/MetalOS.Internal.h>

//Page0: Superblock
//Page1-Size<<PageShift: files
class RamDrive
{
public:
	static constexpr size_t EntrySize = 32;
	static constexpr size_t MAX_NAME = EntrySize - sizeof(uint32_t) - sizeof(size_t);
	static constexpr size_t MAX_ENTRIES = PageSize / EntrySize;

	struct Entry
	{
		char Name[MAX_NAME];
		uint32_t PageNumber;
		size_t Length;
	};
	static_assert(sizeof(Entry) == EntrySize, "Invalid Entry size");

	RamDrive(const void* address, const size_t size);
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
		Entry Entries[MAX_ENTRIES];
	};
	static_assert(sizeof(Superblock) == PageSize, "Invalid superblock size");

	uintptr_t m_address;
	size_t m_size;
	Superblock* m_superblock;
	size_t m_pageWatermark;
};

