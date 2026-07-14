#pragma once

#include "kernel/UProcess.h"
#include "kernel/KProcess.h"
#include "kernel/UThread.h"
#include "kernel/KThread.h"
#include "kernel/Api.h"
#include "new.h"

uint32_t UProcess::LastId = 0;

UProcess::UProcess(const CString& name) :
	KProcess(KProcessType::User),
	Id(++LastId),
	InitProcess(),
	InitThread(),
	IsConsole(),
	Name(name),
	Objects(),
	Threads(),
	State(),
	m_peb(),
	m_userArena(),
	m_moduleIndex()
{

}

/**
 * Sets up the PEB in the process's user space.
 */
void UProcess::Initialize(void* const image, const CString& cmd)
{
	//Reserve OS-managed user space (PEB + command line); no writes to user memory here.
	void* const storage = KeVirtualAlloc(*this, KernelReserve);
	Assert(storage);
	m_userArena = {reinterpret_cast<uint8_t*>(storage), KernelReserve};

	m_peb = reinterpret_cast<ProcessEnvironmentBlock*>(m_userArena.Reserve(sizeof(ProcessEnvironmentBlock)));
	Assert(m_peb);
	char* const cmdLine = reinterpret_cast<char*>(m_userArena.Reserve(cmd.Length + 1));
	Assert(cmdLine);

	//Command line bytes (+ null terminator) into the process.
	KeWriteProcessMemory(*this, cmdLine, cmd.c_str(), cmd.Length);
	const char nul = '\0';
	KeWriteProcessMemory(*this, cmdLine + cmd.Length, &nul, 1);

	//Build the PEB in kernel memory, then copy it into the process. CommandLine points at the
	//user-space copy above.
	ProcessEnvironmentBlock peb = {};
	peb.ImageBase = image;
	peb.CommandLine = CString(cmdLine, cmd.Length);
	peb.Handle = (HProcess)this;
	peb.ProcessId = Id;
	peb.ModuleIndex = 0;
	peb.Debug = false;
	KeWriteProcessMemory(*this, m_peb, &peb, sizeof(peb));
}

UThread* UProcess::CreateThread(KThread& backing, const size_t stackSize, const UThreadStart userStart, void* const arg)
{
	//Where user thread starts executing from. Main process thread calls inits process and executes main.
	//Other user threads call init thread and then jump to this entry
	void* threadStart = userStart == nullptr ? InitProcess : InitThread;
	
	//Reserve the TEB in user space (filled below). UThread keeps a reference to
	//this user-space TEB address; it isn't dereferenced until the thread runs.
	ThreadEnvironmentBlock* teb = reinterpret_cast<ThreadEnvironmentBlock*>(m_userArena.Reserve(sizeof(ThreadEnvironmentBlock)));
	Assert(teb);

	//Allocate thread on the kernel heap
	UThread* thread = KeAlloc<UThread>(AllocType::User, *this, backing, *teb);
	Assert(thread);
	Assert(Threads.Add(thread));

	//Allocate thread stack
	void* m_stack = KeVirtualAlloc(*this, UThreadStackSize);
	Assert(m_stack);
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
	char buffer[128] = {};
	snprintf(buffer, sizeof(buffer), "%s[%d]", Name.c_str(), thread->Id);
	thread->Name = KeCopy(CString(buffer), AllocType::User);

	return thread;
}

UObject* UProcess::CreateObject(const UObjectType type)
{
	UObject* created = KeAlloc<UObject>(AllocType::User, type);
	Assert(Objects.Add(created));
	return created;
}

UObject* UProcess::CreateObject(const UObjectType type, const handle_t handle)
{
	UObject* created = KeAlloc<UObject>(AllocType::User, type, handle);
	Assert(Objects.Add(created));
	return created;
}

UObject* UProcess::GetObject()
{
	return Objects.First();
}

UObject* UProcess::GetObject(const handle_t handle)
{
	handle_t ctx = handle;
	for (UObject*& obj : Objects)
	{
		if (obj->Handle == handle)
			return obj;
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
			//TODO(tsharpe): Free
			break;

		case UObjectType::Pipe:
			//TODO(tsharpe): Free
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

		default:
			break;
	}

	Objects.Remove(object);
	return true;
}

const KModule* UProcess::AddModule(const CString& name, void* image)
{
	//Write the module entry into the process's PEB via the physical window (no direct user access).
	//m_moduleIndex is tracked kernel-side so we never read PEB->ModuleIndex from user space.
	Assert(m_moduleIndex < MaxLoadedModules);
	Module entry = {};
	entry.ImageBase = image;
	strcpy(entry.Name, name.c_str());
	KeWriteProcessMemory(*this, &m_peb->LoadedModules[m_moduleIndex], &entry, sizeof(entry));

	m_moduleIndex++;
	KeWriteProcessMemory(*this, &m_peb->ModuleIndex, &m_moduleIndex, sizeof(m_peb->ModuleIndex));

	KModule* created = KeAlloc<KModule>(AllocType::User, name, image);
	Assert(this->Modules.Add(created));
	return created;
}

void UProcess::Display() const
{
	KProcess::Display();

	Printf("UProcess\n");
	Printf("     Id: %x\n", Id);
	Printf("   Name: %s\n", Name.c_str());
	Printf("  State: %d\n", State);

	if (false /*Tables.IsActive()*/)
	{
		Printf("   PEB:\n");
		Printf("    -    Id: %d\n", m_peb->ProcessId);
		Printf("    -  Base: 0x%016x\n", m_peb->ImageBase);
		Printf("    -   Cmd: %s\n", m_peb->CommandLine.c_str());
	}

	//objects
	Printf("UObjects: %d\n", Objects.Count());
	for (const UObject* obj : Objects)
		obj->Display();
}
