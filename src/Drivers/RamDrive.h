#pragma once

#include "core_crt/stdint.h"
#include "Lib/System.h"

// Page0: Superblock
// Page1-Size<<PageShift: files
class RamDrive
{
public:
	static constexpr size_t EntrySize = 32;
	static constexpr size_t MAX_NAME = EntrySize - sizeof(uint32_t) - sizeof(size_t);
	static constexpr size_t MAX_ENTRIES = x64::PageSize / EntrySize;

	struct Entry
	{
		char Name[MAX_NAME];
		uint32_t PageNumber;
		size_t Length;
	};
	static_assert(sizeof(Entry) == EntrySize, "Invalid Entry size");

	RamDrive() :
		m_address(),
		m_size(),
		m_superblock(),
		m_pageWatermark()
	{

	}
	
	RamDrive(const void *address, const size_t size) : m_address((uintptr_t)address),
													   m_size(size),
													   m_superblock((Superblock *)address),
													   m_pageWatermark(1)
	{
	}

	void Clear()
	{
		// Zero out superblock
		memset((void *)m_address, 0, x64::PageSize);
	}

	size_t FileCount()
	{
		size_t index;
		for (index = 0; index < MAX_ENTRIES; index++)
		{
			// Short circuit
			if (*m_superblock->Entries[index].Name == '\0')
				break;
		}

		return index;
	}

	void *Allocate(const char *name, const size_t size)
	{
		size_t index;
		bool found = GetFileIndex(name, index);
		// TODO: assert false and not -1
		// TODO: assert name is less than MAX_NAME
		// TODO: assert size is large enough

		// Add entry to superblock
		strcpy(m_superblock->Entries[index].Name, name);
		m_superblock->Entries[index].PageNumber = (uint32_t)m_pageWatermark;
		m_superblock->Entries[index].Length = size;

		void *address = (void *)(m_address + (m_pageWatermark << x64::PageShift));
		m_pageWatermark += x64::SizeToPages(size);
		return address;
	}

	bool Open(const char *name, void *&address, size_t &length)
	{
		size_t index;
		if (!GetFileIndex(name, index))
			return false;

		const Entry &entry = m_superblock->Entries[index];
		length = entry.Length;
		address = (void *)(m_address + ((uintptr_t)entry.PageNumber << x64::PageShift));
		return true;
	}

	Entry *begin() const { return &m_superblock->Entries[0]; }
	Entry *end() const { return &m_superblock->Entries[MAX_ENTRIES - 1]; }

	void Display() const
	{
		Printf("RamDrive:\n");
		for(const auto& item : *this)
		{
			//Skip empty entries
			if (*item.Name == '\0')
				break;
			
			Printf("    File: %s PageNumber: 0x%x Length: 0x%x\n", item.Name, item.PageNumber, item.Length);
		}
	}

private:
	// Returns true and index of file or false and index of next slot or -1 for full
	bool GetFileIndex(const char *name, size_t &index)
	{
		for (index = 0; index < MAX_ENTRIES; index++)
		{
			// Short circuit
			if (*m_superblock->Entries[index].Name == '\0')
				return false;

			if (stricmp(name, m_superblock->Entries[index].Name) == 0)
				return true;
		}

		index = (size_t)-1;
		return false;
	}

	struct Superblock
	{
		Entry Entries[MAX_ENTRIES];
	};
	static_assert(sizeof(Superblock) == x64::PageSize, "Invalid superblock size");

	uintptr_t m_address;
	size_t m_size;
	Superblock *m_superblock;
	size_t m_pageWatermark;
};
