#pragma once

#define GNU_EFI_SETJMP_H
#include <efi.h>

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

//#define NextMemoryDescriptor(Ptr,Size)  ((EFI_MEMORY_DESCRIPTOR *) (((UINT8 *) Ptr) + Size))
#define PrevMemoryDescriptor(Ptr,Size)  ((EFI_MEMORY_DESCRIPTOR *) (((UINT8 *) Ptr) - Size))
class MemoryMap
{
public:
	MemoryMap(UINTN MemoryMapSize, UINTN MemoryMapDescriptorSize, UINT32 MemoryMapVersion, EFI_MEMORY_DESCRIPTOR* MemoryMap) :
		MemoryMap(MemoryMapSize, MemoryMapDescriptorSize, MemoryMapVersion, MemoryMap, MemoryMapSize) {	}
	MemoryMap(UINTN MemoryMapSize, UINTN MemoryMapDescriptorSize, UINT32 MemoryMapVersion, EFI_MEMORY_DESCRIPTOR* MemoryMap, UINTN maxSize);

	void ReclaimBootPages();
	void MergeConventionalPages();
	EFI_PHYSICAL_ADDRESS AllocatePages(UINT32 count);

	bool AddressFree(EFI_PHYSICAL_ADDRESS address);

	void DumpMemoryMap();

private:
	EFI_MEMORY_DESCRIPTOR* ResolveAddress(EFI_PHYSICAL_ADDRESS address);

	const char(*MemTypes)[16][27] = &mem_types;

	UINTN m_memoryMapSize;
	UINTN m_memoryMapDescriptorSize;
	UINT32 m_memoryMapVersion;
	EFI_MEMORY_DESCRIPTOR* m_memoryMap;

	UINTN m_maxSize;
};

