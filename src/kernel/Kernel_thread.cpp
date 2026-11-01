#include "kernel/Api.h"
#include "kernel/Scheduler.h"
#include "kernel/KThread.h"
#include "kernel/UThread.h"
#include "kernel/UProcess.h"
#include "Assert.h"

//Kernel globals (defined in Kernel.cpp).
extern KProcess m_process;

namespace
{
	//Allocate and initialize a kernel thread without making it schedulable yet
	KThread* AllocKThread(const KThreadStart start, void* const arg, const char* const name)
	{
		KThread* thread = KeAlloc<KThread>(AllocType::Kernel, start, arg, CString(name));
		thread->Init(&KThreadInit);
		thread->Process = &m_process;
		return thread;
	}
}

KThread* KeCreateThread(const KThreadStart start, void* const arg, const char* const name)
{
	KThread* thread = AllocKThread(start, arg, name);
	Scheduler::MakeReady(*thread);

	return thread;
}

void KeExitThread()
{
	KThread& current = Scheduler::GetThread();
	KeExitThread(current);
}

void KeExitThread(KThread& thread)
{
	Scheduler::KillThread(thread);
}

void KeSleepThread(const nano_t time)
{
	Scheduler::Sleep(time);
}

void KeYield()
{
	Scheduler::Schedule();
}

void KThreadInit()
{
	KThread& current = Scheduler::GetCurrentThread();

	//Run thread
	current.Start();
	Printf("Thread exit: %d\n", current.Id);

	//Exit thread
	KeExitThread();

	Unreachable();
}

//If entry is null, use InitProcess. Otherwise use InitThread with this as its arg
UThread* KeCreateUThread(UProcess& process, const size_t stackSize, const UThreadStart entry, void* const arg)
{
	//Checked first: the TEB slot and stack taken below can't be given back
	if (!Scheduler::HasRoom())
		return nullptr;

	//The user side can fail, so build it before the KThread, which would otherwise need unwinding
	UThread* const user = process.CreateThread(stackSize, entry, arg);
	if (!user)
		return nullptr;

	//Build the thread fully before readying it: at boot a timer tick could otherwise run it before UserThread is set
	KThread* kThread = AllocKThread(&UThreadInit, nullptr, "");
	Assert(kThread);
	kThread->Process = &process;
	kThread->UserThread = user;
	user->Thread = kThread;

	Scheduler::MakeReady(*kThread);
	return user;
}

uint32_t UThreadInit(void* const arg)
{
	UNUSED(arg);

	KThread& current = Scheduler::GetCurrentThread();
	Assert(current.UserThread);
	UThread& user = *current.UserThread;
	user.Start();

	//TODO(tsharpe): Exit code
	return 0;
}

KWaitResult KeWait(KSignal& obj, const milli_t timeout)
{
	return Scheduler::ObjectWait(obj, timeout);
}
