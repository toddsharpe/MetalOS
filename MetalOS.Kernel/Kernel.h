#pragma once

#include <efi.h>
#include <LoaderParams.h>
#include "MetalOS.Kernel.h"
#include "Display.h"
#include "LoadingScreen.h"
#include <PageTablesPool.h>
#include "MemoryMap.h"

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

private:
	//Save from LoaderParams
	uintptr_t m_address;
	size_t m_imageSize;
	EFI_RUNTIME_SERVICES m_runtime;

	PageTablesPool* m_pPagePool;
	MemoryMap* m_pMemoryMap;

	Display* m_pDisplay;
	LoadingScreen* m_pLoading;

	size_t m_lastProcessId;
	std::list<KERNEL_PROCESS>* m_processes;

	size_t m_lastSemaphoreId;
	std::list<uint32_t, PSEMAPHORE>* m_semaphores;

	//Queues
	//std::queue<uint32_t> readyQueue;
	//std::list<uint32_t> sleepQueue;
};

