#pragma once

#include "msvc.h"
#include <string>
#include <cstdint>
#include <set>
#include "VirtualAddressSpace.h"
#include <PageTables.h>
#include "BootHeap.h"
#include "MetalOS.h"
#include "UserRingBuffer.h"
#include "UObject.h"
#include "KSignalObject.h"

class KThread;
class UserProcess : public KSignalObject
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

	const uint32_t GetId() const
	{
		return m_id;
	}

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

	void SetStandardHandle(const StandardHandle handle, UObject* object);
	Handle AddObject(UObject* object);
	UObject* GetObject(Handle handle);
	bool CloseObject(Handle handle);

	bool Delete;
	bool IsConsole;

	friend class Scheduler;
private:
	void* HeapAlloc(size_t size);
	typedef size_t handle_t;
	static constexpr handle_t StartingHandle = 0x10;

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
	std::map<HRingBuffer, UserRingBuffer*> m_ringBuffers;

	handle_t m_lastHandle;
	std::map<handle_t, UObject*> m_objects;
};

