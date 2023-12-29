#include "Kernel/Kernel.h"
#include "Assert.h"
#include "UserRingBuffer.h"
#include "Kernel/Objects/KEvent.h"
#include "Kernel/Objects/UEvent.h"
#include "Kernel/Objects/KFile.h"
#include "Kernel/Objects/UFile.h"
#include "Kernel/Objects/UProcess.h"
#include "Kernel/Objects/UserPipe.h"
#include "Kernel/Objects/UProcess.h"
#include "Kernel/Objects/USemaphore.h"
#include "Kernel/Objects/UDebug.h"
#include "Kernel/Objects/UPipe.h"
#include "user/MetalOS.Types.h"

//TODO: Method to check handles

uint64_t Kernel::Syscall(X64_SYSCALL_FRAME* frame)
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
		return (uint64_t)AllocWindow((HWindow*)frame->Arg0, (Graphics::Rectangle*)frame->Arg1);

	case SystemCall::PaintWindow:
		return (uint64_t)PaintWindow((HWindow*)frame->Arg0, (ReadOnlyBuffer*)frame->Arg1);

	case SystemCall::MoveWindow:
		return (uint64_t)MoveWindow((HWindow)frame->Arg0, (Graphics::Rectangle*)frame->Arg1);

	case SystemCall::GetWindowRect:
		return (uint64_t)GetWindowRect((HWindow)frame->Arg0, (Graphics::Rectangle*)frame->Arg1);

	case SystemCall::GetMessage:
		return (uint64_t)GetMessage((Message*)frame->Arg0);

	case SystemCall::PeekMessage:
		return (uint64_t)PeekMessage((Message*)frame->Arg0);

	case SystemCall::GetScreenRect:
		return (uint64_t)GetScreenRect((Graphics::Rectangle*)frame->Arg0);

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
		return (uintptr_t)VirtualAlloc((void*)frame->Arg0, (size_t)frame->Arg1);

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
	
	info->PageSize = PageSize;
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
	const nano100_t tsc = m_hyperV.ReadTsc();
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
	UserThread& user = m_scheduler->GetCurrentUserThread();
	return &user;
}

HThread Kernel::CreateThread(const size_t stackSize, const ThreadStart startAddress, void* const arg)
{
	if (!IsValidUserPointer(startAddress))
		return nullptr;

	UserProcess& process = m_scheduler->GetCurrentProcess();

	std::shared_ptr<KThread> thread = CreateThread(process, stackSize, startAddress, arg, process.InitThread);
	return (HThread)thread->UserThread;
}

SystemCallResult Kernel::CreateProcess(const char* processName, const CreateProcessArgs* args, CreateProcessResult* result)
{
	if (!IsValidUserPointer(processName))
		return SystemCallResult::InvalidPointer;

	if (args && !IsValidUserPointer(args))
		return SystemCallResult::InvalidPointer;

	std::shared_ptr<UserProcess> newProcess = std::shared_ptr<UserProcess>(KeCreateProcess(std::string(processName)));
	Assert(newProcess);
	m_processes->push_back(newProcess);

	//HACK: While we still switch contexts in CreateProcess, we need to switch it back to calling process
	UserProcess& process = m_scheduler->GetCurrentProcess();
	ArchSetPagingRoot(process.GetCR3());

	if (result && IsValidUserPointer(result))
	{
		const std::shared_ptr<UProcess> uProc = std::make_shared<UProcess>(newProcess);
		result->Process = (HProcess)process.AddObject(uProc);
	}

	kernel.Printf("Console: %d\n", newProcess->IsConsole);
	if (newProcess->IsConsole)
	{
		Assert(args);

		//We only support console apps from a terminal, so we expect full handles
		Assert(args->StdInput);
		Assert(args->StdOutput);
		Assert(args->StdError);

		//Create standard-in handle
		{
			const std::shared_ptr<UObject> input = process.GetObject((Handle)args->StdInput);
			if (!input)
				return SystemCallResult::InvalidHandle;
			if (!input->IsReadable())
				return SystemCallResult::InvalidObject;

			UPipe* uInput = (UPipe*)input.get();
			uInput->Pipe->Readers++;

			const std::shared_ptr<UPipe> readPipe = std::make_shared<UPipe>(uInput->Pipe, PipeOp::Read);
			newProcess->SetStandardHandle(StandardHandle::Input, readPipe);
		}

		{
			const std::shared_ptr<UObject> output = process.GetObject((Handle)args->StdOutput);
			if (!output)
				return SystemCallResult::InvalidHandle;
			if (!output->IsWriteable())
				return SystemCallResult::InvalidObject;

			UPipe* uOutput = (UPipe*)output.get();
			uOutput->Pipe->Writers++;

			const std::shared_ptr<UPipe> writePipe = std::make_shared<UPipe>(uOutput->Pipe, PipeOp::Write);
			newProcess->SetStandardHandle(StandardHandle::Output, writePipe);
		}

		{
			const std::shared_ptr<UObject> error = process.GetObject((Handle)args->StdError);
			if (!error)
				return SystemCallResult::InvalidHandle;
			if (!error->IsWriteable())
				return SystemCallResult::InvalidObject;

			UPipe* uError = (UPipe*)error.get();
			uError->Pipe->Writers++;

			const std::shared_ptr<UPipe> errorPipe = std::make_shared<UPipe>(uError->Pipe, PipeOp::Write);
			newProcess->SetStandardHandle(StandardHandle::Error, errorPipe);
		}
	}
	else
	{
		//Create Debug object to pipe stdout to kernel stream for non-console processes
		const std::shared_ptr<UDebug> uDebug = std::make_shared<UDebug>();
		newProcess->SetStandardHandle(StandardHandle::Output, uDebug);
	}

	kernel.Printf("Returning\n");
	return SystemCallResult::Success;
}

