#pragma once

#include <efi.h>
#include <LoaderParams.h>
#include <MetalOS.h>
#include <WindowsPE.h>
#include "MetalOS.Kernel.h"
#include "Display.h"
#include "LoadingScreen.h"
#include <PageTablesPool.h>
#include "MemoryMap.h"
#include "ConfigTables.h"

class Kernel
{
public:
	Kernel();
	::NO_COPY_OR_ASSIGN(Kernel);
	void Initialize(const PLOADER_PARAMS params);

	void HandleInterrupt(size_t vector, PINTERRUPT_FRAME pFrame);
	void Bugcheck(const char* file, const char* line, const char* assert);

	void Print(const char* format, ...);
	void Print(const char* format, va_list args);

	//Spinlocks
	PSPIN_LOCK GetSpinLock(Handle id);
	Handle CreateSpinlock();
	cpu_flags_t AcquireSpinlock(Handle id);
	void ReleaseSpinlock(Handle id, cpu_flags_t flags);

	//Semaphores
	PSEMAPHORE GetSemaphore(Handle id);
	Handle CreateSemaphore(uint32_t initial, uint32_t maximum, const char* name);
	bool WaitSemaphore(Handle id, size_t count, size_t timeout);//semaphore->P()
	void SignalSemaphore(Handle id, size_t count);//semaphore->V()

	//Get tables
	void* GetAcpiRoot();

	PageTables* GetPageTables()
	{
		return m_pageTables;
	}

	uint64_t GetAcpiTimer();

private:
	PIMAGE_SECTION_HEADER GetKernelSection(const std::string& name);

private:
	//Save from LoaderParams
	uintptr_t m_physicalAddress;
	size_t m_imageSize;
	EFI_RUNTIME_SERVICES m_runtime;

	PageTablesPool* m_pPagePool;
	MemoryMap* m_pMemoryMap;
	ConfigTables* m_pConfigTables;
	PageTables* m_pageTables;

	Display* m_pDisplay;
	LoadingScreen* m_pLoading;

	size_t m_lastProcessId;
	std::list<KERNEL_PROCESS>* m_processes;

	Handle m_objectId;
	std::list<PSPIN_LOCK>* m_spinLocks;
	std::list<PSEMAPHORE>* m_semaphores;

	//Sections
	PIMAGE_SECTION_HEADER m_pdata;

	//Queues
	//std::queue<uint32_t> readyQueue;
	//std::list<uint32_t> sleepQueue;
};

