#pragma once
#include <cstdint>
#include "MetalOS.Kernel.h"
#include "UserThread.h"

/// <summary>
/// Kernel thread implementation.
/// </summary>
class KThread
{
public:
	static const size_t StackPageCount = 8;
	
	static uint32_t LastId;

	KThread(ThreadStart start, void* arg, void* context, UserThread* userThread = nullptr);

	void Run();

	uint32_t GetId() const
	{
		return m_id;
	}

	UserThread* GetUserThread() const
	{
		return m_userThread;
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

	//TODO: x64_CONTEXT_SIZE is const, pipe constant from masm to c
	void* m_context;//Pointer to x64 CONTEXT structure (masm)
	UserThread* m_userThread;
};

