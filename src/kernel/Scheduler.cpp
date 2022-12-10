#include "Kernel.h"
#include "Assert.h"

#include "Scheduler.h"
#include "KSemaphore.h"
#include <kernel/MetalOS.Arch.h>
#include "StackWalk.h"
#include "PortableExecutable.h"

Scheduler::Scheduler(KThread& bootThread) :
	Enabled(),
	m_hyperv(),
	m_threadIndex(),
	m_threads(),
	m_pipeReadWaits(),
	m_pipeWriteWaits(),
	m_signalWaits(),
	m_predicates()
{
	//Set boot thread as running
	bootThread.m_state = ThreadState::Running;
	
	//Initialize context for one CPU. Big TODO for more CPUs
	CpuContext* context = new CpuContext();
	context->SelfPointer = context;
	context->Thread = &bootThread;
	AddReady(&bootThread);

	//Write CPU state
	ArchSetUserCpuContext(context);
}

Scheduler::CpuContext* Scheduler::GetCpuContext()
{
	Assert(_readgsbase_u64() != 0);
	return (CpuContext*)__readgsqword(offsetof(CpuContext, SelfPointer));
}

KThread* Scheduler::GetCurrentThread()
{
	CpuContext* context = GetCpuContext();
	Assert(context);
	return context->Thread;
}

UserThread* Scheduler::GetCurrentUserThread() const
{
	KThread* current = GetCurrentThread();
	Assert(current);
	return current->GetUserThread();
}

UserProcess& Scheduler::GetCurrentProcess() const
{
	KThread* current = GetCurrentThread();
	Assert(current);
	UserThread* user = current->GetUserThread();
	Assert(user);

	return user->GetProcess();
}

void Scheduler::SetCurrentThread(KThread& thread)
{
	CpuContext* context = GetCpuContext();
	Assert(context);
	context->Thread = &thread;
}

