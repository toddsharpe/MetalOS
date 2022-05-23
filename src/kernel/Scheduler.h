#pragma once


#include <list>
#include <map>
#include "MetalOS.Kernel.h"
#include "HyperV.h"
#include "KSemaphore.h"
#include "KThread.h"
#include "UserPipe.h"
#include "KSignalObject.h"
#include "KPredicate.h"
#include <unordered_map>

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

	void AddReady(KThread* thread);
	void Sleep(nano_t value);
	void KillCurrentThread();
	void KillThread(KThread* thread);
	void KillCurrentProcess();

	//Semaphores
	WaitStatus SemaphoreWait(KSemaphore& semaphore, nano100_t timeout);
	void SemaphoreRelease(KSemaphore& semaphore);

	WaitStatus PipeReadWait(UserPipe& pipe, void* buffer, const size_t size);
	WaitStatus PipeWriteWait(UserPipe& pipe, const void* buffer, const size_t size);
	void PipeWait(UserPipe& pipe, bool read);
	void PipeSignal(UserPipe& pipe, bool read);

	//Messages
	Message* MessageWait();

	bool ObjectSignalledOne(KSignalObject& object);
	void ObjectSignalled(KSignalObject& object);
	WaitStatus ObjectWait(KSignalObject& object, const nano_t timeout);

	void Display() const;

	bool Enabled;

private:
	static bool MessageReceived(void* arg);

	struct CpuContext
	{
		CpuContext* SelfPointer;
		KThread* Thread;
	};

	enum class Operation
	{
		Read,
		Write
	};

	//todo: work on this
	typedef bool(*ReadyPredicate)(void* arg);
	struct WaitEvent
	{
		void* Context;
		union
		{
			size_t Count;
		};
		Operation Op;
	};

	static CpuContext* GetCpuContext();
	void SetCurrentThread(KThread& thread);

	HyperV m_hyperv; //TODO: Clock TSC interface
	std::list<KThread*> m_readyQueue;
	std::list<KThread*> m_sleepQueue;

	struct PipeWaitEvent
	{
		KThread* Thread;
		size_t Count;
	};

	struct Wait
	{
		KThread* Thread;
		nano100_t Deadline;
	};

	std::map<UserPipe*, PipeWaitEvent> m_pipeReadWaits;
	std::map<UserPipe*, PipeWaitEvent> m_pipeWriteWaits;

	std::map<KSignalObject*, std::list<Wait>> m_signalWaits;
	std::map<KPredicate*, KThread*> m_predicates;
};
