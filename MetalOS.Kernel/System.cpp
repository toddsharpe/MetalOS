#include "System.h"
#include "LoadingScreen.h"
#include "Memory.h"
#include <SmBios.h>
#include <intrin.h>
#include "Kernel.h"
#include "CRT.h"

extern LoadingScreen* loading;

System::System(EFI_CONFIGURATION_TABLE* ConfigurationTables, UINTN NumConfigTables) : m_configTables(ConfigurationTables), m_configTablesCount(NumConfigTables)
{
}

//https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_2.7.1.pdf
//Hyper-v Reports the following table types: 0, 1, 3, 2, 4, 11, 16, 17, 19, 20, 32, 127
UINTN System::GetInstalledSystemRam()
{
	for (UINT32 i = 0; i < m_configTablesCount; i++)
	{
		EFI_GUID& guid = m_configTables[i].VendorGuid;
		
		loading->WriteLineFormat("Guid = {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

		if (CRT::memcmp(&m_configTables[i].VendorGuid, &Smbios3TableGuid, sizeof(EFI_GUID)) != 0)
			continue;

		SMBIOS_TABLE_3_0_ENTRY_POINT* entry = (SMBIOS_TABLE_3_0_ENTRY_POINT*)m_configTables[i].VendorTable;
		SMBIOS_STRUCTURE* header = (SMBIOS_STRUCTURE*)entry->TableAddress;
		SMBIOS_STRUCTURE* end = (SMBIOS_STRUCTURE*)(entry->TableAddress + entry->TableMaximumSize);

		UINT64 ram = 0;

		while (header < end)
		{
			loading->WriteLineFormat("T: %d, L: %d", header->Type, header->Length);
			if (header->Type == 17)
			{
				UINT16 socketSize = ((SMBIOS_TABLE_TYPE17*)header)->Size;
				if (socketSize == 0x7FFF)
				{
					ram += (UINT64)(((SMBIOS_TABLE_TYPE17*)header)->ExtendedSize) << 20;
				}
				else if (socketSize != 0xFFFF)
				{
					if (socketSize & 0x8000)
						ram += (UINT64)socketSize << 10; //KB
					else
						ram += (UINT64)socketSize << 20; //MB
				}
			}
			
			//Increment to next structure
			header = (SMBIOS_STRUCTURE*)((UINT64)header + header->Length);
			while (*(UINT16*)header != 0x0000)
			{
				header = (SMBIOS_STRUCTURE*)((UINT64)header + 1);
			}
			header = (SMBIOS_STRUCTURE*)((UINT64)header + 2); // Found end of current structure, move to start of next one
			continue;
		}

		loading->WriteLineFormat("RAM %d", ram / 1024 / 1024);
		return ram;
	}

	return 0;
}

bool System::IsPagingEnabled()
{
	UINT64 cr0 = __readcr0();
	loading->WriteLineFormat("%x", (UINT32)cr0);
	return ((cr0 & 0x80000000) != 0);
}

//TODO: use page size constants here
UINT64 System::ResolveAddress(UINT64 virtualAddress)
{
	loading->WriteLineFormat("Resolving: 0x%16x", virtualAddress);
	UINT32 offset = virtualAddress & 0xFFF;
	UINT32 l1Index = (virtualAddress >> 12) & 0x1FF;
	UINT32 l2Index = (virtualAddress >> 21) & 0x1FF;
	UINT32 l3Index = (virtualAddress >> 30) & 0x1FF;
	UINT32 l4Index = (virtualAddress >> 39) & 0x1FF;
	loading->WriteLineFormat("L4: %x L3: %x L2: %x L1: %x O: %x", l4Index, l3Index, l2Index, l1Index, offset);
	
	UINT64 cr3 = __readcr3() & ~0xFFF;
	PPML4E l4 = (PPML4E)cr3;
	loading->WriteLineFormat("L4: 0x%16x", l4);
	
	for (int i = l4Index; i < l4Index + 5; i++)
	{
		const PPML4E& current = &l4[i];
		loading->WriteLineFormat("  L4: 0x%16x - %d", current->Value, i);
	}
	
	PPDPTE l3 = (PPDPTE)(l4[l4Index].Value & ~0xFFF);
	loading->WriteLineFormat("L3: 0x%16x", l3);
	
	
	
	for (int i = l3Index; i < l3Index + 5; i++)
	{
		const PPDPTE& current = &l3[i];
		loading->WriteLineFormat("  L3: 0x%16x - %d", current->Value, i);
	}
	

	PPDE l2 = (PPDE)(l3[l3Index].Value & ~0xFFF);
	loading->WriteLineFormat("L2: 0x%16x - 0x%16x", l2, l2[l2Index].Value);
	
	
	for (int i = l2Index; i < l2Index + 5; i++)
	{
		const PPDE& current = &l2[i];
		loading->WriteLineFormat("  L2: 0x%16x - %d", current->Value, i);
	}
	

	if (l2[l2Index].PageSize == 1)
	{
		//L2 maps a 2MB page
		return (l2[l2Index].Value & ~0x1FFFFF) | (virtualAddress & 0x1FFFFF);
	}
	
	//L2 maps a 4k page

	PPTE l1 = (PPTE)(l2[l2Index].Value & ~0xFFF);
	//loading->WriteLineFormat("L1: 0x%16x", l1);
	/*
	for (int i = l1Index; i < l1Index + 5; i++)
	{
		const PPTE& current = &l1[i];
		loading->WriteLineFormat("  L1: 0x%16x - %d", current->Value, i);
	}
	*/
	
	return (l1[l1Index].Value & ~0xFFF) + offset;
}
