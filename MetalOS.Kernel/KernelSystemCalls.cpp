#include "Kernel.h"
#include "Main.h"
#include "UserWindow.h"

//TODO: kernel method to validate user pointer

uint64_t Kernel::GetSystemInfo(SystemInfo* info)
{
	if (!info)
		return SystemCallResult::Failed;
	
	info->PageSize = PAGE_SIZE;
	info->Architecture = SystemArchitecture::x64;
	return SystemCallResult::Success;
}

uint64_t Kernel::ExitProcess(uint32_t exitCode)
{
	UserProcess& process = m_scheduler->GetCurrentProcess();
	
	m_scheduler->KillThread();
	return SystemCallResult::Success;
}

uint64_t Kernel::DebugPrint(char* s)
{
	Print(s);
	return SystemCallResult::Success;
}

uint32_t Kernel::CreateWindow(const char* name)
{
	if (!name)
		return SystemCallResult::Failed;

	//One window at at time
	Assert(Window == nullptr);

	Rectangle rect = { 0 };
	rect.P1.X = 0;
	rect.P1.Y = 0;
	rect.P2.X = m_display->GetWidth();
	rect.P2.Y = m_display->GetHeight();

	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);

	UserWindow* window = new UserWindow(name, rect, *user);
	user->Window = window;
	this->Window = window;

	return SystemCallResult::Success;
}

uint32_t Kernel::GetWindowRect(Handle handle, Rectangle* rect)
{
	if (!rect)
		return SystemCallResult::Failed;

	UserThread* user = m_scheduler->GetCurrentUserThread();
	if (!user->Window)
		return SystemCallResult::Failed;

	//Copy rectangle
	*rect = user->Window->GetRectangle();

	return SystemCallResult::Success;
}

//Blocks until message and returns it
uint32_t Kernel::GetMessage(Message* message)
{
	if (!message)
		return SystemCallResult::Failed;

	UserThread* user = m_scheduler->GetCurrentUserThread();
	if (!user->Window)
		return SystemCallResult::Failed;

	Message* msg = m_scheduler->MessageWait(); //Can block
	*message = *msg;
	return SystemCallResult::Success;
}

