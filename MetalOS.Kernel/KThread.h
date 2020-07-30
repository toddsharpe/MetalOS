#pragma once
#include <cstdint>
#include "MetalOS.Kernel.h"
#include "UserThread.h"

class KThread
{
public:
	static const size_t StackPageCount = 8;
	
	static uint32_t LastId;

	KThread(ThreadStart start, void* arg, UserThread* userThread = nullptr);
	~KThread();

	void Run();

	void InitContext(void* entry, void* stack);

	uint32_t GetId() const
	{
		return m_id;
	}

	UserThread* GetUserThread() const
	{
		return m_userThread;
	}

	void* GetStack() const
	{
		x64_context* ctx = (x64_context*)m_context;
		return (void*)ctx->Rsp;
	}

	void Display();

	friend class Scheduler;

	//TODO: get Run working and make these private
	ThreadStart m_start;
	void* m_arg;

private:
	uint32_t m_id;

	//Scheduler
	ThreadState m_state;
	WaitStatus m_waitStatus;
	nano100_t m_sleepWake;
	void* m_event;

	void* m_context;
	UserThread* m_userThread;
};

