#include "Kernel.h"
#include "Main.h"

uint64_t Kernel::GetSystemInfo(SystemInfo* info)
{
	info->PageSize = PAGE_SIZE;
	info->Architecture = SystemArchitecture::x64;
	return 1;
}

uint64_t Kernel::ExitProcess(uint32_t exitCode)
{
	KThread* current = m_scheduler->GetCurrentThread();
	UserThread* user = current->GetUserThread();
	Assert(user);

	UserProcess& process = user->GetProcess();
	process.Delete = true;
	
	m_scheduler->KillThread();
	return 0;
}

uint64_t Kernel::DebugPrint(char* s)
{
	Print(s);
	return 1;
}

