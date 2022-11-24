#pragma once

#include <efi.h>
#include <cstdint>

class MemoryMap
{
public:
	MemoryMap(const UINTN MemoryMapSize, const UINTN MemoryMapDescriptorSize, const UINT32 MemoryMapDescriptorVersion, const EFI_MEMORY_DESCRIPTOR& MemoryMap);

	void ReclaimBootPages() const;
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
		return MakePointer<EFI_MEMORY_DESCRIPTOR*>(m_memoryMap, index * m_memoryMapDescriptorSize);
	}

private:
	//Allocate the current size of the map plus space for more entries
	static const size_t BufferCount = 3;
	static const char MemTypes[16][27];

	EFI_MEMORY_DESCRIPTOR* ResolveAddress(EFI_PHYSICAL_ADDRESS address);

	UINTN m_memoryMapSize;
	const UINTN m_memoryMapMaxSize;
	const UINTN m_memoryMapDescriptorSize;
	const UINT32 m_memoryMapDescriptorVersion;
	EFI_MEMORY_DESCRIPTOR* m_memoryMap;
};

