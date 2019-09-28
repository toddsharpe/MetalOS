#include "System.h"

System::System(EFI_CONFIGURATION_TABLE* ConfigurationTables, UINTN NumConfigTables) : m_configTables(ConfigurationTables), m_configTablesCount(NumConfigTables)
{
}
