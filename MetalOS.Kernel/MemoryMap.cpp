#include "MemoryMap.h"
#include "LoadingScreen.h"
#include "LoaderParams.h"
#include "Main.h"
#include "CRT.h"
#include "Kernel.h"

extern LOADER_PARAMS* pParams;
//extern Display display;
extern LoadingScreen* loading;

MemoryMap::MemoryMap(UINTN MemoryMapSize, UINTN MemoryMapDescriptorSize, UINT32 MemoryMapVersion, EFI_MEMORY_DESCRIPTOR* MemoryMap, UINTN maxSize) : 
	m_memoryMapSize(MemoryMapSize), m_memoryMapDescriptorSize(MemoryMapDescriptorSize), m_memoryMapVersion(MemoryMapVersion), m_memoryMap(MemoryMap), m_maxSize(maxSize)
{
}

void MemoryMap::ReclaimBootPages()
{
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = m_memoryMap;
		current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize);
		current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		if ((current->Type == EfiBootServicesCode) || (current->Type == EfiBootServicesData) || (current->Type == EfiLoaderCode))
			current->Type = EfiConventionalMemory;
	}
}

//memoryMapDescriptorSize isn't the same size of the structure, so always use the firmware one
void MemoryMap::MergeConventionalPages()
{
	UINTN descriptorsMerged = 0;
	const UINT32 sentinel = EfiMaxMemoryType + 1;
	UINT32 descriptorCount = (m_memoryMapSize / m_memoryMapDescriptorSize);
	
	EFI_MEMORY_DESCRIPTOR* current;
	EFI_MEMORY_DESCRIPTOR* next;
	for (current = m_memoryMap;
		current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize - m_memoryMapDescriptorSize);
		current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		next = NextMemoryDescriptor(current, m_memoryMapDescriptorSize);
		
		if ((current->Type == EfiConventionalMemory) && (next->Type == EfiConventionalMemory))
			descriptorsMerged++;
	}

	UINT32 newMapSize = m_memoryMapSize - descriptorsMerged * m_memoryMapDescriptorSize;
	loading->WriteLineFormat("m: %d", descriptorsMerged);
	loading->WriteLineFormat("B: %x A: %x", m_memoryMapSize, newMapSize);

	//Seems we don't need to worry about resizing the allocation for the memory map, so just assert its fine
	Assert((m_memoryMapSize >> EFI_PAGE_SIZE) != (newMapSize >> EFI_PAGE_SHIFT));
	//On second thought, we do, maybe?

	EFI_MEMORY_DESCRIPTOR* memoryMapPage = ResolveAddress((EFI_PHYSICAL_ADDRESS)m_memoryMap);
	loading->WriteLineFormat("F: %x A: %x", memoryMapPage->PhysicalStart, memoryMapPage->NumberOfPages);
	return;
	
	//Indexes wont work!
	//Scan over memory map, combining adjacent EfiConventionalMemory blocks
	for (UINT32 i = 0; i < descriptorCount - 1; i++)
	{
		if (m_memoryMap[i].Type != EfiConventionalMemory)
			continue;
		
		UINT32 j = i + 1;
		while ((j < descriptorCount) && (m_memoryMap[j].Type == EfiConventionalMemory))
		{
			m_memoryMap[i].NumberOfPages += m_memoryMap[j].NumberOfPages;
			m_memoryMap[j].Type = sentinel;
			descriptorsMerged++;

			j++;
		}

		//memoryMap[j] points to next real record

		//Copy remaining records back
		CRT::memcpy(&m_memoryMap[i + 1], &m_memoryMap[j], m_memoryMapDescriptorSize * (descriptorCount - j));
		descriptorCount -= (j - i);
	}


	//Find entry that contains memory map, calculate reduction in pages and decrement
	

	//Find entry just after this one. If its free (conventional) decrement the starting address
	//If its not, then insert a new one
}

//Returns the lowest address that is free with at least this many pages
EFI_PHYSICAL_ADDRESS MemoryMap::GetLowestFree(UINT32 count)
{
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = m_memoryMap;
		current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize);
		current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		if (current->NumberOfPages >= count)
			return current->PhysicalStart;
	}

	Assert(false);
}

//Maybe return bool?
EFI_PHYSICAL_ADDRESS MemoryMap::AllocatePages(UINT32 count)
{
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = m_memoryMap; current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize); current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		if (current->NumberOfPages < count)
			continue;

		break;
	}

	Assert(current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize));
	if (current->NumberOfPages > count)
	{
		Assert(m_memoryMapSize + m_memoryMapDescriptorSize <= m_maxSize);

		int index = (UINTN)(m_memoryMap - current) / m_memoryMapDescriptorSize;
		int length = m_memoryMapSize / m_memoryMapDescriptorSize;

		//Copy all subsequent records down one
		for (int i = length - 1; i >= index; i--)
		{
			CRT::memcpy(MakePtr(void*, m_memoryMap, m_memoryMapDescriptorSize * (i + 1)), MakePtr(void*, m_memoryMap, m_memoryMapDescriptorSize * i), m_memoryMapDescriptorSize);
		}
		m_memoryMapSize += m_memoryMapDescriptorSize;
		
		//Fix up next record
		EFI_MEMORY_DESCRIPTOR* next = NextMemoryDescriptor(current, m_memoryMapDescriptorSize);
		next->PhysicalStart = current->PhysicalStart + (count * EFI_PAGE_SIZE);
		next->NumberOfPages = current->NumberOfPages - count;

		//Set size of current
		current->NumberOfPages = count;
	}
	
	//Mark current allocated
	current->Type = EfiLoaderData;
	return current->PhysicalStart;
}

bool MemoryMap::AddressFree(EFI_PHYSICAL_ADDRESS address)
{
	EFI_MEMORY_DESCRIPTOR* descriptor = ResolveAddress(address);
	return descriptor->Type == EfiConventionalMemory;
}

void MemoryMap::DumpMemoryMap()
{
	loading->WriteLineFormat("MapSize: 0x%x, DescSize: 0x%x", m_memoryMapSize, m_memoryMapDescriptorSize);
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = m_memoryMap;
		current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize);
		current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		loading->WriteLineFormat("A:%08x S:%08x T:%s V: %d N: 0x%x", (UINT32)current, current->PhysicalStart, (*MemTypes)[current->Type], current->VirtualStart, current->NumberOfPages);
	}
}

EFI_MEMORY_DESCRIPTOR* MemoryMap::ResolveAddress(EFI_PHYSICAL_ADDRESS address)
{
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = m_memoryMap;
		current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize);
		current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		if ((current->PhysicalStart <= address) && (((address - current->PhysicalStart) >> EFI_PAGE_SHIFT) < current->NumberOfPages))
			return current;
	}

	Fatal("ResolveAddress failed");
}
