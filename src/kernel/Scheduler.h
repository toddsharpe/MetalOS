#pragma once

#include "new.h"
#include "kernel/KThread.h"
#include "Lib/StaticVector.h"
#include "Lib/List.h"

typedef nano100_t (*ReadTsc)();

//TODO(tsharpe): Scheduler should be a namespace not a class!
class KProcess;
class Scheduler
{
public:
	static KThread& GetThread();
	static UThread& GetUThread();
	static UProcess& GetUProcess();

	Scheduler(const ReadTsc readTsc);

	void Initialize();
	KThread* CreateReady(const KThreadStart start, void* const arg, const CString& name);
	void Schedule();

	//Currently running thread
	KThread& GetCurrentThread();

	void Sleep(const nano_t value);
	KWaitResult ObjectWait(KSignalObject& object, const milli_t timeout = TimeoutMax);

	void KillThread(KThread& thread);
	void KillProcess(UProcess& process);

	void Display() const;

	bool Enabled;

private:
	static constexpr size_t MaxKThreads = 32;

	struct CpuContext
	{
		CpuContext() :
			SelfPointer(*this),
			Thread()
		{}
		
		const CpuContext& SelfPointer;
		KThread* Thread;
	};
	static_assert(offsetof(CpuContext, SelfPointer) == 0, "x64_SYSTEMCALL asm invalid");
	static_assert(offsetof(CpuContext, Thread) == 8, "x64_SYSTEMCALL asm invalid");

	//Cpu context
	CpuContext m_cpu;

	//Platform
	ReadTsc m_readTsc;

	//Threads and current thread
	size_t m_threadIndex;
	StaticVector<KThread*, MaxKThreads> m_threads;
	ObjectPool<KThread, MaxKThreads> m_threadPool;
};
