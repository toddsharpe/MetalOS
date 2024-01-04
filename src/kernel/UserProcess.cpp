#include "UserProcess.h"

#include "Kernel/Kernel.h"
#include "Assert.h"
#include <intrin.h>
#include "user/MetalOS.Types.h"
#include "PortableExecutable.h"
#include <string>


uint32_t UserProcess::LastId = 0;

UserProcess::UserProcess(const std::string& name, const bool isConsole) : 
	KSignalObject(),
	InitProcess(),
	InitThread(),
	Name(name),
	Id(++LastId),
	IsConsole(isConsole),

	m_imageBase(),
	m_createTime(),
	m_exitTime(),
	m_pageTables(),
	m_addressSpace(),
	m_heap(),
	m_peb(),
	m_threads(),
	m_ringBuffers(),
	m_lastHandle(StartingHandle),
	m_objects(),
	m_state(ProcessState::Running)
{
	//Create new page tables, using current top level kernel mappings
	m_pageTables = new PageTables();
	m_pageTables->LoadKernelMappings(new PageTables(__readcr3()));
}

void UserProcess::Init(void* address, const std::vector<std::string>& args)
{
	//Has to be called within context of process for now
	Assert(__readcr3() == m_pageTables->GetCr3());

	m_imageBase = (uintptr_t)address;

	void* start = kernel.VirtualAlloc(*this, nullptr, PageSize);
	kernel.Printf("heap: 0x%016x\n", start);
	m_heap = new BootHeap(start, PageSize);
	m_peb = (ProcessEnvironmentBlock*)m_heap->Allocate(sizeof(ProcessEnvironmentBlock));
	Assert(m_peb);
	memset(m_peb, 0, sizeof(ProcessEnvironmentBlock));
	m_peb->ProcessId = Id;
	m_peb->ImageBase = address;

	//Build args in PEB
	m_peb->argc = args.size();
	m_peb->argv = (char**)m_heap->Allocate(sizeof(char*) * args.size());
	for (size_t i = 0; i < args.size(); i++)
	{
		m_peb->argv[i] = (char*)m_heap->Allocate(args[i].size() + 1);
		strcpy(m_peb->argv[i], args[i].c_str());
		m_peb->argv[i][args[i].size()] = '\0';
	}

	kernel.Printf("m_peb: 0x%016x\n", m_peb);
	kernel.Printf(" addr: 0x%016x\n", m_peb->ImageBase);
	kernel.Printf(" argc: %d\n", m_peb->argc);
	for (size_t i = 0; i < m_peb->argc; i++)
	{
		kernel.Printf("   argv[%d]: %s\n", i, m_peb->argv[i]);
	}
}

void UserProcess::AddModule(const char* name, void* address)
{
	//Has to be called within context of process for now
	AssertEqual(__readcr3(), m_pageTables->GetCr3());

	//TODO: check name length

	m_peb->LoadedModules[m_peb->ModuleIndex].ImageBase = address;
	strcpy(m_peb->LoadedModules[m_peb->ModuleIndex].Name, name);
	m_peb->ModuleIndex++;
}

Module* UserProcess::GetModule(const uintptr_t ip) const
{
	AssertEqual(__readcr3(), m_pageTables->GetCr3());
	for (size_t i = 0; i < m_peb->ModuleIndex; i++)
	{
		const void* address = m_peb->LoadedModules[i].ImageBase;
		if (PortableExecutable::Contains(address, ip))
			return &m_peb->LoadedModules[i];
	}
	return nullptr;
}

void* UserProcess::HeapAlloc(const size_t size)
{
	return m_heap->Allocate(size);
}

ThreadEnvironmentBlock* UserProcess::AllocTEB()
{
	ThreadEnvironmentBlock* teb = (ThreadEnvironmentBlock*)HeapAlloc(sizeof(ThreadEnvironmentBlock));
	memset(teb, 0, sizeof(ThreadEnvironmentBlock));
	teb->PEB = m_peb;
	return teb;
}

uintptr_t UserProcess::GetModuleBase(uintptr_t ip) const
{
	for (size_t i = 0; i < m_peb->ModuleIndex; i++)
	{
		void* address = m_peb->LoadedModules[i].ImageBase;

		if (ip < (uintptr_t)address)
			continue;

		PIMAGE_DOS_HEADER dosHeader = static_cast<PIMAGE_DOS_HEADER>(address);
		AssertEqual(dosHeader->e_magic, IMAGE_DOS_SIGNATURE);

		PIMAGE_NT_HEADERS64 ntHeader = MakePointer<PIMAGE_NT_HEADERS64>(address, dosHeader->e_lfanew);
		AssertEqual(ntHeader->Signature, IMAGE_NT_SIGNATURE);

		if ((ip >= (uintptr_t)address) && (ip < (uintptr_t)address + ntHeader->OptionalHeader.SizeOfImage))
			return (uintptr_t)address;
	}

	kernel.Printf("IP: 0x%016x\n", ip);
	Assert(false);
	return 0;
}

uintptr_t UserProcess::GetCR3() const
{
	return m_pageTables->GetCr3();
}

VirtualAddressSpace& UserProcess::GetAddressSpace()
{
	return m_addressSpace;
}

void UserProcess::Display() const
{
	kernel.Printf("UserProcess::Display\n");
	kernel.Printf("     ID: %d\n", Id);
	kernel.Printf("   Name: %s\n", Name.c_str());
	kernel.Printf("   Base: 0x%016x\n", m_imageBase);
	kernel.Printf("Threads: %d\n", m_threads.size());
	kernel.Printf("    PEB: 0x%016x\n", m_peb);
}

void UserProcess::DisplayDetails() const
{
	Assert(GetCR3() == __readcr3());

	kernel.Printf("DisplayPEB\n");
	kernel.Printf("     ID: %d\n", m_peb->ProcessId);
	kernel.Printf("   Base: 0x%016x\n", m_peb->ImageBase);
	kernel.Printf("   Mods: %d\n", m_peb->ModuleIndex);
	for (size_t i = 0; i < m_peb->ModuleIndex; i++)
		kernel.Printf("    %s: 0x%016x\n", m_peb->LoadedModules[i].Name, m_peb->LoadedModules[i].ImageBase);
}

void UserProcess::SetStandardHandle(const StandardHandle handle, const std::shared_ptr<UObject>& object)
{
	Assert(m_objects.find((handle_t)handle) == m_objects.end());
	Assert(object);

	if (handle == StandardHandle::Input)
		Assert(object->IsReadable());

	if (handle == StandardHandle::Error || handle == StandardHandle::Output)
		Assert(object->IsWriteable());

	m_objects.insert({ (handle_t)handle, object });
}

Handle UserProcess::AddObject(const std::shared_ptr<UObject>& object)
{
	m_lastHandle++;
	Assert(object.get());

	kernel.Printf("ObjHandle: %d, Name: %s, Type: %d\n", m_lastHandle, Name.c_str(), object.get()->Type);

	m_objects.insert({ m_lastHandle , object });
	return (Handle)m_lastHandle;
}

std::shared_ptr<UObject> UserProcess::GetObject(Handle handle)
{
	const auto& it = m_objects.find((handle_t)handle);
	if (it == m_objects.end())
		return std::shared_ptr<UObject>();

	return it->second;
}

bool UserProcess::CloseObject(Handle handle)
{
	std::shared_ptr<UObject> object = GetObject(handle);
	if (!object)
		return false;

	m_objects.erase((handle_t)handle);
	return true;
}

bool UserProcess::IsSignalled() const
{
	return m_state == ProcessState::Terminated;
}
