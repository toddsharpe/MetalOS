#pragma once

#define GNU_EFI_SETJMP_H
#include <efi.h>
#include <cstdint>

// https://dox.ipxe.org/UefiMultiPhase_8h.html
const char mem_types[16][27] = {
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

#define PrevMemoryDescriptor(Ptr,Size)  ((EFI_MEMORY_DESCRIPTOR *) (((UINT8 *) Ptr) - Size))
#define MakePtr( cast, ptr, addValue ) (cast)( (uintptr_t)(ptr) + (uintptr_t)(addValue))
class MemoryMap
{
public:
	//Allocate the current size of the map plus space for more entries
	static const size_t BufferCount = 3;

	MemoryMap(UINTN MemoryMapSize, UINTN MemoryMapDescriptorSize, UINT32 MemoryMapDescriptorVersion, EFI_MEMORY_DESCRIPTOR* MemoryMap);

	void ReclaimBootPages();
	void MergeConventionalPages();
	EFI_PHYSICAL_ADDRESS AllocatePages(UINT32 count);

	bool AddressFree(EFI_PHYSICAL_ADDRESS address);

	void DumpMemoryMap();

	UINTN GetPhysicalAddressSize();
	UINTN GetLargestConventionalAddress();
	bool IsConventional(UINTN address);

	UINTN GetVirtualAddress(EFI_PHYSICAL_ADDRESS address) const;

	//Iterator interface
	//m_memoryMapDescriptorSize != sizeof(EFI_MEMORY_DESCRIPTOR)
	size_t Length()
	{
		return m_memoryMapSize / m_memoryMapDescriptorSize;
	}

	EFI_MEMORY_DESCRIPTOR* Get(size_t index)
	{
		//Assert(index < Length());
		return MakePtr(EFI_MEMORY_DESCRIPTOR*, m_memoryMap, index * m_memoryMapDescriptorSize);
	}

private:
	EFI_MEMORY_DESCRIPTOR* ResolveAddress(EFI_PHYSICAL_ADDRESS address);

	const char(*MemTypes)[16][27] = &mem_types;

	UINTN m_memoryMapSize;
	UINTN m_memoryMapMaxSize;
	UINTN m_memoryMapDescriptorSize;
	UINT32 m_memoryMapDescriptorVersion;
	EFI_MEMORY_DESCRIPTOR* m_memoryMap;
};

