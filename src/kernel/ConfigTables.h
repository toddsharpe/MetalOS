#pragma once

#include <efi.h>
#include "kernel/Api.h"

class ConfigTables
{
public:
	constexpr ConfigTables() :
		m_tables(),
		m_count()
	{

	}

	void Initialize(const EFI_CONFIGURATION_TABLE* const configTables, const size_t count);

	void* GetAcpiTable() const;

	void Display() const;

private:
	bool GetTableByGuid(const EFI_GUID& guid, void*& vendorTable) const;

	const EFI_CONFIGURATION_TABLE* m_tables;
	size_t m_count;
};