//TODO(tsharpe): Return system call result?
//TODO(tsharpe): This is written poorly
//TODO(tsharpe): Handle is just the address??
uint32_t Kernel::GetThreadId(const Handle handle)
{
	if (!handle)
		return UINT32_MAX;
	
	kernel.Printf("GetThreadId 0x%016x\n", handle);

	const UserThread& user = m_scheduler->GetCurrentUserThread();

	const UserThread* thread = (UserThread*)handle;
	if (thread->Process.Id != user.Process.Id)
		return UINT32_MAX;

	return thread->Id;
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
	const UserThread& user = m_scheduler->GetCurrentUserThread();
	UserProcess& process = user.Process;
	kernel.Printf("Process: %s exited with code 0x%x\n", process.Name.c_str(), exitCode);

	//Free all windows
	m_windows.FreeWindows(process);

	m_scheduler->KillCurrentProcess();
	return SystemCallResult::Success;
}

SystemCallResult Kernel::ExitThread(const uint32_t exitCode)
{
	const UserThread& user = m_scheduler->GetCurrentUserThread();
	kernel.Printf("Thread of %s exited with code 0x%x\n", user.Process.Name.c_str(), exitCode);

	//TODO(tsharpe): Determine if any other process has a handle to this thread? Is this possible?

	//Remove window from thread
	if (m_windows.ThreadHasWindow(user))
		m_windows.FreeWindow(user);

	m_scheduler->KillCurrentThread();
	return SystemCallResult::Success;
}

SystemCallResult Kernel::AllocWindow(HWindow* handle, const Graphics::Rectangle* bounds)
{
	if (!IsValidUserPointer(handle) || !IsValidUserPointer(bounds))
		return SystemCallResult::InvalidPointer;

	UserThread& user = m_scheduler->GetCurrentUserThread();

	if (m_windows.ThreadHasWindow(user))
		return SystemCallResult::Failed;

	*handle = m_windows.AllocWindow(user, *bounds);
	return SystemCallResult::Success;
}

SystemCallResult Kernel::PaintWindow(HWindow handle, const ReadOnlyBuffer* buffer)
{
	if (!IsValidUserPointer(buffer))
		return SystemCallResult::InvalidPointer;

	if (!m_windows.PaintWindow(handle, *buffer))
		return SystemCallResult::InvalidHandle;

	return SystemCallResult::Success;
}

SystemCallResult Kernel::MoveWindow(HWindow handle, const Graphics::Rectangle* rectangle)
{
	if (!IsValidUserPointer(rectangle))
		return SystemCallResult::InvalidPointer;

	if (!m_windows.MoveWindow(handle, *rectangle))
		return SystemCallResult::InvalidHandle;

	return SystemCallResult::Success;
}

