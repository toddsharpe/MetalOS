#include "MemoryMap.h"
#include "LoadingScreen.h"
#include "LoaderParams.h"
#include "Main.h"
#include "CRT.h"
#include "Kernel.h"

#include <string.h>

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

//BUG: seems memory map isn't totally sequential
//Note: m_memoryMapDescriptorSize (0x30) is not the same size as EFI_MEMORY_DESCRIPTOR (0x28)
void MemoryMap::MergeConventionalPages()
{
	EFI_MEMORY_DESCRIPTOR* destination;;
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = this->m_memoryMap,
			destination = this->m_memoryMap;
		current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize);
		current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize),
			destination = NextMemoryDescriptor(destination, m_memoryMapDescriptorSize))
	{
		//Copy over entry
		memcpy(destination, current, this->m_memoryMapDescriptorSize);

		if (current->Type == EfiConventionalMemory)
		{
			//Look ahead for Conventional memory and condense entry
			EFI_MEMORY_DESCRIPTOR* next = NextMemoryDescriptor(current, m_memoryMapDescriptorSize);
			while (next->Type == EfiConventionalMemory)
			{
				destination->NumberOfPages += next->NumberOfPages;
				current = next;
				next = NextMemoryDescriptor(current, m_memoryMapDescriptorSize);
			}
		}
	}

	//Zero out remaining entries
	UINT32 newSize = (UINT64)destination - (UINT64)this->m_memoryMap;
	memset(destination, this->m_memoryMapSize - newSize, 0);

	//Update size
	this->m_memoryMapSize = newSize;
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

		UINT32 index = (UINTN)(m_memoryMap - current) / m_memoryMapDescriptorSize;
		UINT32 length = m_memoryMapSize / m_memoryMapDescriptorSize;

		//Copy all subsequent records down one
		for (UINT32 i = length - 1; i >= index; i--)
		{
			memcpy(MakePtr(void*, m_memoryMap, m_memoryMapDescriptorSize * (i + 1)), MakePtr(void*, m_memoryMap, m_memoryMapDescriptorSize * i), m_memoryMapDescriptorSize);
		}
		m_memoryMapSize += m_memoryMapDescriptorSize;
		
		//Fix up next record
		EFI_MEMORY_DESCRIPTOR* next = NextMemoryDescriptor(current, m_memoryMapDescriptorSize);
		next->PhysicalStart = current->PhysicalStart + ((UINT64)count * EFI_PAGE_SIZE);
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
		loading->WriteLineFormat("Physical: %16x Virtual: %16x T:%s NumPages: 0x%x", current->PhysicalStart, current->VirtualStart, (*MemTypes)[current->Type], current->NumberOfPages);
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
