#pragma once

#include "kernel/UProcess.h"
#include "kernel/Objects/KSignalObject.h"
#include "kernel/KProcess.h"
#include "kernel/UThread.h"
#include "kernel/KThread.h"
#include "kernel/Api.h"
#include "new.h"

uint32_t UProcess::LastId = 0;

UProcess::UProcess(const CString& name) :
	KProcess(KProcessType::User),
	KSignalObject(),
	Id(++LastId),
	InitProcess(),
	InitThread(),
	IsConsole(),
	Tables(),
	Name(name),
	m_threads(),
	m_state(),
	m_objects(),
	m_peb(),
	m_userArena()
{

}

void UProcess::Initialize()
{
	//Initialize underlying process
	KProcess::Initialize();

	//Initialize thread lists
	ListInitializeHead(m_threads);

	//Initialize uobjects list
	ListInitializeHead(m_objects);
}

//This has to occur when this process is active (page tables are in use)
//TODO(tsharpe): Remove this limitation (map into kernel also?)
void UProcess::InContextInit(void* const image, const CString& cmd)
{
	//Allocate space in process for OS to manage
	void* const storage = KeVirtualAlloc(*this, nullptr, KernelReserve);
	m_userArena = {reinterpret_cast<uint8_t*>(storage), KernelReserve};

	//Allocate PEB
	m_peb = m_userArena.Allocate<ProcessEnvironmentBlock>();
	Assert(m_peb);
	m_peb->ImageBase = image;
	m_peb->CommandLine = m_userArena.Copy(cmd); //Copy string to user address space
	m_peb->Handle = (HProcess)this;
	m_peb->ProcessId = Id;
}

UThread* UProcess::CreateThread(KThread& backing, const size_t stackSize, const UThreadStart userStart, void* const arg)
{
	//Where user thread starts executing from. Main process thread calls inits process and executes main.
	//Other user threads call init thread and then jump to this entry
	void* threadStart = userStart == nullptr ? InitProcess : InitThread;
	
	//Allocate TEB for thread
	ThreadEnvironmentBlock* teb = m_userArena.Allocate<ThreadEnvironmentBlock>();
	teb->SelfPointer = teb;
	teb->PEB = m_peb;
	teb->ThreadStart = userStart;
	teb->Arg = arg;

	//Allocate thread
	void* const mem = m_arena.Allocate(sizeof(UThread));
	Assert(mem);
	UThread* thread = new (mem) UThread(*this, backing, *teb);
	ListInsertTail(m_threads, thread->Link);

	//Allocate thread stack
	void* m_stack = KeVirtualAlloc(*this, nullptr, UThreadStackSize);
	void* const stackPointer = MakePointer<void*>(m_stack, UThreadStackSize - ArchStackReserve());

	//Initialize
	thread->Initialize(stackPointer, threadStart);
	teb->Handle = (HThread)thread;
	teb->ThreadId = thread->Id;

	//Set name
	char buffer[128] = {};
	snprintf(buffer, sizeof(buffer), "%s[%d]", Name.c_str(), thread->Id);
	thread->Name = m_arena.Copy(buffer);

	return thread;
}

UObject* UProcess::CreateObject(const UObjectType type)
{
	UObject* created = m_arena.Allocate<UObject>(type);
	Assert(created);
	ListInsertTail(m_objects, created->Link);
	return created;
}

UObject* UProcess::CreateObject(const UObjectType type, const handle_t handle)
{
	Assert((int)handle < (int)UProcessState::Last);

	UObject* created = m_arena.Allocate<UObject>(type, handle);
	Assert(created);
	ListInsertTail(m_objects, created->Link);
	return created;
}

UObject* UProcess::CreateEvent(const bool manual, const bool initial)
{
	UObject* created = CreateObject(UObjectType::Event);
	created->Event = m_arena.Allocate<KEvent>(manual, initial);
	return created;
}

UObject* UProcess::CreatePipe(KPipe& pipe, const KPipeOp op)
{
	UObject* created = CreateObject(UObjectType::Pipe);
	created->Pipe = m_arena.Allocate<UPipe>(pipe, op);
	return created;
}

UObject* UProcess::CreatePipe(KPipe& pipe, const KPipeOp op, const handle_t handle)
{
	Assert((int)handle < (int)UProcessState::Last);
	
	UObject* created = CreateObject(UObjectType::Pipe, handle);
	created->Pipe = m_arena.Allocate<UPipe>(pipe, op);
	return created;
}

UObject* UProcess::GetObject()
{
	if (!m_objects.Count)
		return nullptr;

	UObject* top = ListGet<UObject>(m_objects, 0);
	return top;
}

UObject* UProcess::GetObject(const handle_t handle)
{
	handle_t ctx = handle;
	UObject* const found = ListFirst<UObject, handle_t>(m_objects, [](const ListEntry&, const UObject& item, handle_t& handle)
	{
		return item.Handle == handle;
	}, ctx);
	return found;
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

		default:
			break;
	}

	//TODO(tsharpe): Free from arena
	ListRemoveEntry(m_objects, object->Link);
	return true;
}

KModule* UProcess::AddModule(const CString& name, void* image)
{
	Assert(Tables.IsActive());
	m_peb->LoadedModules[m_peb->ModuleIndex].ImageBase = image;
	strcpy(m_peb->LoadedModules[m_peb->ModuleIndex].Name, name.c_str());
	m_peb->ModuleIndex++;

	return KProcess::AddModule(name, image);
}

void UProcess::Display() const
{
	KProcess::Display();

	Printf("UProcess\n");
	Printf("     Id: %x\n", Id);
	Printf("   Name: %s\n", Name.c_str());
	Printf("  State: %d\n", m_state);

	if (Tables.IsActive())
	{
		Printf("   PEB:\n");
		Printf("    -    Id: %d\n", m_peb->ProcessId);
		Printf("    -  Base: 0x%016x\n", m_peb->ImageBase);
		Printf("    -   Cmd: %s\n", m_peb->CommandLine.c_str());
	}

	//objects
	Printf("UObjects: %d\n", m_objects.Count);
	ListForEach<UObject>(m_objects, [](const ListEntry&, const UObject& item)
	{
		item.Display();
	});
}

bool UProcess::IsSignalled() const
{
	return m_state == UProcessState::Terminated;
}
