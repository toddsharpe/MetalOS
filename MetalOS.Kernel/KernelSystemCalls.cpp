#include "Kernel.h"
#include "Assert.h"
#include "UserRingBuffer.h"
#include "UserPipe.h"

//TODO: Method to check handles

uint64_t Kernel::Syscall(SystemCallFrame* frame)
{
	//Printf("Syscall - Call 0x%x\n", frame->SystemCall);
	switch (frame->SystemCall)
	{
	case SystemCall::GetSystemInfo:
		return (uint64_t)GetSystemInfo((SystemInfo*)frame->Arg0);

	case SystemCall::GetTickCount:
		return GetTickCount();

	case SystemCall::GetSystemTime:
		return (uint64_t)GetSystemTime((SystemTime*)frame->Arg0);

	case SystemCall::GetCurrentThread:
		return (uint64_t)GetCurrentThread();

	case SystemCall::CreateThread:
		return (uint64_t)CreateThread((size_t)frame->Arg0, (ThreadStart)frame->Arg1, (void*)frame->Arg2);

	case SystemCall::CreateProcess:
		return (uintptr_t)CreateProcess((char*)frame->Arg0, (CreateProcessArgs*)frame->Arg1, (CreateProcessResult*)frame->Arg2);

	case SystemCall::GetThreadId:
		return (uint64_t)GetThreadId((Handle)frame->Arg0);

	case SystemCall::Sleep:
		Sleep((uint32_t)frame->Arg0);
		return 0;

	case SystemCall::SwitchToThread:
		SwitchToThread();
		return 0;

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

	case SystemCall::CreatePipe:
		return (uint64_t)CreatePipe((HFile*)frame->Arg0, (HFile*)frame->Arg1);

	case SystemCall::ReadFile:
		return (uint64_t)ReadFile((HFile*)frame->Arg0, (void*)frame->Arg1, (size_t)frame->Arg2, (size_t*)frame->Arg3);

	case SystemCall::WriteFile:
		return (uint64_t)WriteFile((HFile*)frame->Arg0, (void*)frame->Arg1, (size_t)frame->Arg2, (size_t*)frame->Arg3);

	case SystemCall::SetFilePointer:
		return (uint64_t)SetFilePointer((HFile*)frame->Arg0, (size_t)frame->Arg1, (FilePointerMove)frame->Arg2, (size_t*)frame->Arg3);

	case SystemCall::CloseFile:
		return (uint64_t)CloseFile((HFile*)frame->Arg0);

	case SystemCall::MoveFile:
		return (uint64_t)MoveFile((char*)frame->Arg0, (char*)frame->Arg1);

	case SystemCall::DeleteFile:
		return (uint64_t)DeleteFile((char*)frame->Arg0);

	case SystemCall::CreateDirectory:
		return (uint64_t)CreateDirectory((char*)frame->Arg0);

	case SystemCall::WaitForSingleObject:
		return (uint64_t)WaitForSingleObject((Handle)frame->Arg0, (uint32_t)frame->Arg1, (WaitStatus*)frame->Arg2);

	case SystemCall::GetPipeInfo:
		return (uintptr_t)GetPipeInfo((HFile)frame->Arg0, (PipeInfo*)frame->Arg1);

	case SystemCall::CloseHandle:
		return (uintptr_t)CloseHandle((Handle)frame->Arg0);

	case SystemCall::CreateEvent:
		return (uintptr_t)CreateEvent((HEvent*)frame->Arg0, (bool)frame->Arg1, (bool)frame->Arg2);

	case SystemCall::SetEvent:
		return (uintptr_t)SetEvent((HEvent)frame->Arg0);

	case SystemCall::ResetEvent:
		return (uintptr_t)ResetEvent((HEvent)frame->Arg0);

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

	case SystemCall::DebugPrintBytes:
		return (uint64_t)DebugPrintBytes((char*)frame->Arg0, (size_t)frame->Arg1);

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
	if (!IsValidUserPointer(startAddress))
		return nullptr;

	UserProcess& process = m_scheduler->GetCurrentProcess();
	return CreateThread(process, stackSize, startAddress, arg, process.InitThread);
}

SystemCallResult Kernel::CreateProcess(const char* processName, const CreateProcessArgs* args, CreateProcessResult* result)
{
	if (!IsValidUserPointer(processName))
		return SystemCallResult::InvalidPointer;

	if (args && !IsValidUserPointer(args))
		return SystemCallResult::InvalidPointer;

	UserProcess* newProcess = KeCreateProcess(std::string(processName));
	Assert(newProcess);

	//HACK: While we still switch contexts in CreateProcess, we need to switch it back
	UserProcess& process = m_scheduler->GetCurrentProcess();
	ArchSetPagingRoot(process.GetCR3());
	kernel.Printf("NewCr3: 0x%016x\n", process.GetCR3());

	if (result && IsValidUserPointer(result))
	{
		UObject* procObj = new UObject(*newProcess, UserObjectType::Process);
		result->Process = (HProcess)process.AddObject(procObj);
	}

	if (newProcess->IsConsole)
	{
		Assert(args);

		//We only support console apps from a terminal, so we expect full handles
		Assert(args->StdInput);
		Assert(args->StdOutput);
		Assert(args->StdError);

		UObject* input = process.GetObject((Handle)args->StdInput);
		if (!input)
			return SystemCallResult::InvalidHandle;
		if (!input->IsReadable())
			return SystemCallResult::InvalidObject;
		input->GetObject<UserPipe&>().ReaderCount++;

		UObject* output = process.GetObject((Handle)args->StdOutput);
		if (!output)
			return SystemCallResult::InvalidHandle;
		if (!output->IsWriteable())
			return SystemCallResult::InvalidObject;
		output->GetObject<UserPipe&>().WriterCount++;

		UObject* error = process.GetObject((Handle)args->StdError);
		if (!error)
			return SystemCallResult::InvalidHandle;
		if (!error->IsWriteable())
			return SystemCallResult::InvalidObject;
		error->GetObject<UserPipe&>().WriterCount++;

		newProcess->SetStandardHandle(StandardHandle::Input, input);
		newProcess->SetStandardHandle(StandardHandle::Output, output);
		newProcess->SetStandardHandle(StandardHandle::Error, error);
	}
	else
	{
		//Redirect stdout to debug
		UserPipe* pipe = new UserPipe();

		UObject* debugObj = new UObject(*pipe, UserObjectType::Debug);
		HFile output = (HFile)process.AddObject(debugObj);

		newProcess->SetStandardHandle(StandardHandle::Output, debugObj);

	}

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

SystemCallResult Kernel::ExitProcess(const uint32_t exitCode)
{
	UserThread* thread = m_scheduler->GetCurrentUserThread();
	UserProcess& process = thread->GetProcess();
	kernel.Printf("Process: %s exited with code 0x%x\n", process.GetName().c_str(), exitCode);

	//Free all windows
	if (m_windows->ThreadHasWindow(thread))
		m_windows->FreeWindow(thread);


	m_scheduler->KillCurrentProcess();
	return SystemCallResult::Success;
}

SystemCallResult Kernel::ExitThread(const uint32_t exitCode)
{
	UserThread* thread = m_scheduler->GetCurrentUserThread();
	UserProcess& process = thread->GetProcess();
	kernel.Printf("Thread of %s exited with code 0x%x\n", process.GetName().c_str(), exitCode);

	//TODO: Check if anybody has handle to this thread and clear

	//Remove window from thread
	if (m_windows->ThreadHasWindow(thread))
		m_windows->FreeWindow(thread);

	m_scheduler->KillCurrentThread();
	return SystemCallResult::Success;
}

SystemCallResult Kernel::AllocWindow(HWindow* handle, const Rectangle* bounds)
{
	if (!IsValidUserPointer(handle) || !IsValidUserPointer(bounds))
		return SystemCallResult::InvalidPointer;

	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);

	if (m_windows->ThreadHasWindow(user))
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

HFile Kernel::CreateFile(const char* name, const GenericAccess access)
{
	if (!IsValidUserPointer(name))
		return nullptr;

	kernel.Printf("CreateFile: %s Access: %d\n", name, access);

	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);
	UserProcess& process = user->GetProcess();

	KFile* file = this->KeCreateFile(std::string(name), access);
	if (file == nullptr)
		return nullptr;

	UObject* uFile = new UObject(*file, UserObjectType::File);

	Handle handle = process.AddObject(uFile);
	return (HFile)handle;
}

SystemCallResult Kernel::CreatePipe(HFile* readHandle, HFile* writeHandle)
{
	if (!IsValidUserPointer(readHandle) || !IsValidUserPointer(writeHandle))
		return SystemCallResult::InvalidPointer;

	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);
	UserProcess& process = user->GetProcess();

	UserPipe* pipe = new UserPipe();
	pipe->ReaderCount = 1;
	pipe->WriterCount = 1;

	UObject* readObj = new UObject(*pipe, UserObjectType::ReadPipe);
	*readHandle = (HFile)process.AddObject(readObj);

	UObject* writeObj = new UObject(*pipe, UserObjectType::WritePipe);
	*writeHandle = (HFile)process.AddObject(writeObj);

	return SystemCallResult::Success;
}

