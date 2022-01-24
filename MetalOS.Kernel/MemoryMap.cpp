#include "Kernel.h"
#include "Assert.h"

#include <crt_string.h>
#include "MetalOS.Kernel.h"
#include "MemoryMap.h"

// https://dox.ipxe.org/UefiMultiPhase_8h.html
const char MemoryMap::MemTypes[16][27] = {
	  "EfiReservedMemoryType     ",
	  "EfiLoaderCode             ",
	  "EfiLoaderData             ",
	  "EfiBootServicesCode       ",
	  "EfiBootServicesData       ",
	  "EfiRuntimeServicesCode    ",
	  "EfiRuntimeServicesData    ",
	  "EfiConventionalMemory     ",
	  "EfiUnusableMemory         ",
	  "EfiACPIReclaimMemory      ",
	  "EfiACPIMemoryNVS          ",
	  "EfiMemoryMappedIO         ",
	  "EfiMemoryMappedIOPortSpace",
	  "EfiPalCode                ",
	  "EfiPersistentMemory       ",
	  "EfiMaxMemoryType          "
};

MemoryMap::MemoryMap(const UINTN MemoryMapSize, const UINTN MemoryMapDescriptorSize, const UINT32 MemoryMapDescriptorVersion, const EFI_MEMORY_DESCRIPTOR& MemoryMap) :
	m_memoryMapSize(MemoryMapSize),
	m_memoryMapMaxSize(MemoryMapSize + BufferCount * MemoryMapDescriptorSize),
	m_memoryMapDescriptorSize(MemoryMapDescriptorSize),
	m_memoryMapDescriptorVersion(MemoryMapDescriptorVersion)
{
	Assert(MemoryMapSize % MemoryMapDescriptorSize == 0);

	//Allocate copy of memory map
	m_memoryMap = (EFI_MEMORY_DESCRIPTOR*)operator new(m_memoryMapMaxSize);
	Assert(m_memoryMap);
	memcpy(m_memoryMap, &MemoryMap, MemoryMapSize);
}

void MemoryMap::ReclaimBootPages() const
{
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = m_memoryMap;
		current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize);
		current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		if ((current->Type == EfiBootServicesCode) || (current->Type == EfiBootServicesData) || (current->Type == EfiLoaderCode))
		{
			Assert((current->Attribute & EFI_MEMORY_RUNTIME) == 0);
			current->Type = EfiConventionalMemory;
		}
	}
}

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
			//Look ahead for Conventional memory and condense entry if its sequential
			EFI_MEMORY_DESCRIPTOR* next = NextMemoryDescriptor(current, m_memoryMapDescriptorSize);
			while (next->Type == EfiConventionalMemory && next->PhysicalStart == current->PhysicalStart + current->NumberOfPages * PAGE_SIZE)
			{
				destination->NumberOfPages += next->NumberOfPages;
				current = next;
				next = NextMemoryDescriptor(current, m_memoryMapDescriptorSize);
			}
		}
	}

	//Zero out remaining entries
	uintptr_t newSize = (uintptr_t)destination - (uintptr_t)this->m_memoryMap;
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
		Assert(m_memoryMapSize + m_memoryMapDescriptorSize <= m_memoryMapMaxSize);

		UINTN index = (UINTN)(m_memoryMap - current) / m_memoryMapDescriptorSize;
		UINTN length = m_memoryMapSize / m_memoryMapDescriptorSize;

		//Copy all subsequent records down one
		for (UINTN i = length - 1; i >= index; i--)
		{
			void* destination = MakePointer<void*>(m_memoryMap, m_memoryMapDescriptorSize * (i + 1));
			void* source = MakePointer<void*>(m_memoryMap, m_memoryMapDescriptorSize * i);
			memcpy(destination, source, m_memoryMapDescriptorSize);
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
	kernel.Printf("MapSize: 0x%x, DescSize: 0x%x\n", m_memoryMapSize, m_memoryMapDescriptorSize);
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = m_memoryMap;
		current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize);
		current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		const bool runtime = (current->Attribute & EFI_MEMORY_RUNTIME) != 0;
		kernel.Printf("P: %016x V: %016x T:%s #: 0x%x A:0x%016x %c\n", current->PhysicalStart, current->VirtualStart, MemTypes[current->Type], current->NumberOfPages, current->Attribute, runtime ? 'R' : ' ');
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

UINTN MemoryMap::GetPhysicalAddressSize()
{
	UINTN highest = 0;

	EFI_MEMORY_DESCRIPTOR* current;
	for (current = m_memoryMap; current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize); current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		uintptr_t address = current->PhysicalStart + (current->NumberOfPages << EFI_PAGE_SHIFT);
		if (address > highest)
			highest = address;
	}

	return highest;
}

UINTN MemoryMap::GetLargestConventionalAddress()
{
	EFI_MEMORY_DESCRIPTOR* largest = m_memoryMap;

	EFI_MEMORY_DESCRIPTOR* current;
	for (current = m_memoryMap; current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize); current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		if (current->Type != EfiConventionalMemory)
			continue;

		if (current->NumberOfPages > largest->NumberOfPages)
			largest = current;
	}

	return largest->PhysicalStart;
}

//TODO: maybe refactor out a "find descriptor" method
bool MemoryMap::IsConventional(UINTN address)
{
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = m_memoryMap; current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize); current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		if (current->Type != EfiConventionalMemory)
			continue;
		
		if ((address >= current->PhysicalStart) && (address < current->PhysicalStart + (current->NumberOfPages << EFI_PAGE_SHIFT)))
			return true;
	}

	return false;
}

UINTN MemoryMap::GetVirtualAddress(EFI_PHYSICAL_ADDRESS address) const
{
	EFI_MEMORY_DESCRIPTOR* current;
	for (current = m_memoryMap; current < NextMemoryDescriptor(m_memoryMap, m_memoryMapSize); current = NextMemoryDescriptor(current, m_memoryMapDescriptorSize))
	{
		const UINTN end = current->PhysicalStart + (current->NumberOfPages << EFI_PAGE_SHIFT);
		if ((address >= current->PhysicalStart) && (address < end))
			return current->VirtualStart + (address - current->PhysicalStart);
	}

	return 0;
}
