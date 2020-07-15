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
	Print("Process: %s exited with code 0x%x\n", process.GetName().c_str(), exitCode);
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

//Doesn't block
uint32_t Kernel::PeekMessage(Message* message)
{
	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);

	Message* msg = user->DequeueMessage();
	if (!msg)
		return SystemCallResult::Failed;
	
	*message = *msg;
	return SystemCallResult::Success;
}

uint64_t Kernel::SetScreenBuffer(void* buffer)
{
	if (!buffer)
		return SystemCallResult::Failed;

	const size_t size = (size_t)m_display->GetHeight() * m_display->GetWidth();
	memcpy((void*)m_display->Buffer(), buffer, size);
	return SystemCallResult::Success;
}

Handle Kernel::CreateFile(const char* name, GenericAccess access)
{
	if (!name)
		return nullptr;

	return this->CreateFile(std::string(name), access);
}

uint32_t Kernel::ReadFile(Handle* handle, void* buffer, size_t bufferSize, size_t* bytesRead)
{
	if (!handle || !buffer || !bufferSize)
		return SystemCallResult::Failed;

	FileHandle* file = (FileHandle*)handle;
	return this->ReadFile(file, buffer, bufferSize, bytesRead);
}

uint32_t Kernel::SetFilePosition(Handle* handle, size_t position)
{
	if (!handle)
		return SystemCallResult::Failed;

	return this->SetFilePosition((FileHandle*)handle, position);
}

void* Kernel::VirtualAlloc(void* address, size_t size, MemoryAllocationType allocationType, MemoryProtection protect)
{
	if (!size)
		return nullptr;

	UserProcess& process = m_scheduler->GetCurrentProcess();
	
	return m_virtualMemory->Allocate((uintptr_t)address, SIZE_TO_PAGES(size), protect, process.GetAddressSpace());
}

