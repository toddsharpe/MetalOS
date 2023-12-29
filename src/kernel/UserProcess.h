#pragma once


#include <string>
#include <cstdint>
#include <set>
#include "VirtualAddressSpace.h"
#include "BootHeap.h"
#include "UserRingBuffer.h"
#include "Kernel/Objects/UObject.h"
#include "Kernel/Objects/KSignalObject.h"
#include "PageTables.h"

#include <memory>
#include <map>
#include <list>

enum class ProcessState
{
	Running,
	Terminated
};

class KThread;
class Scheduler;
class UserProcess : public KSignalObject
{
	friend Scheduler;
public:
	UserProcess(const std::string& name, const bool isConsole);
	
	void Init(void* address);

	uintptr_t GetModuleBase(uintptr_t ip) const;

	uintptr_t GetCR3() const;
	VirtualAddressSpace& GetAddressSpace();

	ThreadEnvironmentBlock* AllocTEB();

	void AddThread(KThread& thread)
	{
		m_threads.push_back(&thread);
	}

	void AddModule(const char* name, void* address);

	void Display() const;
	void DisplayDetails() const;

	void AddRingBuffer(UserRingBuffer& buffer)
	{
		m_ringBuffers.insert({ &buffer, &buffer });
	}

	UserRingBuffer* GetRingBuffer(HRingBuffer buffer)
	{
		const auto& it = m_ringBuffers.find(buffer);
		if (it == m_ringBuffers.end())
			return nullptr;

		return it->second;
	}

	virtual bool IsSignalled() const override;

	void SetStandardHandle(const StandardHandle handle, const std::shared_ptr<UObject>& object);
	Handle AddObject(const std::shared_ptr<UObject>& object);
	std::shared_ptr<UObject> GetObject(Handle handle);
	bool CloseObject(Handle handle);

	const std::string Name;
	const uint32_t Id;
	const bool IsConsole;

	void* InitProcess;
	void* InitThread;

private:
	static uint32_t LastId;

	void* HeapAlloc(const size_t size);
	
	typedef size_t handle_t;
	static constexpr handle_t StartingHandle = 0x10;

	uintptr_t m_imageBase;
	time_t m_createTime;
	time_t m_exitTime;
	PageTables* m_pageTables;
	UserAddressSpace m_addressSpace;
	BootHeap* m_heap;
	ProcessEnvironmentBlock* m_peb;
	std::list<KThread*> m_threads;
	std::map<HRingBuffer, UserRingBuffer*> m_ringBuffers;

	handle_t m_lastHandle;
	std::map<handle_t, std::shared_ptr<UObject>> m_objects;

	ProcessState m_state;
};

