#pragma once

#include "msvc.h"
#include <list>
#include <map>
#include "MetalOS.Kernel.h"
#include "HyperV.h"
#include "KSemaphore.h"

class Scheduler
{
public:
	Scheduler();

	void Schedule();
	void Add(KernelThread& thread);
	void Sleep(nano_t value);
	WaitStatus SemaphoreWait(KSemaphore* semaphore, nano100_t timeout);
	void SemaphoreRelease(KSemaphore* semaphore, size_t count);

	void Display();

	bool Enabled;

private:
	//If event type objects was a class with add/remove methods
	//We could just keep the semaphore/event pointer and call remove
	//blindly

	void DisplayThread(KernelThread& thread);

	HyperV m_hyperv;
	std::list<uint32_t> m_readyQueue; //make priority queue of pairs?
	std::list<uint32_t> m_sleepQueue;
	std::list<uint32_t> m_semaphoreTimeouts;
	std::map<KSemaphore*, std::list<uint32_t>> m_semaphoreWaits;
};

