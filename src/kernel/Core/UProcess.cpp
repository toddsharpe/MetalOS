#include "kernel/Core/UProcess.h"
#include "kernel/Core/KProcess.h"
#include "kernel/Core/UThread.h"
#include "kernel/Core/KThread.h"
#include "kernel/Vmem.h"
#include "kernel/Heap.h"
#include "kernel/Shm.h"
#include "kernel/Thread.h"
#include "kernel/Process.h"
#include "kernel/Pipe.h"
#include "kernel/PebModules.h"
#include "new.h"
#include "core_crt/stdio.h"

static uint32_t LastId = 0;

UProcess::UProcess(const cstr_t name) :
	KProcess(KProcessType::User),
	Id(++LastId),
	InitProcess(),
	InitThread(),
	IsConsole(),
	Name(),
	Objects(),
	Threads(),
	State(),
	Refs(1),
	Reaped(),
	m_peb(),
	m_userArena(),
	m_moduleIndex()
{
	StrCopy(Name, name);
}

/**
 * Sets up the PEB in the process's user space.
 */
void UProcess::Initialize(void* const image, const cstr_t cmd)
{
	//Reserve OS-managed user space (PEB + command line); no writes to user memory here.
	void* const storage = KeVirtualAlloc(*this, KernelReserve);
	Assert(storage);
	m_userArena = {reinterpret_cast<uint8_t*>(storage), KernelReserve};

	m_peb = reinterpret_cast<ProcessEnvironmentBlock*>(m_userArena.Reserve(sizeof(ProcessEnvironmentBlock)));
	Assert(m_peb);
	char* const cmdLine = reinterpret_cast<char*>(m_userArena.Reserve(cmd.Size + 1));
	Assert(cmdLine);

	//Command line bytes (+ null terminator) into the process.
	KeWriteProcessMemory(*this, cmdLine, cmd.Data, cmd.Size);
	const char nul = '\0';
	KeWriteProcessMemory(*this, cmdLine + cmd.Size, &nul, 1);

	//Build the PEB in kernel memory, then copy it into the process. CommandLine points at the
	//user-space copy above.
	ProcessEnvironmentBlock peb = {};
	peb.ImageBase = image;
	peb.CommandLine = { cmdLine, cmd.Size };
	peb.Handle = (HProcess)this;
	peb.ProcessId = Id;
	peb.ModuleIndex = 0;
	peb.Debug = false;
	KeWriteProcessMemory(*this, m_peb, &peb, sizeof(peb));
}

UThread* UProcess::CreateThread(const size_t stackSize, const UThreadStart userStart, void* const arg)
{
	UNUSED(stackSize);
	//Where user thread starts executing from. Main process thread calls inits process and executes main.
	//Other user threads call init thread and then jump to this entry
	void* threadStart = userStart == nullptr ? InitProcess : InitThread;

	//Reserve the TEB in user space (filled below). UThread keeps a reference to
	//this user-space TEB address; it isn't dereferenced until the thread runs.
	ThreadEnvironmentBlock* teb = reinterpret_cast<ThreadEnvironmentBlock*>(m_userArena.Reserve(sizeof(ThreadEnvironmentBlock)));
	if (!teb)
		return nullptr; //TEB space is the per-process thread cap

	//Allocate thread stack
	void* m_stack = KeVirtualAlloc(*this, UThreadStackSize);
	if (!m_stack)
		return nullptr;

	//Allocate thread on the kernel heap
	UThread* thread = KeAlloc<UThread>(AllocType::User, *this, *teb);
	Assert(thread);
	Threads.InsertTail(*thread);
	thread->StackBase = m_stack;

	void* const stackPointer = MakePointer<void*>(m_stack, UThreadStackSize - ArchStackReserve());
	Assert(stackPointer);

	//Initialize
	thread->Initialize(stackPointer, threadStart);

	//Build the TEB in kernel memory and copy it into the process.
	ThreadEnvironmentBlock tebData = {};
	tebData.SelfPointer = teb;
	tebData.PEB = m_peb;
	tebData.ThreadStart = userStart;
	tebData.Arg = arg;
	tebData.Handle = (HThread)thread;
	tebData.ThreadId = thread->Id;
	KeWriteProcessMemory(*this, teb, &tebData, sizeof(tebData));

	//Set name
	snprintf(thread->Name, sizeof(thread->Name), "%s[%d]", Name, thread->Id);

	return thread;
}

