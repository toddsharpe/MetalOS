#include "Kernel.h"
#include "Assert.h"
#include "UserRingBuffer.h"

//TODO: Method to check handles

uint64_t Kernel::Syscall(SystemCallFrame* frame)
{
	//Printf("Syscall - Call 0x%x, RSP: 0x%016x\n", frame->SystemCall, frame->RSP);
	switch (frame->SystemCall)
	{
	case SystemCall::GetSystemInfo:
		return (uint64_t)GetSystemInfo((SystemInfo*)frame->Arg0);

	case SystemCall::GetTickCount:
		return GetTickCount();

	case SystemCall::GetCurrentThread:
		return (uint64_t)GetCurrentThread();

	case SystemCall::CreateThread:
		return (uint64_t)CreateThread((size_t)frame->Arg0, (ThreadStart)frame->Arg1, (void*)frame->Arg2);

	case SystemCall::CreateProcess:
		return (uintptr_t)CreateProcess((char*)frame->Arg0);

	case SystemCall::GetThreadId:
		return (uint64_t)GetThreadId((Handle)frame->Arg0);

	case SystemCall::Sleep:
		Sleep((uint32_t)frame->Arg0);
		return 0;

	case SystemCall::SwitchToThread:
		SwitchToThread();
		return 0;

	case SystemCall::SuspendThread:
		return (uint64_t)SuspendThread((Handle*)frame->Arg0, (size_t*)frame->Arg1);

	case SystemCall::ResumeThread:
		return (uint64_t)ResumeThread((Handle*)frame->Arg0, (size_t*)frame->Arg1);

	case SystemCall::ExitProcess:
		return (uint64_t)ExitProcess((uint32_t)frame->Arg0);

	case SystemCall::ExitThread:
		return (uint64_t)ExitThread((uint32_t)frame->Arg0);

	case SystemCall::AllocWindow:
		return (uint64_t)AllocWindow((HWindow*)frame->Arg0, (Rectangle*)frame->Arg1);

	case SystemCall::PaintWindow:
		return (uint64_t)PaintWindow((HWindow*)frame->Arg0, (ReadOnlyBuffer*)frame->Arg1);

	case SystemCall::MoveWindow:
		return (uint64_t)MoveWindow((HWindow)frame->Arg0, (Rectangle*)frame->Arg1);

	case SystemCall::GetWindowRect:
		return (uint64_t)GetWindowRect((HWindow)frame->Arg0, (Rectangle*)frame->Arg1);

	case SystemCall::GetMessage:
		return (uint64_t)GetMessage((Message*)frame->Arg0);

	case SystemCall::PeekMessage:
		return (uint64_t)PeekMessage((Message*)frame->Arg0);

	case SystemCall::GetScreenRect:
		return (uint64_t)GetScreenRect((Rectangle*)frame->Arg0);

	case SystemCall::CreateFile:
		return (uintptr_t)CreateFile((char*)frame->Arg0, (GenericAccess)frame->Arg1);

	case SystemCall::ReadFile:
		return (uint64_t)ReadFile((Handle*)frame->Arg0, (void*)frame->Arg1, (size_t)frame->Arg2, (size_t*)frame->Arg3);

	case SystemCall::WriteFile:
		return (uint64_t)WriteFile((Handle*)frame->Arg0, (void*)frame->Arg1, (size_t)frame->Arg2, (size_t*)frame->Arg3);

	case SystemCall::SetFilePointer:
		return (uint64_t)SetFilePointer((Handle*)frame->Arg0, (__int64)frame->Arg1, (FilePointerMove)frame->Arg2, (size_t*)frame->Arg3);

	case SystemCall::CloseFile:
		return (uint64_t)CloseFile((Handle*)frame->Arg0);

	case SystemCall::MoveFile:
		return (uint64_t)MoveFile((char*)frame->Arg0, (char*)frame->Arg1);

	case SystemCall::DeleteFile:
		return (uint64_t)DeleteFile((char*)frame->Arg0);

	case SystemCall::CreateDirectory:
		return (uint64_t)CreateDirectory((char*)frame->Arg0);

	case SystemCall::VirtualAlloc:
		return (uintptr_t)VirtualAlloc((void*)frame->Arg0, (size_t)frame->Arg1, (MemoryAllocationType)frame->Arg2, (MemoryProtection)frame->Arg3);

	case SystemCall::CreateRingBuffer:
		return (uintptr_t)CreateRingBuffer((char*)frame->Arg0, (size_t)frame->Arg1, (size_t)frame->Arg2);

	case SystemCall::CreateSharedMemory:
		return (uintptr_t)CreateSharedMemory((char*)frame->Arg0, (size_t)frame->Arg1);

	case SystemCall::MapObject:
		return (uintptr_t)MapObject((void*)frame->Arg0, (Handle)frame->Arg1);

	case SystemCall::MapSharedObject:
		return (uintptr_t)MapSharedObject((void*)frame->Arg0, (char*)frame->Arg1);

	case SystemCall::DebugPrint:
		return (uint64_t)DebugPrint((char*)frame->Arg0);

	default:
		kernel.Printf("SystemCall: 0x%x\n", frame->SystemCall);
		Assert(false);
		return -1;
	}
}

