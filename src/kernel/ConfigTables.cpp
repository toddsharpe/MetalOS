#include "kernel/ConfigTables.h"
#include "Assert.h"

//Copy tables to kernel memory
void ConfigTables::Initialize(const EFI_CONFIGURATION_TABLE* const configTables, const size_t count)
{
	const size_t size = sizeof(EFI_CONFIGURATION_TABLE) * count;
	EFI_CONFIGURATION_TABLE* const allocated = reinterpret_cast<EFI_CONFIGURATION_TABLE*>(KeAlloc(size, AllocType::Boot));
	Assert(allocated);

	memcpy(allocated, configTables, size);
	m_tables = allocated;
	m_count = count;
}

void* ConfigTables::GetAcpiTable() const
{
	void* table = nullptr;
	constexpr EFI_GUID guid = { 0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} };
	Assert(GetTableByGuid(guid, table));

	return table;
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