UObject* UProcess::CreateObject(const UObjectType type)
{
	UObject* created = KeAlloc<UObject>(AllocType::User, type);
	Objects.InsertTail(*created);
	return created;
}

UObject* UProcess::CreateObject(const UObjectType type, const handle_t handle)
{
	UObject* created = KeAlloc<UObject>(AllocType::User, type, handle);
	Objects.InsertTail(*created);
	return created;
}

UObject* UProcess::GetObject()
{
	return Objects.IsEmpty() ? nullptr : &Objects.First();
}

UObject* UProcess::GetObject(const handle_t handle)
{
	for (UObject& obj : Objects)
	{
		if (obj.Handle == handle)
			return &obj;
	}

	return nullptr;
}

bool UProcess::CloseObject(const handle_t handle)
{
	UObject* object = GetObject(handle);
	if (!object)
		return false;

	switch (object->Type)
	{
		case UObjectType::Event:
			//Drop the handle's reference, which frees the event once no thread is waiting on it either
			KeEventUnref(*object->Event);
			break;

		case UObjectType::Pipe:
			//Drop this end's reference on the pipe, which frees it once no end or blocked thread is left
			switch (object->PipeOp)
			{
				case KPipeOp::Read:
					KePipeUnref(*object->Pipe, object->Pipe->Readers);
					break;

				case KPipeOp::Write:
					KePipeUnref(*object->Pipe, object->Pipe->Writers);
					break;

				default:
					Assert(false);
			}
			break;

		case UObjectType::SharedMemory:
			if (object->Shm)
			{
				//Unmap our view (if we mapped it), then drop this handle's reference.
				if (object->ShmAddress)
					KeVirtualFree(*this, object->ShmAddress);
				KeShmUnref(*object->Shm);
			}
			break;

		case UObjectType::Thread:
			KeReleaseUThread(*object->Thread);
			break;

		case UObjectType::Process:
			KeReleaseUProcess(*object->Process);
			break;

		default:
			break;
	}

	Objects.Remove(*object);
	KeFree(object, AllocType::User);
	return true;
}

void UProcess::AddModule(const cstr_t name, void* const image)
{
	//Write the module entry into the process's PEB via the physical window (no direct user access).
	//m_moduleIndex is tracked kernel-side so we never read PEB->ModuleIndex from user space.
	Assert(m_moduleIndex < MaxLoadedModules);
	Module entry = {};
	entry.ImageBase = image;
	StrCopy(entry.Name, name);
	KeWriteProcessMemory(*this, &m_peb->LoadedModules[m_moduleIndex], &entry, sizeof(entry));

	m_moduleIndex++;
	KeWriteProcessMemory(*this, &m_peb->ModuleIndex, &m_moduleIndex, sizeof(m_peb->ModuleIndex));
}

bool UProcess::GetModule(const uintptr_t address, KModule& module) const
{
	return GetPebModule(m_peb, Space, address, module);
}

void UProcess::Display() const
{
	KProcess::Display();

	Printf("UProcess\n");
	Printf("     Id: %x\n", Id);
	Printf("   Name: %s\n", Name);
	Printf("  State: %d\n", State);

	if (false /*Tables.IsActive()*/)
	{
		Printf("   PEB:\n");
		Printf("    -    Id: %d\n", m_peb->ProcessId);
		Printf("    -  Base: 0x%016x\n", m_peb->ImageBase);
		Printf("    -   Cmd: %.*s\n", (int)m_peb->CommandLine.Size, m_peb->CommandLine.Data);
	}

	//objects
	Printf("UObjects: %d\n", Objects.Count());
	for (const UObject& obj : Objects)
		obj.Display();
}