SystemCallResult Kernel::GetSystemInfo(SystemInfo* info)
{
	if (!IsValidUserPointer(info))
		return SystemCallResult::Failed;
	
	info->PageSize = PAGE_SIZE;
	info->Architecture = SystemArchitecture::x64;
	info->NumberOfProcessors = 1;
	info->AllocationGranularity = VirtualAddressSpace::AllocationGranularity;
	info->MinimumApplicationAddress = VirtualAddressSpace::AllocationGranularity;
	info->MaximumApplicationAddress = (UserStop - 1) - VirtualAddressSpace::AllocationGranularity;
	return SystemCallResult::Success;
}

//Milliseconds
size_t Kernel::GetTickCount()
{
	const nano100_t tsc = m_hyperV->ReadTsc();
	return (tsc * 100) / 1000000;
}

SystemCallResult Kernel::GetSystemTime(SystemTime* time)
{
	if (!IsValidUserPointer(time))
		return SystemCallResult::InvalidPointer;

	KeGetSystemTime(*time);

	return SystemCallResult::Success;
}

HThread Kernel::GetCurrentThread()
{
	KThread* current = m_scheduler->GetCurrentThread();
	UserThread* userThread = current->GetUserThread();
	Assert(userThread);
	return userThread;
}

HThread Kernel::CreateThread(size_t stackSize, ThreadStart startAddress, void* arg)
{
	if (!startAddress || !arg)
		return nullptr;

	UserProcess& process = m_scheduler->GetCurrentProcess();
	return CreateThread(process, stackSize, startAddress, arg, process.InitThread);
}

SystemCallResult Kernel::CreateProcess(const char* processName)
{
	if (!processName || !IsValidUserPointer(processName))
		return SystemCallResult::InvalidPointer;

	bool result = KeCreateProcess(std::string(processName));

	//HACK: While we still switch contexts in CreateProcess, we need to switch it back
	UserProcess& process = m_scheduler->GetCurrentProcess();
	ArchSetPagingRoot(process.GetCR3());
	kernel.Printf("NewCr3: 0x%016x\n", process.GetCR3());

	kernel.Printf("Returning\n");
	return SystemCallResult::Success;
}

uint32_t Kernel::GetThreadId(const Handle handle)
{
	if (!handle)
		return UINT32_MAX;
	
	kernel.Printf("GetThreadId 0x%016x\n", handle);
	UserProcess& process = m_scheduler->GetCurrentProcess();
	UserThread* thread = (UserThread*)handle;

	UserProcess& proc = thread->GetProcess();
	if (proc.GetId() != process.GetId())
		return UINT32_MAX;

	return thread->GetId();
}

void Kernel::Sleep(const uint32_t milliseconds)
{
	if (!milliseconds)
		return;

	KeSleepThread((nano_t)milliseconds * 1000 * 1000);
}

void Kernel::SwitchToThread()
{
	m_scheduler->Schedule();
}

SystemCallResult Kernel::SuspendThread(Handle thread, size_t* prevCount)
{
	if (!thread || !IsValidUserPointer(prevCount))
		return SystemCallResult::Failed;

	KThread* kThread = (KThread*)thread;
	*prevCount = m_scheduler->Suspend(*kThread);
	return SystemCallResult::Success;
}

SystemCallResult Kernel::ResumeThread(Handle thread, size_t* prevCount)
{
	if (!thread || !IsValidUserPointer(prevCount))
		return SystemCallResult::Failed;

	KThread* kThread = (KThread*)thread;
	*prevCount = m_scheduler->Resume(*kThread);
	return SystemCallResult::Success;
}

SystemCallResult Kernel::ExitProcess(const uint32_t exitCode)
{
	UserProcess& process = m_scheduler->GetCurrentProcess();
	process.Delete = true;
	kernel.Printf("Process: %s exited with code 0x%x\n", process.GetName().c_str(), exitCode);
	m_scheduler->KillThread();
	return SystemCallResult::Success;
}

