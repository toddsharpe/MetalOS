#include "RamDrive.h"
#include "crt_string.h"

RamDrive::RamDrive(void* address, size_t size) :
	m_address((uintptr_t)address),
	m_size(size),
	m_superblock((Superblock*)address),
	m_pageWatermark(1)
{
	//Zero out superblock
	memset(address, 0, PAGE_SIZE);
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
	m_superblock->Entries[index].PageNumber = m_pageWatermark;

	m_pageWatermark += SIZE_TO_PAGES(size);
	return (void*)(m_address + (m_pageWatermark << PAGE_SHIFT));
}

void* RamDrive::Open(const char* name)
{
	size_t index;
	if (!GetFileIndex(name, index))
		return nullptr;

	return (void*)(m_address + (m_superblock->Entries[index].PageNumber << PAGE_SHIFT));
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