SystemCallResult Kernel::ReadFile(const HFile handle, void* buffer, const size_t bufferSize, size_t* bytesRead)
{
	if (!IsValidUserPointer(buffer) || (bytesRead && !IsValidUserPointer(bytesRead)))
		return SystemCallResult::InvalidPointer;

	if (!bufferSize)
		return SystemCallResult::Failed;

	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);
	UserProcess& process = user->GetProcess();

	UObject* userObject = process.GetObject((Handle)handle);
	if (!userObject || !userObject->IsReadable())
		return SystemCallResult::Failed;

	switch (userObject->GetType())
	{
	case UserObjectType::File:
	{
		KFile& file = (KFile&)userObject->GetObject();
		if (!CanRead(file.Access))
			return SystemCallResult::Failed;

		Assert(this->KeReadFile(file, buffer, bufferSize, bytesRead));
		return SystemCallResult::Success;
	}
		break;
		
	case UserObjectType::ReadPipe:
	{
		UserPipe& pipe = (UserPipe&)userObject->GetObject();
		if (pipe.IsBroken())
		{
			size_t read = pipe.ReadPartial(buffer, bufferSize);
			kernel.Printf("ReadPartial: %d");
			if (bytesRead)
				*bytesRead = bufferSize;

			return SystemCallResult::BrokenPipe;
		}

		WaitStatus status = m_scheduler->PipeReadWait(pipe, buffer, bufferSize);
		if (status == WaitStatus::BrokenPipe)
		{
			size_t read = pipe.ReadPartial(buffer, bufferSize);
			kernel.Printf("ReadPartial: %d");
			if (bytesRead)
				*bytesRead = bufferSize;

			return SystemCallResult::BrokenPipe;
		}
		
		if (bytesRead)
			*bytesRead = bufferSize;
		return SystemCallResult::Success;
	}
		break;

	default:
		return SystemCallResult::Failed;
	}
}