void Scheduler::Schedule()
{
	Assert(Enabled);

	const uint64_t tsc = m_hyperv.ReadTsc();
	KThread* current = GetCurrentThread();
	AssertEqual(current, m_threads[m_threadIndex]);
	
	//Wake up sleeping threads
	for (KThread* thread : m_threads)
	{
		if (thread->m_sleepWake == 0)
			continue;

		Assert(thread->m_state == ThreadState::Sleeping);
		if (thread->m_sleepWake <= tsc)
		{
			thread->m_sleepWake = 0;
			thread->m_state = ThreadState::Ready;
		}
	}

	//Promote timeouts off queue
	if (!m_signalWaits.empty())
	{
		for (auto& pair : m_signalWaits)
		{
			//Loop through waits
			auto it = pair.second.begin();
			while (it != pair.second.end())
			{
				Wait& item = *it;
				if (item.Deadline <= tsc)
				{
					KThread* thread = item.Thread;
					Assert(thread);
					thread->m_waitStatus = WaitStatus::Timeout;
					thread->m_state = ThreadState::Ready;
					it = pair.second.erase(it);
				}
				else
				{
					it++;
				}
			}
		}
	}

	//Evaluate predicates
	if (!m_predicates.empty())
	{
		auto it = m_predicates.begin();
		while (it != m_predicates.end())
		{
			const KPredicate* item = it->first;
			Assert(item);
			if (item->IsSignalled())
			{
				KThread* thread = it->second;
				Assert(thread);
				thread->m_state = ThreadState::Ready;
				it = m_predicates.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	//Mark pre-empted thread as ready
	if (current->m_state == ThreadState::Running)
		current->m_state = ThreadState::Ready;

	// Select new thread, round robin
	while (true)
	{
		m_threadIndex = (m_threadIndex + 1) % m_threads.size();

		if (m_threads[m_threadIndex]->m_state == ThreadState::Ready)
			break;
	}

	KThread* next = m_threads[m_threadIndex];
	next->m_state = ThreadState::Running;
	if (next == current)
		return;

	//Save current context
	if (ArchSaveContext(current->m_context) == 0)
	{
		//Switch cr3 if needed
		UserThread* userThread = next->GetUserThread();
		if (userThread != nullptr)
		{
			const uintptr_t cr3 = userThread->GetProcess().GetCR3();
			ArchSetPagingRoot(cr3);
		}

		//Switch to thread
		SetCurrentThread(*next);
		//TODO: this isnt the current stack pointer of the context struct
		ArchSetInterruptStack(next->GetStackPointer());
		ArchLoadContext(next->m_context);
	}
}

void Scheduler::AddReady(KThread* thread)
{
	Assert(thread);
	thread->m_state = ThreadState::Ready;
	for (size_t i = 0; i < m_threads.size(); i++)
	{
		AssertNotEqual(m_threads[i], thread);
	}
	m_threads.push_back(thread);
}

void Scheduler::Sleep(nano_t value)
{
	KThread* current = GetCurrentThread();
	Assert(current);

	const nano100_t tscStart = m_hyperv.ReadTsc();
	const nano_t deadline = tscStart + value / 100;

	//TODO(tsharpe): Using ksleep slows system a lot.
	//KSleep sleep;
	//WaitStatus status = ObjectWait(sleep, deadline);
	//AssertEqual(status, WaitStatus::Timeout);

	current->m_sleepWake = deadline;
	current->m_state = ThreadState::Sleeping;
	this->Schedule();
}

void Scheduler::KillCurrentThread()
{
	KThread* current = GetCurrentThread();
	Assert(current);

	this->KillThread(current);

	this->Schedule();

	kernel.Printf("Schedule\n");
}

void Scheduler::KillThread(KThread* thread)
{
	kernel.Printf("KillThread %x\n", thread->GetId());

	//Mark thread deleted
	thread->m_state = ThreadState::Terminated;
	thread->Dispose();

	UserThread* user = thread->GetUserThread();
	if (user)
	{
		//todo: dispose user thread
		user->Deleted = true;
	}

	//TODO: determine if process needs to be cleaned

	this->ObjectSignalled(*thread);

	kernel.Printf("End\n");
}

void Scheduler::KillCurrentProcess()
{
	UserThread* thread = GetCurrentUserThread();
	UserProcess& process = thread->GetProcess();

	//Kill threads
	for (auto& thread : process.m_threads)
	{
		this->KillThread(thread);
	}

	//Clean up objects
	for (auto& item : process.m_objects)
	{
		UObject* userObject = item.second;
		KObject& obj = userObject->GetObject();
		
		if (userObject->IsPipe())
		{
			UserPipe& pipe = (UserPipe&)obj;
			if (userObject->GetType() == UserObjectType::ReadPipe)
			{
				pipe.ReaderCount--;
				if (pipe.ReaderCount == 0)
					this->PipeSignal(pipe, true);
			}
			else if (userObject->GetType() == UserObjectType::WritePipe)
			{
				pipe.WriterCount--;
				if (pipe.WriterCount == 0)
					this->PipeSignal(pipe, false);
			}
			else
				Assert(false);
		}
		
		delete userObject;

		//todo: signal?
		if (obj.IsClosed())
			obj.Dispose();
	}
	process.m_objects.clear();

	//TODO: clean up process
	process.m_state = ProcessState::Terminated;
	this->ObjectSignalled(process);

	this->Schedule();
}

void Scheduler::SemaphoreRelease(KSemaphore& semaphore)
{
	KThread* current = GetCurrentThread();
	Assert(current);

	semaphore.Signal(1);

	if (!m_signalWaits[&semaphore].size())
		return;

	this->ObjectSignalledOne(semaphore);
}

WaitStatus Scheduler::SemaphoreWait(KSemaphore& semaphore, nano100_t timeout)
{
	KThread* current = GetCurrentThread();
	Assert(current);

	bool wasSignaled = semaphore.IsSignalled();
	semaphore.Wait(1);
	
	if (wasSignaled)
		return WaitStatus::Signaled;
	return this->ObjectWait(semaphore, timeout * 100);
}

//If there is a message, return immediately
Message* Scheduler::MessageWait()
{
	KThread* current = GetCurrentThread();
	Assert(current);
	UserThread* user = current->GetUserThread();
	Assert(user);

	//If there is a message, return immediately
	Message* msg = user->DequeueMessage();
	if (msg)
		return msg;

	//Block
	current->m_state = ThreadState::MessageWait;
	KPredicate messageWait(&Scheduler::MessageReceived, current);
	m_predicates.insert({ &messageWait, current });

	//Context switch
	this->Schedule();

	//Return message
	msg = user->DequeueMessage();
	Assert(msg);
	return msg;
}

//todo we never drain the pipe wait

//Blocks until write operation completes or number of bytes requested has been read
WaitStatus Scheduler::PipeReadWait(UserPipe& pipe, void* buffer, const size_t size)
{
	KThread* current = GetCurrentThread();
	Assert(current);

	//If there is enough data, return immediately
	if (pipe.Read(buffer, size))
	{
		//If a writer is waiting, schedule it
		if (m_pipeWriteWaits.find(&pipe) != m_pipeWriteWaits.end())
		{
			PipeWaitEvent& wait = m_pipeWriteWaits[&pipe];
			if (wait.Count <= pipe.FreeSpace())
			{
				wait.Thread->m_state = ThreadState::Ready;
				m_pipeWriteWaits.erase(&pipe);
			}
		}

		return WaitStatus::None;
	}

	//Block waiting for write. We only support one waiter
	Assert(m_pipeReadWaits.find(&pipe) == m_pipeReadWaits.end());
	current->m_state = ThreadState::PipeWait;
	m_pipeReadWaits[&pipe] = { current, size };

	//Context switch
	this->Schedule();
	kernel.Printf("Restored\n");

	if (current->m_waitStatus == WaitStatus::BrokenPipe)
		return current->m_waitStatus;

	//Return message
	Assert(pipe.Read(buffer, size));
	return WaitStatus::Signaled;
}

//Blocks until buffer space is available
WaitStatus Scheduler::PipeWriteWait(UserPipe& pipe, const void* buffer, const size_t size)
{
	KThread* current = GetCurrentThread();
	Assert(current);

	if (pipe.Write(buffer, size))
	{
		//If a reader is waiting, schedule it
		if (m_pipeReadWaits.find(&pipe) != m_pipeReadWaits.end())
		{
			PipeWaitEvent& wait = m_pipeReadWaits[&pipe];
			if (wait.Count >= pipe.Count())
			{
				wait.Thread->m_state = ThreadState::Ready;
				m_pipeWriteWaits.erase(&pipe);
			}
		}

		return WaitStatus::None;
	}

	kernel.Printf("Blocking\n");

	//Block waiting for read. We only support one reader
	Assert(m_pipeWriteWaits.find(&pipe) == m_pipeWriteWaits.end());
	current->m_waitStatus = WaitStatus::None;
	current->m_state = ThreadState::PipeWait;
	m_pipeWriteWaits[&pipe] = { current, size };

	//Context switch
	this->Schedule();
	kernel.Printf("Restored\n");

	if (current->m_waitStatus == WaitStatus::BrokenPipe)
		return current->m_waitStatus;

	//Return message
	Assert(pipe.Write(buffer, size));
	return WaitStatus::Signaled;
}

void Scheduler::PipeWait(UserPipe& pipe, bool read)
{
	KThread* current = GetCurrentThread();
	Assert(current);
	
	kernel.Printf("PipeWait %d\n", read);

	WaitEvent event = {};
	event.Context = &pipe;
	event.Op = read ? Operation::Read : Operation::Write;

	if (read)
	{
		Assert(m_pipeReadWaits.find(&pipe) == m_pipeReadWaits.end());
		current->m_state = ThreadState::PipeWait;
		m_pipeReadWaits[&pipe] = { current, 0 };
	}
	else
	{
		Assert(m_pipeWriteWaits.find(&pipe) == m_pipeWriteWaits.end());
		current->m_state = ThreadState::PipeWait;
		m_pipeWriteWaits[&pipe] = { current, 0 };
	}

	this->Schedule();
}

//Read means signal a writer
//!Read means signal a reader
void Scheduler::PipeSignal(UserPipe& pipe, bool read)
{
	if (read)
	{
		auto it = m_pipeWriteWaits.find(&pipe);
		if (it != m_pipeWriteWaits.end())
		{
			const PipeWaitEvent event = it->second;
			event.Thread->m_waitStatus = WaitStatus::BrokenPipe;
			event.Thread->m_state = ThreadState::Ready;
			m_pipeWriteWaits.erase(&pipe);
		}
	}
	else
	{
		auto it = m_pipeReadWaits.find(&pipe);
		if (it != m_pipeReadWaits.end())
		{
			const PipeWaitEvent event = it->second;
			event.Thread->m_waitStatus = WaitStatus::BrokenPipe;
			event.Thread->m_state = ThreadState::Ready;
			m_pipeReadWaits.erase(&pipe);
		}
	}
}

void Scheduler::ObjectSignalled(KSignalObject& object)
{
	Assert(object.IsSignalled());

	if (!m_signalWaits[&object].size())
		return;

	//Ready threads waiting on object
	for (auto& wait : m_signalWaits[&object])
	{
		KThread* thread = wait.Thread;
		thread->m_waitStatus = WaitStatus::Signaled;
		thread->m_state = ThreadState::Ready;
	}
	m_signalWaits[&object].clear();
	m_signalWaits.erase(&object);
}

//Schedule one thread
bool Scheduler::ObjectSignalledOne(KSignalObject& object)
{
	if (object.Type != KObjectType::Semaphore)
		Assert(object.IsSignalled());

	if (!m_signalWaits[&object].size())
		return false;

	const Wait& wait = m_signalWaits[&object].front();
	m_signalWaits[&object].pop_front();
	//if (m_signalWaits[&object].size() == 0)
		//m_signalWaits.erase(&object);

	KThread* thread = wait.Thread;
	thread->m_waitStatus = WaitStatus::Signaled;
	thread->m_state = ThreadState::Ready;
	return true;
}

WaitStatus Scheduler::ObjectWait(KSignalObject& object, const nano_t timeout)
{
	KThread* current = GetCurrentThread();
	Assert(current);

	if (object.IsSignalled())
		return WaitStatus::Signaled;

	//Calculate deadline
	const nano100_t tscStart = m_hyperv.ReadTsc();
	const nano_t deadline = tscStart + timeout / 100;

	current->m_state = ThreadState::SignalWait;
	m_signalWaits[&object].push_back({ current, deadline });

	this->Schedule();
	return current->m_waitStatus;
}

void Scheduler::Display() const
{
	kernel.Printf("Scheduler::Display\n");
	kernel.Printf("    Threads: %d\n", m_threads.size());
	kernel.Printf("   SigWaits: %d\n", m_signalWaits.size());
	kernel.Printf("      Preds: %d\n", m_predicates.size());

	for (size_t i = 0; i < m_threads.size(); i++)
	{
		m_threads[i]->Display();
	}
	
	/*

	kernel.Printf("Scheduler::Schedule - Ready: %d, Sleep: %d, Predicates %d\n", m_readyQueue.size(), m_sleepQueue.size(), m_predicates.size());
	if (!m_readyQueue.empty())
	{
		kernel.Printf("  Ready: ");
		for (const auto& i : m_readyQueue)
		{
			kernel.Printf("i: 0x%016x - ", i);
			kernel.Printf("%x ", i->GetId());
		}
		kernel.Printf("\n");
	}
	if (!m_sleepQueue.empty())
	{
		kernel.Printf("  Sleep: ");
		for (const auto& i : m_sleepQueue)
		{
			kernel.Printf("i: 0x%016x - ", i);
			kernel.Printf("%x(0x%016x)\n", i->GetId(), i->m_sleepWake);
		}
		kernel.Printf("\n");
	}
	
	if (!m_predicates.empty())
	{
		kernel.Printf("  Predicates: ");
		for (const auto& i : m_predicates)
		{
			kernel.Printf("i: 0x%016x\n", i.second);
			kernel.Printf("%x ", i.second->GetId());
		}
		kernel.Printf("\n");
	}
	*/
}

bool Scheduler::MessageReceived(void* arg)
{
	KThread* item = (KThread*)arg;
	Assert(item);
	UserThread* user = item->GetUserThread();
	Assert(user);
	return user->HasMessage();
}

