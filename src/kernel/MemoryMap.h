#pragma once

#include <efi.h>
#include "core_crt/stdint.h"
#include "x64/PageTables.h"
#include "Lib/System.h"
#include "kernel/VirtualMemoryManager.h"

//NOTE(tsharpe): Descriptor sizes must come from UEFI. As observed:
// * m_descSize = 0x30
// * sizeof(EFI_MEMORY_DESCRIPTOR) = 0x28
class MemoryMap
{
public:
	MemoryMap(const EFI_MEMORY_DESCRIPTOR* const table, const size_t size, const size_t descSize) :
		m_table(table),
		m_size(size),
		m_descSize(descSize)
	{

	}

	//Reallocates and copies table. Used to move from UEFI space to Kernel
	//While reallocating, reclaim memory from UEFI
	template <typename T>
	void Initialize(T& arena)
	{
		//Reallocate table to kernel space
		EFI_MEMORY_DESCRIPTOR* const allocated = reinterpret_cast<EFI_MEMORY_DESCRIPTOR*>(arena.Allocate(m_size));
		Assert(allocated);
		memcpy(allocated, m_table, m_size);

		//Mark UEFI memory as available
		for (EFI_MEMORY_DESCRIPTOR* current = allocated;
			current < NextMemoryDescriptor(allocated, m_size);
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
		m_table = allocated;
	}

	void MapRuntime(KProcess& process)
	{
		for (const EFI_MEMORY_DESCRIPTOR* current = m_table;
			current < NextMemoryDescriptor(m_table, m_size);
			current = NextMemoryDescriptor(current, m_descSize))
		{
			if ((current->Attribute & EFI_MEMORY_RUNTIME) == 0)
				continue;

			Assert(VMM::MapContiguous(process, (void*)current->VirtualStart, current->PhysicalStart, current->NumberOfPages));
		}
	}

	void Display() const
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

	uintptr_t GetPhysicalAddressSize() const
	{
		uintptr_t highest = 0;

		for (const EFI_MEMORY_DESCRIPTOR* current = m_table;
			current < NextMemoryDescriptor(m_table, m_size);
			current = NextMemoryDescriptor(current, m_descSize))
		{
			uintptr_t address = current->PhysicalStart + (current->NumberOfPages << x64::PageShift);
			if (address > highest)
				highest = address;
		}

		return highest;
	}

	uintptr_t GetLargestConventionalAddress() const
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

	//Iterator interface. Required since m_memoryMapDescriptorSize != sizeof(EFI_MEMORY_DESCRIPTOR)
	size_t Length() const
	{
		return m_size / m_descSize;
	}

	//NOTE(tsharpe): Because DescSize != sizeof(EFI_MEMORY_DESCRIPTOR), must use pointer math
	EFI_MEMORY_DESCRIPTOR& operator[](const size_t index) const
	{
		Assert(index < Length());
		return *MakePointer<EFI_MEMORY_DESCRIPTOR*>(m_table, index * m_descSize);
	}

private:
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

	const EFI_MEMORY_DESCRIPTOR* m_table;
	const size_t m_size;
	const size_t m_descSize;
};

