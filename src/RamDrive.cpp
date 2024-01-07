#include "RamDrive.h"
#include <string.h>

RamDrive::RamDrive(const void* address, const size_t size) :
	m_address((uintptr_t)address),
	m_size(size),
	m_superblock((Superblock*)address),
	m_pageWatermark(1)
{

}

void RamDrive::Clear()
{
	//Zero out superblock
	memset((void*)m_address, 0, PageSize);
}

size_t RamDrive::FileCount()
{
	size_t index;
	for (index = 0; index < MAX_ENTRIES; index++)
	{
		//Short circuit
		if (*m_superblock->Entries[index].Name == '\0')
			break;
	}

	return index;
}

void* RamDrive::Allocate(const char* name, const size_t size)
{
	size_t index;
	bool found = GetFileIndex(name, index);
	//TODO: assert false and not -1
	//TODO: assert name is less than MAX_NAME
	//TODO: assert size is large enough

	//Add entry to superblock
	strcpy(m_superblock->Entries[index].Name, name);
	m_superblock->Entries[index].PageNumber = (uint32_t)m_pageWatermark;
	m_superblock->Entries[index].Length = size;

	void* address = (void*)(m_address + (m_pageWatermark << PageShift));
	m_pageWatermark += SizeToPages(size);
	return address;
}

bool RamDrive::Open(const char* name, void*& address, size_t& length)
{
	size_t index;
	if (!GetFileIndex(name, index))
		return false;

	const Entry& entry = m_superblock->Entries[index];
	length = entry.Length;
	address = (void*)(m_address + ((uintptr_t)entry.PageNumber << PageShift));
	return true;
}

//Returns true and index of file or false and index of next slot or -1 for full
bool RamDrive::GetFileIndex(const char* name, size_t& index)
{
	for (index = 0; index < MAX_ENTRIES; index++)
	{
		//Short circuit
		if (*m_superblock->Entries[index].Name == '\0')
			return false;
		
		if (strcmp(name, m_superblock->Entries[index].Name) == 0)
			return true;
	}

	index = (size_t)-1;
	return false;
}