SystemCallResult Kernel::ExitThread(const uint32_t exitCode)
{
	UserProcess& process = m_scheduler->GetCurrentProcess();
	kernel.Printf("Thread of %s exited with code 0x%x\n", process.GetName().c_str(), exitCode);
	m_scheduler->KillThread();
	return SystemCallResult::Success;
}

SystemCallResult Kernel::AllocWindow(HWindow* handle, const Rectangle* bounds)
{
	if (!IsValidUserPointer(handle) || !IsValidUserPointer(bounds))
		return SystemCallResult::InvalidPointer;

	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);

	if (m_windows->ThreadHasWindow(user->GetId()))
		return SystemCallResult::Failed;

	*handle = m_windows->AllocWindow(user, *bounds);
	return SystemCallResult::Success;
}

SystemCallResult Kernel::PaintWindow(HWindow handle, const ReadOnlyBuffer* buffer)
{
	if (!IsValidUserPointer(buffer))
		return SystemCallResult::InvalidPointer;

	if (!m_windows->PaintWindow(handle, *buffer))
		return SystemCallResult::Failed;

	return SystemCallResult::Success;
}

SystemCallResult Kernel::MoveWindow(HWindow handle, const Rectangle* rectangle)
{
	if (!IsValidUserPointer(rectangle))
		return SystemCallResult::InvalidPointer;

	if (!m_windows->MoveWindow(handle, *rectangle))
		return SystemCallResult::Failed;

	return SystemCallResult::Success;
}

SystemCallResult Kernel::GetWindowRect(HWindow handle, Rectangle* bounds)
{
	if (!IsValidUserPointer(bounds))
		return SystemCallResult::InvalidPointer;

	if (!m_windows->GetWindowRect(handle, *bounds))
		return SystemCallResult::Failed;

	return SystemCallResult::Success;
}

//Blocks until message and returns it
SystemCallResult Kernel::GetMessage(Message* message)
{
	if (!IsValidUserPointer(message))
		return SystemCallResult::Failed;

	UserThread* user = m_scheduler->GetCurrentUserThread();
	Message* msg = m_scheduler->MessageWait(); //Can block
	*message = *msg;
	return SystemCallResult::Success;
}

//Doesn't block
SystemCallResult Kernel::PeekMessage(Message* message)
{
	if (!IsValidUserPointer(message))
		return SystemCallResult::Failed;
	
	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);

	Message* msg = user->DequeueMessage();
	if (!msg)
		return SystemCallResult::Failed;

	*message = *msg;
	return SystemCallResult::Success;
}

SystemCallResult Kernel::GetScreenRect(Rectangle* rectangle)
{
	if (!IsValidUserPointer(rectangle))
		return SystemCallResult::Failed;

	rectangle->X = 0;
	rectangle->Y = 0;
	rectangle->Width = m_display->GetWidth();
	rectangle->Height = m_display->GetHeight();

	return SystemCallResult::Success;
}

Handle Kernel::CreateFile(const char* name, const GenericAccess access)
{
	if (!IsValidUserPointer(name))
		return nullptr;

	kernel.Printf("CreateFile: %s Access: %d\n", name, access);

	return this->KeCreateFile(std::string(name), access);
}

SystemCallResult Kernel::ReadFile(const Handle handle, void* buffer, const size_t bufferSize, size_t* bytesRead)
{
	if (!handle)
		return SystemCallResult::Failed;

	if (!IsValidUserPointer(buffer) || !bufferSize)
		return SystemCallResult::Failed;

	FileHandle* file = (FileHandle*)handle;

	bool result = this->KeReadFile(*file, buffer, bufferSize, bytesRead);
	return result ? SystemCallResult::Success : SystemCallResult::Failed;
}

SystemCallResult Kernel::WriteFile(const Handle handle, const void* lpBuffer, size_t bufferSize, size_t* bytesWritten)
{
	//Not implemented
	Assert(false);
	SystemCallResult::Failed;
}

SystemCallResult Kernel::SetFilePointer(const Handle handle, const __int64 position, const FilePointerMove moveType, size_t* newPosition)
{
	if (!handle)
		return SystemCallResult::Failed;

	FileHandle* file = (FileHandle*)handle;
	
	size_t pos;
	switch (moveType)
	{
	case FilePointerMove::Begin:
		pos = position;
		break;

	case FilePointerMove::Current:
		pos = file->Position + position;
		break;

	case FilePointerMove::End:
		pos = file->Length + position;
		break;

	default:
		return SystemCallResult::Failed;
	}

	bool result = this->KeSetFilePosition(*file, pos);
	if (!result)
		return SystemCallResult::Failed;

	if (newPosition != nullptr)
		*newPosition = pos;

	return SystemCallResult::Success;
}

