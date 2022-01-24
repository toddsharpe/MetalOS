#include "Kernel.h"
#include "Assert.h"
#include <intrin.h>

#include "UserProcess.h"

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
	m_ringBuffers(),
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
	kernel.Printf("heap: 0x%016x\n", start);
	m_heap = new BootHeap(start, PAGE_SIZE);
	m_peb = (ProcessEnvironmentBlock*)m_heap->Allocate(sizeof(ProcessEnvironmentBlock));
	Assert(m_peb);
	memset(m_peb, 0, sizeof(ProcessEnvironmentBlock));
	m_peb->ProcessId = m_id;
	m_peb->BaseAddress = (uintptr_t)address;
	kernel.Printf("m_peb: 0x%016x\n", m_peb);
	kernel.Printf(" addr: 0x%016x\n", m_peb->BaseAddress);
}

void UserProcess::AddModule(const char* name, void* address)
{
	//Has to be called within context of process for now
	AssertEqual(__readcr3(), m_pageTables->GetCr3());

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
		void* address = m_peb->LoadedModules[i].Address;

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
	kernel.Printf("     ID: %d\n", m_id);
	kernel.Printf("   Name: %s\n", m_name.c_str());
	kernel.Printf("   Base: 0x%016x\n", m_imageBase);
	kernel.Printf("Threads: %d\n", m_threads.size());
	kernel.Printf("    PEB: 0x%016x\n", m_peb);
}

void UserProcess::DisplayDetails() const
{
	Assert(GetCR3() == __readcr3());

	kernel.Printf("DisplayPEB\n");
	kernel.Printf("     ID: %d\n", m_peb->ProcessId);
	kernel.Printf("   Base: 0x%016x\n", m_peb->BaseAddress);
	kernel.Printf("   Mods: %d\n", m_peb->ModuleIndex);
	for (size_t i = 0; i < m_peb->ModuleIndex; i++)
		kernel.Printf("    %s: 0x%016x\n", m_peb->LoadedModules[i].Name, m_peb->LoadedModules[i].Address);
}

