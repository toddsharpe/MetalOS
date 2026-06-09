#pragma once

#include "Boot/EfiMemory.h"
#include "Boot/EfiPrint.h"
#include "MetalOS.Space.h"
#include "x64/x64.h"
#include "Assert.h"

extern EFI_BOOT_SERVICES* BS;

namespace EfiMemory
{
	//Static storage for the UEFI memory map (one page easily holds Hyper-V's map),
	//avoiding a boot-services allocation. The kernel never consumes the EFI map.
	alignas(EFI_MEMORY_DESCRIPTOR) static uint8_t MemoryMapBuffer[x64::PageSize];

	EFI_STATUS GetMemoryMap(EFI_MEMORY_MAP& map, UINTN& mapKey, UINT32& descriptorVersion)
	{
		map.Table = (EFI_MEMORY_DESCRIPTOR*)MemoryMapBuffer;
		map.Size = sizeof(MemoryMapBuffer);
		return BS->GetMemoryMap(&map.Size, map.Table, &mapKey, &map.DescriptorSize, &descriptorVersion);
	}

	UINTN GetPhysicalAddressSize(const EFI_MEMORY_MAP& map)
	{
		UINTN highest = 0;

		for (const EFI_MEMORY_DESCRIPTOR* current = map.Table;
			current < NextMemoryDescriptor(map.Table, map.Size);
			current = NextMemoryDescriptor(current, map.DescriptorSize))
		{
			const uintptr_t address = current->PhysicalStart + (current->NumberOfPages << x64::PageShift);
			if (address > highest)
				highest = address;
		}

		return highest;
	}

	void BuildRanges(EFI_MEMORY_MAP& map, LoaderParams& params)
	{
		size_t freeCount = 0;
		size_t runtimeCount = 0;
		for (EFI_MEMORY_DESCRIPTOR* current = map.Table;
			current < NextMemoryDescriptor(map.Table, map.Size);
			current = NextMemoryDescriptor(current, map.DescriptorSize))
		{
			//Runtime regions are mapped by the kernel at KernelUefiStart + physical
			if (current->Attribute & EFI_MEMORY_RUNTIME)
			{
				current->VirtualStart = current->PhysicalStart + KernelUefiStart;
				Assert(runtimeCount < MaxMemoryRanges);
				params.RuntimeRanges[runtimeCount++] = { current->PhysicalStart, current->NumberOfPages };
				continue;
			}

			//Memory the kernel allocator may use. Boot-services memory is reclaimable
			//now; loader data (PFN DB, kernel, ramdrive, these arrays) stays reserved.
			switch (current->Type)
			{
				case EfiConventionalMemory:
				case EfiBootServicesCode:
				case EfiBootServicesData:
				case EfiLoaderCode:
					Assert(freeCount < MaxMemoryRanges);
					params.FreeRanges[freeCount++] = { current->PhysicalStart, current->NumberOfPages };
					break;
			}
		}
		//Arrays are zero-initialized, so the unwritten entry past each list is the
		//{0, 0} sentinel the kernel iterates to.
	}

	EFI_STATUS DumpMemoryMap(const EFI_MEMORY_MAP& map)
	{
		const CHAR16 mem_types[16][27] = {
		  L"EfiReservedMemoryType     ",
		  L"EfiLoaderCode             ",
		  L"EfiLoaderData             ",
		  L"EfiBootServicesCode       ",
		  L"EfiBootServicesData       ",
		  L"EfiRuntimeServicesCode    ",
		  L"EfiRuntimeServicesData    ",
		  L"EfiConventionalMemory     ",
		  L"EfiUnusableMemory         ",
		  L"EfiACPIReclaimMemory      ",
		  L"EfiACPIMemoryNVS          ",
		  L"EfiMemoryMappedIO         ",
		  L"EfiMemoryMappedIOPortSpace",
		  L"EfiPalCode                ",
		  L"EfiPersistentMemory       ",
		  L"EfiMaxMemoryType          "
		};

		UartPrintf(L"MapSize: 0x%016X, Size: 0x%x, DescSize: 0x%x\n", map.Table, map.Size, map.DescriptorSize);

		for (EFI_MEMORY_DESCRIPTOR* current = map.Table; current < NextMemoryDescriptor(map.Table, map.Size); current = NextMemoryDescriptor(current, map.DescriptorSize))
		{
			const bool runtime = (current->Attribute & EFI_MEMORY_RUNTIME) != 0;
			UartPrintf(L"P: %016x V: %016x T:%s #: 0x%x A:0x%016x %c\n", current->PhysicalStart, current->VirtualStart, mem_types[current->Type], current->NumberOfPages, current->Attribute, runtime ? 'R' : ' ');
		}

		return EFI_SUCCESS;
	}
}
