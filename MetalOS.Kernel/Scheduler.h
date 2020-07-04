#pragma once

#include "msvc.h"
#include <list>
#include <map>
#include "MetalOS.Kernel.h"
#include "HyperV.h"
#include "KSemaphore.h"
#include "KThread.h"

class Scheduler
{
public:
	Scheduler(KThread& bootThread);

	void Schedule();
	void Add(KThread& thread);
	void Sleep(nano_t value);

	WaitStatus SemaphoreWait(KSemaphore* semaphore, nano100_t timeout);
	void SemaphoreRelease(KSemaphore* semaphore, size_t count);

	void SetCurrentThread(KThread& thread);
	KThread* GetCurrentThread();

	void Display();

	bool Enabled;

private:
	struct CpuContext
	{
		CpuContext* SelfPointer;
		KThread* Thread;
	};

	CpuContext* GetCpuContext();

	HyperV m_hyperv; //TODO: Clock TSC interface
	std::list<uint32_t> m_readyQueue; //make priority queue of pairs?
	std::list<uint32_t> m_sleepQueue;
	std::list<uint32_t> m_semaphoreTimeouts;
	std::map<KSemaphore*, std::list<uint32_t>> m_semaphoreWaits;
};