SystemCallResult Kernel::GetWindowRect(HWindow handle, Graphics::Rectangle* bounds)
{
	if (!IsValidUserPointer(bounds))
		return SystemCallResult::InvalidPointer;

	if (!m_windows.GetWindowRect(handle, *bounds))
		return SystemCallResult::InvalidHandle;

	return SystemCallResult::Success;
}

//Blocks until message and returns it
SystemCallResult Kernel::GetMessage(Message* message)
{
	if (!IsValidUserPointer(message))
		return SystemCallResult::InvalidPointer;

	UserThread& user = m_scheduler->GetCurrentUserThread();

	//Create signal, block if low
	KPredicate signal(&UserThread::HasMessage, &user);
	m_scheduler->ObjectWait(signal);

	//Return message
	Assert(user.HasMessage());
	user.DequeueMessage(*message);

	return SystemCallResult::Success;
}

//Doesn't block
SystemCallResult Kernel::PeekMessage(Message* message)
{
	if (!IsValidUserPointer(message))
		return SystemCallResult::InvalidPointer;
	
	UserThread& user = m_scheduler->GetCurrentUserThread();
	if (!user.HasMessage())
		return SystemCallResult::Failed;

	Assert(user.DequeueMessage(*message));
	return SystemCallResult::Success;
}

SystemCallResult Kernel::GetScreenRect(Graphics::Rectangle* rectangle)
{
	if (!IsValidUserPointer(rectangle))
		return SystemCallResult::InvalidPointer;

	rectangle->X = 0;
	rectangle->Y = 0;
	rectangle->Width = m_display.GetWidth();
	rectangle->Height = m_display.GetHeight();

	return SystemCallResult::Success;
}

//TODO(tsharpe): Should this return SystemCallResult?
HFile Kernel::CreateFile(const char* name, const GenericAccess access)
{
	if (!IsValidUserPointer(name))
		return nullptr;

	kernel.Printf("CreateFile: %s Access: %d\n", name, access);

	const UserThread& user = m_scheduler->GetCurrentUserThread();

	const std::shared_ptr<UFile> uFile = std::make_shared<UFile>();
	bool result = this->KeCreateFile(uFile->File, std::string(name), access);
	if (!result)
		return nullptr;

	Handle handle = user.Process.AddObject(uFile);
	return (HFile)handle;
}

SystemCallResult Kernel::CreatePipe(HFile* readHandle, HFile* writeHandle)
{
	if (!IsValidUserPointer(readHandle) || !IsValidUserPointer(writeHandle))
		return SystemCallResult::InvalidPointer;

	const UserThread& user = m_scheduler->GetCurrentUserThread();
	UserProcess& process = user.Process;

	std::shared_ptr<UserPipe> pipe = std::make_shared<UserPipe>();
	pipe->Readers = 1;
	pipe->Writers = 1;

	const std::shared_ptr<UPipe> readPipe = std::make_shared<UPipe>(pipe, PipeOp::Read);
	*readHandle = (HFile)process.AddObject(readPipe);

	const std::shared_ptr<UPipe> writePipe = std::make_shared<UPipe>(pipe, PipeOp::Write);
	*writeHandle = (HFile)process.AddObject(writePipe);

	return SystemCallResult::Success;
}

