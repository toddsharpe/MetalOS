#pragma once

#include "msvc.h"
#include <list>
#include <map>
#include "MetalOS.Kernel.h"
#include "HyperV.h"
#include "KSemaphore.h"
#include "KThread.h"

class Debugger;
class Scheduler
{
	friend Debugger;
public:
	Scheduler(KThread& bootThread);

	void Schedule();
	static KThread* GetCurrentThread();
	UserThread* GetCurrentUserThread() const;
	UserProcess& GetCurrentProcess() const;

	void AddReady(KThread& thread);
	void Sleep(nano_t value);
	void KillThread();

	size_t Suspend(KThread& thread);
	size_t Resume(KThread& thread);

	WaitStatus SemaphoreWait(KSemaphore* semaphore, nano100_t timeout);
	void SemaphoreRelease(KSemaphore* semaphore, size_t count);

	Message* MessageWait();

	void Display() const;

	bool Enabled;

private:
	struct CpuContext
	{
		CpuContext* SelfPointer;
		KThread* Thread;
	};

	static CpuContext* GetCpuContext();
	void SetCurrentThread(KThread& thread);
	void Remove(KThread*& thread);
	void RemoveFromEvent(KThread*& thread);

	HyperV m_hyperv; //TODO: Clock TSC interface
	std::list<KThread*> m_readyQueue;
	std::list<KThread*> m_sleepQueue;
	std::list<KThread*> m_timeouts;
	std::map<void*, std::list<KThread*>> m_events;

	//Message waits (should be part of event framework)
	std::list<KThread*> m_messageWaits;
};

