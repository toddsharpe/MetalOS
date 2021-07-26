#include "Kernel.h"
#include "Assert.h"
#include "System.h"

#include <tianocore-edk2/IndustryStandard/SmBios.h>
#define PACKED
#include <tianocore-edk2/IndustryStandard/PlatformAcpi.h>
#include <intrin.h>
#include <crt_string.h>


System::System(EFI_CONFIGURATION_TABLE* ConfigurationTables, UINTN NumConfigTables) :
	m_configTables(ConfigurationTables),
	m_configTablesCount(NumConfigTables)
{
}

//https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_2.7.1.pdf
//Hyper-v Reports the following table types: 0, 1, 3, 2, 4, 11, 16, 17, 19, 20, 32, 127
UINTN System::GetInstalledSystemRam()
{
	void* smbiosTable = nullptr;
	if (!GetTableByGuid(&Smbios3TableGuid, &smbiosTable))
		return 0;

	SMBIOS_TABLE_3_0_ENTRY_POINT* entry = (SMBIOS_TABLE_3_0_ENTRY_POINT*)smbiosTable;
	SMBIOS_STRUCTURE* header = (SMBIOS_STRUCTURE*)entry->TableAddress;
	SMBIOS_STRUCTURE* end = (SMBIOS_STRUCTURE*)(entry->TableAddress + entry->TableMaximumSize);

	UINT64 ram = 0;

	while (header < end)
	{
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

	kernel.Printf("RAM %d MB", ram / 1024 / 1024);
	return ram;
}

void System::DisplayTableIds()
{
	for (UINT32 i = 0; i < m_configTablesCount; i++)
	{
		EFI_GUID& guid = m_configTables[i].VendorGuid;

		kernel.Printf("Guid = {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}, Address: 0x%016x",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7],
			m_configTables[i].VendorTable
			);
	}
}

bool System::GetTableByGuid(const EFI_GUID* guid, void** vendorTable)
{
	for (UINT32 i = 0; i < m_configTablesCount; i++)
	{
		if (memcmp(&m_configTables[i].VendorGuid, guid, sizeof(EFI_GUID)) != 0)
			continue;

		*vendorTable = m_configTables[i].VendorTable;
		return true;
	}

	return false;
}

#pragma pack(push, 1)
struct EFI_ACPI_XSDT
{
	EFI_ACPI_DESCRIPTION_HEADER Header;
	uint64_t Tables[0];
};

struct EFI_ACPI_DSDT
{
	EFI_ACPI_DESCRIPTION_HEADER Header;

};
#pragma pack(pop)

//Not really needed since acpica is working
void System::DisplayAcpi2()
{
	void* table = nullptr;
	if (!GetTableByGuid(&Acpi20TableGuid, &table))
		return;

	EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER* entry = (EFI_ACPI_2_0_ROOT_SYSTEM_DESCRIPTION_POINTER*)table;
	char sig[9] = { 0 };
	memcpy(&sig, &entry->Signature, sizeof(entry->Signature));
	kernel.Printf("Sig: %s Version: %d Address: 0x%016x", sig, entry->Revision, entry->XsdtAddress);

	EFI_ACPI_XSDT* xdt = (EFI_ACPI_XSDT*)entry->XsdtAddress;
	size_t size = (xdt->Header.Length - sizeof(xdt->Header)) / sizeof(uintptr_t);
	kernel.Printf("  size: 0x%x", size);

	char oemId[7] = { 0 };
	memcpy(oemId, xdt->Header.OemId, 6);
	char tableId[9] = { 0 };
	memcpy(tableId, &xdt->Header.OemTableId, 8);
	kernel.Printf("  OemId: %s TableId %s", oemId, tableId);

	for (size_t i = 0; i < size; i++)
	{
		EFI_ACPI_DESCRIPTION_HEADER* header = (EFI_ACPI_DESCRIPTION_HEADER*)xdt->Tables[i];
		kernel.Printf("  %8x, Size: 0x%x, Revision: %d OemRevision: %d", header->Signature, header->Length, header->Revision, header->OemRevision);

		switch (header->Signature)
		{
			case EFI_ACPI_2_0_FIXED_ACPI_DESCRIPTION_TABLE_SIGNATURE:
			{
				Assert(header->Revision == EFI_ACPI_6_0_FIXED_ACPI_DESCRIPTION_TABLE_REVISION);
				EFI_ACPI_6_0_FIXED_ACPI_DESCRIPTION_TABLE* fadt6 = (EFI_ACPI_6_0_FIXED_ACPI_DESCRIPTION_TABLE*)xdt->Tables[i];
				Assert(fadt6->MinorVersion == EFI_ACPI_6_2_FIXED_ACPI_DESCRIPTION_TABLE_MINOR_REVISION);
				EFI_ACPI_6_2_FIXED_ACPI_DESCRIPTION_TABLE* fadt6_2 = (EFI_ACPI_6_2_FIXED_ACPI_DESCRIPTION_TABLE*)fadt6;
				kernel.Printf("    FADT 6.2 detected");

				char sig2[9] = { 0 };
				EFI_ACPI_DESCRIPTION_HEADER* xdstHeader = (EFI_ACPI_DESCRIPTION_HEADER*)fadt6_2->XDsdt;
				Assert(xdstHeader->Signature == EFI_ACPI_6_0_DIFFERENTIATED_SYSTEM_DESCRIPTION_TABLE_SIGNATURE);
				kernel.Printf("    xdstHeader %d sig 0x%x", xdstHeader->Revision, xdstHeader->Signature);
			}
			break;

			case EFI_ACPI_2_0_MULTIPLE_APIC_DESCRIPTION_TABLE_SIGNATURE:
			{

				Assert(header->Revision == EFI_ACPI_6_0_MULTIPLE_APIC_DESCRIPTION_TABLE_REVISION);
				EFI_ACPI_6_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER* madt6 = (EFI_ACPI_6_0_MULTIPLE_APIC_DESCRIPTION_TABLE_HEADER*)xdt->Tables[i];
				kernel.Printf("    MADT 6 detected");
			}
			break;
		}

		//Print("0x%016x 0x%016x", &xdt->Tables[i], xdt->Tables[i]);
		//uint8_t* p = (uint8_t*)&xdt->Tables[i - 5];
		//Print("0x%016x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", &xdt->Tables[i-5], p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
	}
}
