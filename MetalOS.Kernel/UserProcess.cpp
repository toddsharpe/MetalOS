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
	m_threads(),
	InitProcess(),
	InitThread(),
	Delete()
{
	//Create new page tables, using current top level kernel mappings
	m_pageTables = new PageTables();
	m_pageTables->LoadKernelMappings(new PageTables(__readcr3()));
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

void UserProcess::AddModule(const char* name, void* address)
{
	//Has to be called within context of process for now
	Assert(__readcr3() == m_pageTables->GetCr3());

	//TODO: check name length

	m_peb->LoadedModules[m_peb->ModuleIndex].Address = address;
	strcpy(m_peb->LoadedModules[m_peb->ModuleIndex].Name, name);
	m_peb->ModuleIndex++;
}

void* UserProcess::HeapAlloc(size_t size)
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
		uintptr_t address = (uintptr_t)m_peb->LoadedModules[i].Address;

		if (ip < (uintptr_t)address)
			continue;

		PIMAGE_DOS_HEADER dosHeader = MakePtr(PIMAGE_DOS_HEADER, address, 0);
		Assert(dosHeader->e_magic == IMAGE_DOS_SIGNATURE);

		PIMAGE_NT_HEADERS64 ntHeader = MakePtr(PIMAGE_NT_HEADERS64, address, dosHeader->e_lfanew);
		Assert(ntHeader->Signature == IMAGE_NT_SIGNATURE);

		if ((ip >= address) && (ip < address + ntHeader->OptionalHeader.SizeOfImage))
			return address;
	}

	Print("IP: 0x%016x\n", ip);
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
	Print("UserProcess::Display\n");
	Print("    ID: %d\n", m_id);
	Print("  Name: %s\n", m_name.c_str());
	Print("  Base: 0x%016x\n", m_imageBase);
	Print("    Ts: %d\n", m_threads.size());
	Print("   PEB: 0x%016x\n", m_peb);
}

void UserProcess::DisplayDetails() const
{
	Assert(GetCR3() == __readcr3());

	Print("DisplayPEB\n");
	Print("    ID: %d\n", m_peb->ProcessId);
	Print("  Base: 0x%016x\n", m_peb->BaseAddress);
	Print("  Mods: %d\n", m_peb->ModuleIndex);
	for (size_t i = 0; i < m_peb->ModuleIndex; i++)
		Print("    %s: 0x%016x\n", m_peb->LoadedModules[i].Name, m_peb->LoadedModules[i].Address);
}

