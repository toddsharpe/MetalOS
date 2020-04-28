#pragma once

#include "msvc.h"
#include <list>
#include <map>
#include "MetalOS.Kernel.h"
#include "HyperV.h"

class Scheduler
{
public:
	Scheduler();

	void Schedule();
	void Add(KernelThread& thread);
	void Sleep(nano_t value);

	void Display();

private:
	static const uint64_t PreemptiveQuota = 0.01 * 1000000000 / 100;

	void DisplayThread(KernelThread& thread);

	HyperV m_hyperv;
	std::list<uint32_t> m_readyQueue; //make priority queue of pairs?
	std::list<uint32_t> m_sleepQueue;
};

