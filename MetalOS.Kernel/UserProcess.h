#pragma once

#include "msvc.h"
#include <string>
#include <cstdint>
#include <set>
#include "VirtualAddressSpace.h"
#include <PageTables.h>
#include "BootHeap.h"
#include "MetalOS.h"
#include "UserWindow.h"

class KThread;
class UserProcess
{
public:
	static uint32_t LastId;

	UserProcess(const std::string& name);
	void Init(void* address);

	uintptr_t GetModuleBase(uintptr_t ip) const;

	uintptr_t GetCR3() const;
	VirtualAddressSpace& GetAddressSpace();

	ThreadEnvironmentBlock* AllocTEB();

	void* InitProcess;
	void* InitThread;

	void AddThread(KThread& thread)
	{
		m_threads.push_back(&thread);
	}

	void AddModule(const char* name, void* address);

	void Display() const;
	void DisplayDetails() const;

	const std::string& GetName() const
	{
		return m_name;
	}

	bool Delete;

	friend class Scheduler;
private:
	void* HeapAlloc(size_t size);

	uintptr_t m_imageBase;
	uint32_t m_id;
	std::string m_name;
	time_t m_createTime;
	time_t m_exitTime;
	PageTables* m_pageTables;
	VirtualAddressSpace& m_addressSpace;
	BootHeap* m_heap;
	ProcessEnvironmentBlock* m_peb;
	std::list<KThread*> m_threads;
};

