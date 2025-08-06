#pragma once

#include <efi.h>
#include "Lib/Arena.h"
#include "kernel/MemoryMap.h"

class ConfigTables
{
public:
	ConfigTables(const EFI_CONFIGURATION_TABLE* const configTables, const size_t count);

	void Initialize(Arena& arena);

	void* GetAcpiTable() const;

	void Display() const;

private:
	bool GetTableByGuid(const EFI_GUID& guid, void*& vendorTable) const;

	const EFI_CONFIGURATION_TABLE* m_tables;
	const size_t m_count;
};