SystemCallResult Kernel::ReadFile(const HFile handle, void* buffer, const size_t bufferSize, size_t* bytesRead)
{
	if (!IsValidUserPointer(buffer) || (bytesRead && !IsValidUserPointer(bytesRead)))
		return SystemCallResult::InvalidPointer;

	if (!bufferSize)
		return SystemCallResult::Failed;

	const UserThread& user = m_scheduler->GetCurrentUserThread();

	const std::shared_ptr<UObject> uObject = user.Process.GetObject((Handle)handle);
	if (!uObject)
		return SystemCallResult::InvalidHandle;

	if (!uObject->IsReadable())
		return SystemCallResult::InvalidObject;

	switch (uObject->Type)
	{
	case UObjectType::File:
	{
		UFile* uFile = (UFile*)uObject.get();

		if (!CanRead(uFile->File.Access))
			return SystemCallResult::Failed;

		Assert(this->KeReadFile(uFile->File, buffer, bufferSize, bytesRead));
		return SystemCallResult::Success;
	}
		break;
		
	case UObjectType::Pipe:
	{
		UPipe* uPipe = (UPipe*)uObject.get();
		UserPipe& pipe = *uPipe->Pipe.get();

		//Set up signal
		PipeEvent event(pipe, PipeOp::Read, bufferSize);
		KPredicate pipeWait(&UserPipe::EventSignal, &event);

		//Block if not signaled
		WaitStatus status = m_scheduler->ObjectWait(pipeWait);
		Assert(status == WaitStatus::Signaled);

		//Read
		size_t read;
		Assert(pipe.Read(buffer, bufferSize, read));
		if (bytesRead)
			*bytesRead = read;

		return pipe.IsBroken() ? SystemCallResult::BrokenPipe : SystemCallResult::Success;
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

	const UserThread& user = m_scheduler->GetCurrentUserThread();

	const std::shared_ptr<UObject> uObject = user.Process.GetObject((Handle)handle);
	if (!uObject)
		return SystemCallResult::InvalidHandle;

	if (!uObject->IsWriteable())
		return SystemCallResult::InvalidObject;

	switch (uObject->Type)
	{
	case UObjectType::File:
	{
		Assert(false);
	}
	break;

	case UObjectType::Pipe:
	{
		const UPipe* uPipe = (UPipe*)uObject.get();
		UserPipe& pipe = *uPipe->Pipe.get();

		//Set up signal
		PipeEvent event(pipe, PipeOp::Write, bufferSize);
		KPredicate pipeWait(&UserPipe::EventSignal, &event);

		//Block if not signaled
		WaitStatus status = m_scheduler->ObjectWait(pipeWait);
		Assert(status == WaitStatus::Signaled);

		//Write
		Assert(pipe.Write(buffer, bufferSize));
		return SystemCallResult::Success;
	}
	break;

	case UObjectType::Debug:
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
	const UserThread& user = m_scheduler->GetCurrentUserThread();

	const std::shared_ptr<UObject> uObject = user.Process.GetObject((Handle)handle);
	if (!uObject)
		return SystemCallResult::InvalidHandle;

	if (uObject->Type != UObjectType::File)
		return SystemCallResult::InvalidObject;

	UFile* uFile = (UFile*)uObject.get();
	
	size_t pos;
	switch (moveType)
	{
	case FilePointerMove::Begin:
		pos = position;
		break;

	case FilePointerMove::Current:
		pos = uFile->File.Position + position;
		break;

	case FilePointerMove::End:
		pos = uFile->File.Length + position;
		break;

	default:
		return SystemCallResult::Failed;
	}

	bool result = this->KeSetFilePosition(uFile->File, pos);
	if (!result)
		return SystemCallResult::Failed;

	if (IsValidUserPointer(newPosition))
		*newPosition = pos;

	return SystemCallResult::Success;
}

SystemCallResult Kernel::CloseFile(const HFile handle)
{
	const UserThread& user = m_scheduler->GetCurrentUserThread();
	UserProcess& process = user.Process;

	const std::shared_ptr<UObject> uObject = process.GetObject((Handle)handle);
	if (!uObject)
		return SystemCallResult::InvalidHandle;

	if (uObject->Type != UObjectType::File)
		return SystemCallResult::InvalidObject;

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
	if (!IsValidUserPointer(status))
		return SystemCallResult::InvalidPointer;
	
	const UserThread& user = m_scheduler->GetCurrentUserThread();
	UserProcess& process = user.Process;

	const std::shared_ptr<UObject> uObject = process.GetObject((Handle)handle);
	if (!uObject)
		return SystemCallResult::InvalidHandle;

	//TODO(tsharpe): Might be a better way of getting the KSignalObject from all of these?
	switch (uObject->Type)
	{
		case UObjectType::Process:
		{
			const UProcess* process = (UProcess*)uObject.get();
			*status = this->KeWait(*process->Proc.get(), milliseconds);
		}
		break;

		case UObjectType::Semaphore:
		{
			USemaphore* sem = (USemaphore*)uObject.get();
			*status = this->KeWait(sem->Sem, milliseconds);
		}
		break;

		case UObjectType::Event:
		{
			UEvent* event = (UEvent*)uObject.get();
			*status = this->KeWait(event->Event, milliseconds);
		}
		break;

		default:
			return SystemCallResult::InvalidObject;
	}

	return SystemCallResult::Success;
}

SystemCallResult Kernel::GetPipeInfo(const HFile handle, PipeInfo* info)
{
	if (!IsValidUserPointer(info))
		return SystemCallResult::InvalidPointer;

	const UserThread& user = m_scheduler->GetCurrentUserThread();

	const std::shared_ptr<UObject> uObject = user.Process.GetObject((Handle)handle);
	if (!uObject)
		return SystemCallResult::InvalidHandle;

	if (uObject->Type != UObjectType::Pipe)
		return SystemCallResult::InvalidObject;

	const UPipe* uPipe = (UPipe*)uObject.get();
	const UserPipe& pipe = *uPipe->Pipe.get();
	
	info->BytesAvailable = pipe.Count();
	info->IsBroken = pipe.IsBroken();
	return SystemCallResult::Success;
}

//TODO(tsharpe): Use deleters on objects?
SystemCallResult Kernel::CloseHandle(const Handle handle)
{
	const UserThread& user = m_scheduler->GetCurrentUserThread();
	UserProcess& process = user.Process;

	const std::shared_ptr<UObject> uObject = process.GetObject((Handle)handle);
	if (!uObject)
		return SystemCallResult::InvalidHandle;

	if (uObject->Type == UObjectType::Pipe)
	{
		const UPipe* uPipe = (UPipe*)uObject.get();
		UserPipe& pipe = *uPipe->Pipe.get();

		if (uPipe->Op == PipeOp::Read)
		{
			pipe.Readers--;
		}
		else
		{
			Assert(uPipe->Op == PipeOp::Write);
			pipe.Writers--;
		}
	}

	Assert(process.CloseObject(handle));

	return SystemCallResult::Success;
}
SystemCallResult Kernel::CreateEvent(HEvent* handle, const bool manual, const bool initial)
{
	if (!IsValidUserPointer(handle))
		return SystemCallResult::InvalidPointer;
	
	const UserThread& user = m_scheduler->GetCurrentUserThread();
	UserProcess& process = user.Process;

	const std::shared_ptr<UEvent> uEvent = std::make_shared<UEvent>(manual, initial);

	*handle = (HEvent)process.AddObject(uEvent);
	return SystemCallResult::Success;
}

SystemCallResult Kernel::SetEvent(const HEvent handle)
{
	const UserThread& user = m_scheduler->GetCurrentUserThread();
	UserProcess& process = user.Process;

	const std::shared_ptr<UObject> uObject = process.GetObject((Handle)handle);
	if (!uObject)
		return SystemCallResult::InvalidHandle;

	if (uObject->Type != UObjectType::Event)
		return SystemCallResult::InvalidObject;

	UEvent* uEvent = (UEvent*)uObject.get();
	uEvent->Event.Set();

	return SystemCallResult::Success;
}

SystemCallResult Kernel::ResetEvent(const HEvent handle)
{
	const UserThread& user = m_scheduler->GetCurrentUserThread();
	UserProcess& process = user.Process;

	const std::shared_ptr<UObject> uObject = process.GetObject((Handle)handle);
	if (!uObject)
		return SystemCallResult::InvalidHandle;

	if (uObject->Type != UObjectType::Event)
		return SystemCallResult::InvalidObject;

	UEvent* uEvent = (UEvent*)uObject.get();
	uEvent->Event.Reset();

	return SystemCallResult::Success;
}

void* Kernel::VirtualAlloc(const void* address, const size_t size)
{
	if (!size)
		return nullptr;

	UserProcess& process = m_scheduler->GetCurrentProcess();
	
	return m_virtualMemory->Allocate(address, SizeToPages(size), process.GetAddressSpace());
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
	
	const size_t indexCount = SizeToPages(indexSize);
	const size_t ringCount = SizeToPages(ringSize);
	const size_t count = indexCount + 2 * ringCount;
	paddr_t address = AllocatePhysical(count);

	std::vector<paddr_t> addresses;

	//Add index pages
	for (size_t i = 0; i < indexCount; i++)
	{
		addresses.push_back(address + (i << PageShift));
	}

	//Push physical addresses twice (for wraparound)
	for (size_t i = 0; i < ringCount; i++)
	{
		addresses.push_back(address + ((i + indexCount) << PageShift));
	}
	for (size_t i = 0; i < ringCount; i++)
	{
		addresses.push_back(address + ((i + indexCount) << PageShift));
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

	return kernel.VirtualMap(address, buffer->GetAddresses());
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

	return kernel.VirtualMap(address, buffer->GetAddresses());
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