SystemCallResult Kernel::WriteFile(const HFile handle, const void* buffer, size_t bufferSize, size_t* bytesWritten)
{
	if (!IsValidUserPointer(buffer) || (bytesWritten && !IsValidUserPointer(bytesWritten)))
		return SystemCallResult::InvalidPointer;

	if (!bufferSize)
		return SystemCallResult::Failed;

	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);
	UserProcess& process = user->GetProcess();

	UObject* userObject = process.GetObject((Handle)handle);
	if (!userObject)
		return SystemCallResult::InvalidHandle;

	if (!userObject->IsWriteable())
		return SystemCallResult::InvalidObject;

	kernel.Printf("WriteFile:\n");
	kernel.PrintBytes((char*)buffer, bufferSize);

	switch (userObject->GetType())
	{
	case UserObjectType::File:
	{
		Assert(false);
	}
	break;

	case UserObjectType::WritePipe:
	{
		UserPipe& pipe = (UserPipe&)userObject->GetObject();
		WaitStatus status = m_scheduler->PipeWriteWait(pipe, buffer, bufferSize);
		if (status == WaitStatus::BrokenPipe)
			return SystemCallResult::BrokenPipe;

		if (bytesWritten)
			*bytesWritten = bufferSize;
		return SystemCallResult::Success;
	}
	break;

	case UserObjectType::Debug:
	{
		this->m_printer->Write((char*)buffer);
		return SystemCallResult::Success;
	}
	break;

	default:
		return SystemCallResult::Failed;
	}
}

SystemCallResult Kernel::SetFilePointer(const HFile handle, const size_t position, const FilePointerMove moveType, size_t* newPosition)
{
	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);
	UserProcess& process = user->GetProcess();

	UObject* userObject = process.GetObject((Handle)handle);
	if (!userObject)
		return SystemCallResult::Failed;

	Assert(userObject->GetType() == UserObjectType::File);
	KFile& file = (KFile&)userObject->GetObject();
	
	size_t pos;
	switch (moveType)
	{
	case FilePointerMove::Begin:
		pos = position;
		break;

	case FilePointerMove::Current:
		pos = file.Position + position;
		break;

	case FilePointerMove::End:
		pos = file.Length + position;
		break;

	default:
		return SystemCallResult::Failed;
	}

	bool result = this->KeSetFilePosition(file, pos);
	if (!result)
		return SystemCallResult::Failed;

	if (IsValidUserPointer(newPosition))
		*newPosition = pos;

	return SystemCallResult::Success;
}

