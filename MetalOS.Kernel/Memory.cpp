#include "Memory.h"
#include "LoadingScreen.h"
#include "LoaderParams.h"
#include "Main.h"

extern LOADER_PARAMS* pParams;
//extern Display display;
extern LoadingScreen* loading;

//TODO: pull out of this class, rename to memorymap
void Memory::memcpy(void* dest, void* source, UINT32 size)
{
	UINT64* dest_64 = (UINT64*)dest;
	UINT64* source_64 = (UINT64*)source;
	UINT32 chunkSize = 64;
	
	for (int i = 0; i < size / chunkSize; i++)
	{
		*dest_64 = *source_64;

		dest_64++;
		source_64++;
	}

	UINT8* dest_8 = (UINT8*)dest;
	UINT8* source_8 = (UINT8*)source;
	for (int i = 0; i < size % chunkSize; i++)
	{
		*dest_8 = *source_8;
		dest_8++;
		source_8++;
	}
}

int Memory::memcmp(const void* ptr1, const void* ptr2, UINT32 num)
{
	UINT64* ptr1_64 = (UINT64*)ptr1;
	UINT64* ptr2_64 = (UINT64*)ptr2;
	UINT32 chunkSize = 64;

	int i = 0;
	while (i < num / chunkSize)
	{
		if (*ptr1_64 < *ptr2_64)
			return -1;
		else if (*ptr1_64 > * ptr2_64)
			return 1;

		i++;
		ptr1_64++;
		ptr2_64++;
	}
	
	UINT8* ptr1_8 = (UINT8*)ptr1;
	UINT8* ptr2_8 = (UINT8*)ptr2;

	i = 0;
	while (i <  num % chunkSize)
	{
		if (*ptr1_8 < *ptr2_8)
			return -1;
		else if (*ptr1_8 > * ptr2_8)
			return 1;

		i++;
		ptr1_8++;
		ptr2_8++;
	}

	return 0;
}

Memory::Memory(UINTN MemoryMapSize, UINTN MemoryMapDescriptorSize, UINT32 MemoryMapVersion, EFI_MEMORY_DESCRIPTOR* MemoryMap,
	EFI_PHYSICAL_ADDRESS kernelBaseAddress, UINTN kernelPageCount)
	: m_memoryMapSize(MemoryMapSize), m_memoryMapDescriptorSize(MemoryMapDescriptorSize), m_memoryMapVersion(MemoryMapVersion), m_memoryMap(MemoryMap),
	m_kernelBaseAddress(kernelBaseAddress), m_kernelPageCount(kernelPageCount)
{

}

void Memory::ReclaimBootPages()
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
void Memory::MergeConventionalPages()
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
		Memory::memcpy(&m_memoryMap[i + 1], &m_memoryMap[j], m_memoryMapDescriptorSize * (descriptorCount - j));
		descriptorCount -= (j - i);
	}


	//Find entry that contains memory map, calculate reduction in pages and decrement
	

	//Find entry just after this one. If its free (conventional) decrement the starting address
	//If its not, then insert a new one
}


void Memory::DumpMemoryMap()
{
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = m_memoryMap;
		current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize);
		current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		loading->WriteLineFormat("S:%08x T:%s V:%x N:%x", current->PhysicalStart, (*MemTypes)[current->Type], current->VirtualStart, current->NumberOfPages);
	}
}

EFI_MEMORY_DESCRIPTOR* Memory::ResolveAddress(EFI_PHYSICAL_ADDRESS address)
{
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = m_memoryMap;
		current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize);
		current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		if ((current->PhysicalStart <= address) &&
			(((address - current->PhysicalStart) >> EFI_PAGE_SHIFT) < current->NumberOfPages))
			return current;
	}

	Fatal("ResolveAddress failed");
}
