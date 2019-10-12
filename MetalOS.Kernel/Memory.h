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
class Memory
{
public:
	static void memcpy(void* dest, void* source, UINT32 size);
	static int memcmp(const void* ptr1, const void* ptr2, UINT32 num);

	Memory(UINTN MemoryMapSize, UINTN MemoryMapDescriptorSize, UINT32 MemoryMapVersion, EFI_MEMORY_DESCRIPTOR* MemoryMap,
		EFI_PHYSICAL_ADDRESS kernelBaseAddress, UINTN kernelPageCount);

	void ReclaimBootPages();
	void MergeConventionalPages();
	EFI_PHYSICAL_ADDRESS AllocatePhysicalPages(UINT32 count);
	void DumpMemoryMap();

private:
	EFI_MEMORY_DESCRIPTOR* ResolveAddress(EFI_PHYSICAL_ADDRESS address);

	const char(*MemTypes)[16][27] = &mem_types;

	UINTN m_memoryMapSize;
	UINTN m_memoryMapDescriptorSize;
	UINT32 m_memoryMapVersion;
	EFI_MEMORY_DESCRIPTOR* m_memoryMap;

	//Kernel allocation
	EFI_PHYSICAL_ADDRESS m_kernelBaseAddress;
	UINTN m_kernelPageCount;
};