SystemCallResult Kernel::CloseFile(const HFile handle)
{
	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);
	UserProcess& process = user->GetProcess();

	UObject* userObject = process.GetObject((Handle)handle);
	if (!userObject)
		return SystemCallResult::Failed;

	Assert(userObject->GetType() == UserObjectType::File);
	KFile& file = (KFile&)userObject->GetObject();
	KeCloseFile(&file);

	process.CloseObject((Handle)handle);
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

SystemCallResult Kernel::WaitForSingleObject(const Handle handle, const uint32_t milliseconds, WaitStatus* status)
{
	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);
	UserProcess& process = user->GetProcess();

	if (!IsValidUserPointer(status))
		return SystemCallResult::InvalidPointer;

	UObject* userObject = process.GetObject((HFile)handle);
	if (!userObject)
		return SystemCallResult::InvalidHandle;

	KObject& kObj = (KObject&)userObject->GetObject();
	if (!kObj.IsSyncObj())
		return SystemCallResult::InvalidObject;

	*status = this->KeWait((KSignalObject&)kObj, milliseconds);
	return SystemCallResult::Success;
}

SystemCallResult Kernel::GetPipeInfo(const HFile handle, PipeInfo* info)
{
	if (!IsValidUserPointer(info))
		return SystemCallResult::InvalidPointer;

	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);
	UserProcess& process = user->GetProcess();

	UObject* userObject = process.GetObject((HFile)handle);
	if (!userObject)
		return SystemCallResult::InvalidHandle;
	if (!userObject->IsPipe())
		return SystemCallResult::InvalidObject;

	UserPipe& pipe = (UserPipe&)userObject->GetObject();
	info->BytesAvailable = pipe.Count();
	info->IsBroken = pipe.IsBroken();
	return SystemCallResult::Success;
}

SystemCallResult Kernel::CloseHandle(const Handle handle)
{
	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);
	UserProcess& process = user->GetProcess();

	UObject* userObject = process.GetObject((HFile)handle);
	if (!userObject)
		return SystemCallResult::InvalidHandle;

	KObject& obj = userObject->GetObject();

	if (userObject->IsPipe())
	{
		UserPipe& pipe = (UserPipe&)obj;
		if (userObject->GetType() == UserObjectType::ReadPipe)
		{
			pipe.ReaderCount--;
			if (pipe.ReaderCount == 0)
				m_scheduler->PipeSignal(pipe, true);
		}
		else if (userObject->GetType() == UserObjectType::WritePipe)
		{
			pipe.WriterCount--;
			if (pipe.WriterCount == 0)
				m_scheduler->PipeSignal(pipe, false);
		}
		else
			Assert(false);
	}

	Assert(process.CloseObject(handle));

	//todo: signal?

	if (obj.IsClosed())
		obj.Dispose();

	return SystemCallResult::Success;
}
SystemCallResult Kernel::CreateEvent(HEvent* event, const bool manual, const bool initial)
{
	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);
	UserProcess& process = user->GetProcess();

	if (!IsValidUserPointer(event))
		return SystemCallResult::InvalidPointer;

	KEvent* kEvent = new KEvent(manual, initial);
	UObject* obj = new UObject(*kEvent, UserObjectType::Event);

	*event = (HEvent)process.AddObject(obj);
	return SystemCallResult::Success;
}

SystemCallResult Kernel::SetEvent(const HEvent event)
{
	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);
	UserProcess& process = user->GetProcess();

	UObject* userObject = process.GetObject((HFile)event);
	if (!userObject)
		return SystemCallResult::InvalidHandle;

	if (!userObject->IsEvent())
		return SystemCallResult::InvalidObject;

	KEvent& obj = userObject->GetObject<KEvent&>();
	this->KeSignal(obj);

	return SystemCallResult::Success;
}

SystemCallResult Kernel::ResetEvent(const HEvent event)
{
	UserThread* user = m_scheduler->GetCurrentUserThread();
	Assert(user);
	UserProcess& process = user->GetProcess();

	UObject* userObject = process.GetObject((HFile)event);
	if (!userObject)
		return SystemCallResult::InvalidHandle;

	if (!userObject->IsEvent())
		return SystemCallResult::InvalidObject;

	KEvent& obj = userObject->GetObject<KEvent&>();
	obj.Reset();

	return SystemCallResult::Success;
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

SystemCallResult Kernel::DebugPrint(const char* s)
{
	this->m_printer->Write(s);
	return SystemCallResult::Success;
}

SystemCallResult Kernel::DebugPrintBytes(const char* buffer, const size_t length)
{
	this->m_printer->PrintBytes(buffer, length);
	return SystemCallResult::Success;
}
