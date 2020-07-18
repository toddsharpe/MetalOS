#pragma once

#include <efi.h>

//
// System Configuration Table Definitions
//

#define ACPI_10_TABLE_GUID    \
    { 0xeb9d2d30, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

#define ACPI_20_TABLE_GUID  \
    { 0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} }

#define SMBIOS_TABLE_GUID    \
    { 0xeb9d2d31, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

#define SMBIOS3_TABLE_GUID    \
    { 0xf2fd1544, 0x9794, 0x4a2c, {0x99, 0x2e, 0xe5, 0xbb, 0xcf, 0x20, 0xe3, 0x94} }

static const EFI_GUID Acpi10TableGuid = ACPI_10_TABLE_GUID;
static const EFI_GUID Acpi20TableGuid = ACPI_20_TABLE_GUID;
static const EFI_GUID SmbiosTableGuid = SMBIOS_TABLE_GUID;
static const EFI_GUID Smbios3TableGuid = SMBIOS3_TABLE_GUID;

class System
{
public:
	System(EFI_CONFIGURATION_TABLE* ConfigurationTables, UINTN NumConfigTables);
	
	UINTN GetInstalledSystemRam();
	void DisplayTableIds();
	void DisplayAcpi2();

private:
	bool GetTableByGuid(const EFI_GUID* guid, void** vendorTable);

	EFI_CONFIGURATION_TABLE* m_configTables;
	UINTN m_configTablesCount;
};