SystemCallResult Kernel::CloseFile(const Handle handle)
{
	if (!handle)
		return SystemCallResult::Failed;

	KeCloseFile((FileHandle*)handle);
	return SystemCallResult::Success;
}

SystemCallResult Kernel::MoveFile(const char* existingFileName, const char* newFileName)
{
	Assert(false);
	SystemCallResult::Failed;
}

SystemCallResult Kernel::DeleteFile(const char* fileName)
{
	Assert(false);
	SystemCallResult::Failed;
}

SystemCallResult Kernel::CreateDirectory(const char* path)
{
	Assert(false);
	SystemCallResult::Failed;
}

void* Kernel::VirtualAlloc(const void* address, const size_t size, const MemoryAllocationType allocationType, const MemoryProtection protect)
{
	if (!size)
		return nullptr;

	UserProcess& process = m_scheduler->GetCurrentProcess();
	
	return m_virtualMemory->Allocate((uintptr_t)address, SIZE_TO_PAGES(size), protect, process.GetAddressSpace());
}

HRingBuffer Kernel::CreateRingBuffer(const char* name, const size_t indexSize, const size_t ringSize)
{
	if (indexSize == 0 || ringSize == 0)
		return nullptr;

	if (name != nullptr)
	{
		if (!IsValidUserPointer(name))
			return nullptr;

		if (strlen(name) == 0)
			return nullptr;

		const auto& it = m_objectsRingBuffers->find(std::string(name));
		if (it != m_objectsRingBuffers->end())
			return nullptr;
	}
	
	const size_t indexCount = SIZE_TO_PAGES(indexSize);
	const size_t ringCount = SIZE_TO_PAGES(ringSize);
	const size_t count = indexCount + 2 * ringCount;
	paddr_t address = AllocatePhysical(count);

	std::vector<paddr_t> addresses;

	//Add index pages
	for (size_t i = 0; i < indexCount; i++)
	{
		addresses.push_back(address + (i << PAGE_SHIFT));
	}

	//Push physical addresses twice (for wraparound)
	for (size_t i = 0; i < ringCount; i++)
	{
		addresses.push_back(address + ((i + indexCount) << PAGE_SHIFT));
	}
	for (size_t i = 0; i < ringCount; i++)
	{
		addresses.push_back(address + ((i + indexCount) << PAGE_SHIFT));
	}
	Assert(addresses.size() == count);

	//Add to process
	UserProcess& process = m_scheduler->GetCurrentProcess();
	UserRingBuffer* ringBuffer = new UserRingBuffer("", addresses);
	Assert(ringBuffer);
	process.AddRingBuffer(*ringBuffer);

	//if named, add to system (like FS on linux)
	if ((name != nullptr) && (strlen(name) > 0))
	{
		m_objectsRingBuffers->insert({ std::string(name), ringBuffer });
	}

	return ringBuffer;
}

HSharedMemory Kernel::CreateSharedMemory(const char* name, const size_t size)
{
	return nullptr;
}

void* Kernel::MapObject(const void* address, Handle handle)
{
	UserProcess& process = m_scheduler->GetCurrentProcess();
	UserRingBuffer* buffer = process.GetRingBuffer((HRingBuffer)handle);
	if (!buffer)
		return nullptr;

	return kernel.VirtualMap(address, buffer->GetAddresses(), MemoryProtection::PageReadWrite);
}

void* Kernel::MapSharedObject(const void* address, const char* name)
{
	kernel.Printf("MapSharedObject: 0x%016x 0x%016x\n", address, name);
	if (name == nullptr)
		return nullptr;
	
	if (!IsValidUserPointer(name))
		return nullptr;

	if (strlen(name) == 0)
		return nullptr;

	kernel.Printf("MapSharedObject: %s %d\n", name, m_objectsRingBuffers->size());

	const auto& it = m_objectsRingBuffers->find(std::string(name));
	if (it == m_objectsRingBuffers->end())
		return nullptr;

	UserRingBuffer* buffer = (UserRingBuffer*)it->second;
	Assert(buffer);

	UserProcess& process = m_scheduler->GetCurrentProcess();

	return kernel.VirtualMap(address, buffer->GetAddresses(), MemoryProtection::PageReadWrite);
}

SystemCallResult Kernel::DebugPrint(char* s)
{
	this->m_printer->Write(s);
	return SystemCallResult::Success;
}
