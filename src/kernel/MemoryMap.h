#pragma once

#include <efi.h>
#include <cstdint>
#include "PageTables.h"

//NOTE(tsharpe): Descriptor sizes must come from UEFI. As observed:
// * m_descSize = 0x30
// * sizeof(EFI_MEMORY_DESCRIPTOR) = 0x28
class MemoryMap
{
public:
	MemoryMap(const EFI_MEMORY_DESCRIPTOR* const table, const size_t size, const size_t descSize);

	void Reallocate();
	void Display();
	void MapRuntime(PageTables& pageTables);

	uintptr_t GetPhysicalAddressSize() const;
	uintptr_t GetLargestConventionalAddress() const;

	//Iterator interface. Required since m_memoryMapDescriptorSize != sizeof(EFI_MEMORY_DESCRIPTOR)
	size_t Length() const;
	EFI_MEMORY_DESCRIPTOR* Get(size_t index) const;

private:
	const EFI_MEMORY_DESCRIPTOR* m_table;
	const size_t m_size;
	const size_t m_descSize;
};

