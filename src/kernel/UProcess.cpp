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
	m_state(),
	m_peb(),
	m_userArena()
{

}

void UProcess::Initialize()
{
	//Threads is a LinkedList; it self-initializes. Kept for the process init contract.
}

//This has to occur when this process is active (page tables are in use)
//TODO(tsharpe): Remove this limitation (map into kernel also?)
void UProcess::InContextInit(void* const image, const CString& cmd)
{
	//Allocate space in process for OS to manage
	void* const storage = KeVirtualAlloc(*this, KernelReserve);
	Assert(storage);
	m_userArena = {reinterpret_cast<uint8_t*>(storage), KernelReserve};

	//Allocate PEB
	m_peb = m_userArena.Allocate<ProcessEnvironmentBlock>();
	Assert(m_peb);
	m_peb->ImageBase = image;
	m_peb->CommandLine = m_userArena.Copy(cmd); //Copy string to user address space
	m_peb->Handle = (HProcess)this;
	m_peb->ProcessId = Id;
	m_peb->Debug = false;
}

UThread* UProcess::CreateThread(KThread& backing, const size_t stackSize, const UThreadStart userStart, void* const arg)
{
	//Where user thread starts executing from. Main process thread calls inits process and executes main.
	//Other user threads call init thread and then jump to this entry
	void* threadStart = userStart == nullptr ? InitProcess : InitThread;
	
	//Allocate TEB for thread
	ThreadEnvironmentBlock* teb = m_userArena.Allocate<ThreadEnvironmentBlock>();
	Assert(teb);
	teb->SelfPointer = teb;
	teb->PEB = m_peb;
	teb->ThreadStart = userStart;
	teb->Arg = arg;

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
	teb->Handle = (HThread)thread;
	teb->ThreadId = thread->Id;

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
	Assert((int)handle < (int)UProcessState::Last);

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
	//Assert(Tables.IsActive());
	m_peb->LoadedModules[m_peb->ModuleIndex].ImageBase = image;
	strcpy(m_peb->LoadedModules[m_peb->ModuleIndex].Name, name.c_str());
	m_peb->ModuleIndex++;

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
	Printf("  State: %d\n", m_state);

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
