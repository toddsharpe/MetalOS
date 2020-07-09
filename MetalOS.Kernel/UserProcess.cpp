#include "UserProcess.h"
#include <intrin.h>
#include "Main.h"
//#include "Main.h"

uint32_t UserProcess::LastId = 0;

UserProcess::UserProcess(const std::string& name) : 
	m_imageBase(),
	m_id(++LastId),
	m_name(name),
	m_createTime(),
	m_exitTime(),
	m_pageTables(),
	m_addressSpace(*new UserAddressSpace()),
	m_heap(),
	m_peb(),
	m_threads()
{
	//Create new page tables, using current top level kernel mappings
	m_pageTables = new PageTables();
	m_pageTables->LoadKernelMappings(new PageTables(__readcr3()));
}

void UserProcess::AddModule(const char* name, void* address)
{
	//Has to be called within context of process for now
	Assert(__readcr3() == m_pageTables->GetCr3());

	//TODO: check name length

	m_peb->LoadedModules[m_peb->ModuleIndex].Address = address;
	strcpy(m_peb->LoadedModules[m_peb->ModuleIndex].Name, name);
	m_peb->ModuleIndex++;
}

void UserProcess::Init(void* address)
{
	//Has to be called within context of process for now
	Assert(__readcr3() == m_pageTables->GetCr3());

	m_imageBase = (uintptr_t)address;

	void* start = kernel.VirtualAlloc(*this, nullptr, PAGE_SIZE, MemoryAllocationType::CommitReserve, MemoryProtection::PageReadWrite);
	Print("heap: 0x%016x\n", start);
	m_heap = new BootHeap(start, PAGE_SIZE);
	m_peb = (ProcessEnvironmentBlock*)m_heap->Allocate(sizeof(ProcessEnvironmentBlock));
	Assert(m_peb);
	memset(m_peb, 0, sizeof(ProcessEnvironmentBlock));
	m_peb->ProcessId = m_id;
	m_peb->BaseAddress = (uintptr_t)address;
	Print("m_peb: 0x%016x\n", m_peb);
	Print(" addr: 0x%016x\n", m_peb->BaseAddress);
}

void* UserProcess::HeapAlloc(size_t size)
{
	return m_heap->Allocate(size);
}

uintptr_t UserProcess::GetCR3() const
{
	return m_pageTables->GetCr3();
}

VirtualAddressSpace& UserProcess::GetAddressSpace()
{
	return m_addressSpace;
}

ProcessEnvironmentBlock* UserProcess::GetPEB() const
{
	return m_peb;
}

