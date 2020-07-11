#pragma once

#include "msvc.h"
#include <string>
#include <cstdint>
#include "VirtualAddressSpace.h"
#include <PageTables.h>
#include "BootHeap.h"
#include "MetalOS.h"

class KThread;
class UserProcess
{
public:
	static uint32_t LastId;

	UserProcess(const std::string& name);
	void Init(void* address);

	uintptr_t GetImageBase() const
	{
		return m_imageBase;
	}
	uintptr_t GetCR3() const;
	VirtualAddressSpace& GetAddressSpace();
	ProcessEnvironmentBlock* GetPEB() const;

	void* HeapAlloc(size_t size);

	void* InitProcess;
	void* InitThread;

	size_t ThreadCount()
	{
		return m_threads.size();
	}

	void AddThread(KThread& thread)
	{
		m_threads.push_back(&thread);
	}

	void AddModule(const char* name, void* address);

	bool Delete;

	friend class Scheduler;
private:
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

