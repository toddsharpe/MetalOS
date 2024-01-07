#include "MemoryMap.h"
#include "Assert.h"
#include <string>
#include <MetalOS.System.h>
#include <PageTables.h>

namespace
{
	// https://dox.ipxe.org/UefiMultiPhase_8h.html
	const char MemTypes[16][27] = {
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
}

MemoryMap::MemoryMap(const EFI_MEMORY_DESCRIPTOR* const table, const size_t size, const size_t descSize) :
	m_table(table),
	m_size(size),
	m_descSize(descSize)
{

}

//Reallocates and copies table. Used to move from UEFI space to Kernel
//While reallocating, reclaim memory from UEFI
void MemoryMap::Reallocate()
{
	const size_t tableSize = m_size * m_descSize;
	uint8_t* const buffer = new uint8_t[tableSize];
	Assert(buffer);

	EFI_MEMORY_DESCRIPTOR* const newTable = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(buffer);
	memcpy(newTable, m_table, tableSize);

	//Reclaim UEFI memory
	for (EFI_MEMORY_DESCRIPTOR* current = newTable;
		current < NextMemoryDescriptor(newTable, m_size);
		current = NextMemoryDescriptor(current, m_descSize))
	{
		switch (current->Type)
		{
			case EfiBootServicesCode:
			case EfiBootServicesData:
			case EfiLoaderCode:
				Assert((current->Attribute & EFI_MEMORY_RUNTIME) == 0);
				current->Type = EfiConventionalMemory;
		}
	}

	//Adopt new table
	m_table = newTable;
}

void MemoryMap::Display()
{
	Printf("MapSize: 0x%016X, Size: 0x%x DescSize: 0x%x\n", m_table, m_size, m_descSize);
	for (const EFI_MEMORY_DESCRIPTOR* current = m_table;
		current < NextMemoryDescriptor(m_table, m_size);
		current = NextMemoryDescriptor(current, m_descSize))
	{
		const bool runtime = (current->Attribute & EFI_MEMORY_RUNTIME) != 0;
		Printf("P: %016x V: %016x T:%s #: 0x%x A:0x%016x %c\n", current->PhysicalStart, current->VirtualStart, MemTypes[current->Type], current->NumberOfPages, current->Attribute, runtime ? 'R' : ' ');
	}
}

void MemoryMap::MapRuntime(PageTables& pageTables)
{
	for (const EFI_MEMORY_DESCRIPTOR* current = m_table;
		current < NextMemoryDescriptor(m_table, m_size);
		current = NextMemoryDescriptor(current, m_descSize))
	{
		if ((current->Attribute & EFI_MEMORY_RUNTIME) == 0)
			continue;

		Assert(pageTables.MapPages(current->VirtualStart, current->PhysicalStart, current->NumberOfPages, true));
	}
}

uintptr_t MemoryMap::GetPhysicalAddressSize() const
{
	uintptr_t highest = 0;

	for (const EFI_MEMORY_DESCRIPTOR* current = m_table;
		current < NextMemoryDescriptor(m_table, m_size);
		current = NextMemoryDescriptor(current, m_descSize))
	{
		uintptr_t address = current->PhysicalStart + (current->NumberOfPages << PageShift);
		if (address > highest)
			highest = address;
	}

	return highest;
}

uintptr_t MemoryMap::GetLargestConventionalAddress() const
{
	const EFI_MEMORY_DESCRIPTOR* largest = m_table;

	for (const EFI_MEMORY_DESCRIPTOR* current = m_table;
		current < NextMemoryDescriptor(m_table, m_size);
		current = NextMemoryDescriptor(current, m_descSize))
	{
		if (current->Type != EfiConventionalMemory)
			continue;

		if (current->NumberOfPages > largest->NumberOfPages)
			largest = current;
	}

	return largest->PhysicalStart;
}

size_t MemoryMap::Length() const
{
	return m_size / m_descSize;
}

EFI_MEMORY_DESCRIPTOR* MemoryMap::Get(size_t index) const
{
	Assert(index < Length());
	return MakePointer<EFI_MEMORY_DESCRIPTOR*>(m_table, index * m_descSize);
}
