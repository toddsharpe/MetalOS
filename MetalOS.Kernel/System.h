#pragma once

#include <efi.h>

class System
{
public:
	System(EFI_CONFIGURATION_TABLE* ConfigurationTables, UINTN NumConfigTables);

private:
	EFI_CONFIGURATION_TABLE* m_configTables;
	UINTN m_configTablesCount;
};

