#include "Kernel.h"
#include "Assert.h"

#include "ConfigTables.h"

ConfigTables::ConfigTables(EFI_CONFIGURATION_TABLE* ConfigurationTables, UINTN NumConfigTables) :
	m_tables(new EFI_CONFIGURATION_TABLE[NumConfigTables]),
	m_count(NumConfigTables)
{
	Assert(m_tables != nullptr);

	memcpy(m_tables, ConfigurationTables, NumConfigTables * sizeof(EFI_CONFIGURATION_TABLE));
}

void* ConfigTables::GetAcpiTable() const
{
	void* table = nullptr;
	EFI_GUID guid = ACPI_20_TABLE_GUID;
	Assert(GetTableByGuid(&guid, &table));

	return table;
}

bool ConfigTables::GetTableByGuid(const EFI_GUID* guid, void** vendorTable) const
{
	for (UINT32 i = 0; i < m_count; i++)
	{
		if (memcmp(&m_tables[i].VendorGuid, guid, sizeof(EFI_GUID)) != 0)
			continue;

		*vendorTable = m_tables[i].VendorTable;
		return true;
	}

	return false;
}

void ConfigTables::Dump() const
{
	for (UINT32 i = 0; i < m_count; i++)
	{
		const EFI_GUID guid = m_tables[i].VendorGuid;
		const void* address = m_tables[i].VendorTable;

		kernel.Printf("Guid: {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}, Address: 0x%016x\n",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7],
			address
		);
	}
}
