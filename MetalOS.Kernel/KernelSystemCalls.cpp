#include "Kernel.h"
#include "Main.h"

uint64_t Kernel::GetSystemInfo(SystemInfo* info)
{
	info->PageSize = PAGE_SIZE;
	info->Architecture = SystemArchitecture::x64;
	return 1;
}

uint64_t Kernel::DebugPrint(char* s)
{
	Print(s);
	return 1;
}

