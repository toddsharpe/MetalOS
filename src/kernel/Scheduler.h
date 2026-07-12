#pragma once

#include "new.h"
#include "kernel/KThread.h"

typedef nano100_t (*ReadTsc)();

class KProcess;

//Singleton kernel scheduler. A namespace, not a class: there is exactly one, and it needs no
//friendship into KThread/UProcess (they expose the state it touches). All internal state lives
//in Scheduler.cpp. Enabled gates preemption from the timer ISR.
namespace Scheduler
{
	KThread& GetThread();
	UThread& GetUThread();
	UProcess& GetUProcess();

	void Initialize(KProcess& proc, const ReadTsc readTsc);
	void MakeReady(KThread& thread);
	void Schedule();

	//Currently running thread
	KThread& GetCurrentThread();

	void Sleep(const nano_t value);
	KWaitResult ObjectWait(KSignal& object, const milli_t timeout = TimeoutMax);

	void KillThread(KThread& thread);
	void KillProcess(UProcess& process);

	void Display();

	//Preemption gate; the timer ISR only reschedules when set. Defined in Scheduler.cpp.
	extern bool Enabled;
}
