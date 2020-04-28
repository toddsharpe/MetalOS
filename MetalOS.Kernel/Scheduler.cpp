#include "Scheduler.h"
#include "Main.h"
#include "x64_support.h"
#include "x64.h"

Scheduler::Scheduler() : m_hyperv(), m_readyQueue(), m_sleepQueue()
{
}

void Scheduler::Schedule()
{
	const uint64_t tsc = m_hyperv.ReadTsc();
	KernelThread* current = kernel.GetCurrentThread();
	
	//Display();

	//Promote off sleep queue
	if (m_sleepQueue.size() > 0)
	{
		auto it = m_sleepQueue.begin();
		while (it != m_sleepQueue.end())
		{
			const uint32_t id = *it;
			KernelThread* item = kernel.GetKernelThread(id);
			//Print("  %d = 0x%x\n", id, item->SleepWake);
			if (item->SleepWake > 0 && item->SleepWake <= tsc)
			{
				item->State = ThreadState::Ready;
				item->SleepWake = 0;
				m_readyQueue.push_back(id);
				//Print("  Awake!\n");

				it = m_sleepQueue.erase(it);
			}
			else
			{
				it++;
			}
		}
	}

	if (!m_readyQueue.empty())
	{
		//Save current context
		if (x64_save_context(current->Context) == 0)
		{
			//Queue current thread if it was preempted
			if (current->State == ThreadState::Running)
			{
				current->State = ThreadState::Ready;
				m_readyQueue.push_back(current->Id);
			}

			//Get next thread
			uint32_t nextId = m_readyQueue.front();
			m_readyQueue.pop_front();
			KernelThread* next = kernel.GetKernelThread(nextId);
			Assert(next);

			//Print("  Schedule: 0x%x -> 0x%x\n", current->Id, next->Id);

			//Switch to thread
			next->State = ThreadState::Running;
			x64::SetKernelTEB(next->TEB);
			x64_swapgs();
			x64_load_context(next->Context);//Does not return
		}
	}
}

void Scheduler::Sleep(nano_t value)
{
	//Print("Scheduler::Sleep\n");
	KernelThread* current = kernel.GetCurrentThread();
	
	const nano100_t value100 = value / 100;
	const nano100_t tscStart = m_hyperv.ReadTsc();
	current->SleepWake = tscStart + value100;
	current->State = ThreadState::Waiting;
	m_sleepQueue.push_back(current->Id);

	//Print("  Wake: 0x%016x\n", current->SleepWake);

	this->Schedule();
}

void Scheduler::Add(KernelThread& thread)
{
	DisplayThread(thread);
	switch (thread.State)
	{
	case ThreadState::Ready:
		m_readyQueue.push_back(thread.Id);
		break;
	default:
		Assert(false);
	}
}

//TODO: somehow export struct from asm
struct x64_context
{
	uint64_t R12;
	uint64_t R13;
	uint64_t R14;
	uint64_t R15;
	uint64_t Rdi;
	uint64_t Rsi;
	uint64_t Rbx;
	uint64_t Rbp;
	uint64_t Rsp;
	uint64_t Rip;
	uint64_t Rflags;
};

void Scheduler::DisplayThread(KernelThread& thread)
{
	Print("Thread %d\n  Start:0x%016x\n  Arg: 0x%016x\n",
		thread.Id,
		thread.Start,
		thread.Arg
	);
	x64_context* context = (x64_context*)thread.Context;
	Print("  Rbp: 0x%016x Rsp: 0x%016x Rip: 0x%016x RFlags:0x%08x\n",
		context->Rbp, context->Rsp, context->Rip, (uint32_t)context->Rflags);
}

void Scheduler::Display()
{
	Print("Scheduler::Schedule - Ready: %d, Sleep: %d\n", m_readyQueue.size(), m_sleepQueue.size());
	if (!m_readyQueue.empty())
	{
		Print("  Ready: ");
		for (const auto& i : m_readyQueue)
		{
			Print("%x ", i);
		}
		Print("\n");
	}
	if (!m_sleepQueue.empty())
	{
		Print("  Sleep: ");
		for (const auto& i : m_sleepQueue)
		{
			KernelThread* t = kernel.GetKernelThread(i);
			Print("%x(0x%016x) ", i, t->SleepWake);
		}
		Print("\n");
	}
}
