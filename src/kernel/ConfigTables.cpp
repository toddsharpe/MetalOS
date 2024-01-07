#include "ConfigTables.h"

#include "Assert.h"
#include <string>

ConfigTables::ConfigTables(const EFI_CONFIGURATION_TABLE* const configTables, const size_t count) :
	m_tables(configTables),
	m_count(count)
{

}

//Reallocates and copies config tables. Used to move from UEFI space to Kernel
void ConfigTables::Reallocate()
{
	EFI_CONFIGURATION_TABLE* const newTables = new EFI_CONFIGURATION_TABLE[m_count];
	Assert(newTables);

	memcpy(newTables, m_tables, m_count * sizeof(EFI_CONFIGURATION_TABLE));
	m_tables = newTables;
}

void* ConfigTables::GetAcpiTable() const
{
	void* table = nullptr;
	const EFI_GUID guid = ACPI_20_TABLE_GUID;
	Assert(GetTableByGuid(guid, table));

	return table;
}

bool ConfigTables::GetTableByGuid(const EFI_GUID& guid, void*& vendorTable) const
{
	for (size_t i = 0; i < m_count; i++)
	{
		if (memcmp(&m_tables[i].VendorGuid, &guid, sizeof(EFI_GUID)) != 0)
			continue;

		vendorTable = m_tables[i].VendorTable;
		return true;
	}

	return false;
}

void ConfigTables::Display() const
{
	for (size_t i = 0; i < m_count; i++)
	{
		const EFI_GUID guid = m_tables[i].VendorGuid;
		const void* address = m_tables[i].VendorTable;

		Printf("Guid: {%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}, Address: 0x%016x\n",
			guid.Data1, guid.Data2, guid.Data3,
			guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
			guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7],
			address
		);
	}
}
