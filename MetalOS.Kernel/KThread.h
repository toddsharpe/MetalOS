#pragma once
#include <cstdint>
#include "MetalOS.Kernel.h"
#include "UserThread.h"

class KThread
{
	friend class Scheduler;

public:
	static uint32_t LastId;

	KThread(ThreadStart start, void* arg, UserThread* userThread = nullptr);
	~KThread();

	void Run();
	void InitContext(void* entry);

	uint32_t GetId() const;
	void* GetStackPointer() const;
	UserThread* GetUserThread() const;

	void Display() const;


private:
	static const size_t StackPages = 8;

	uint32_t m_id;
	ThreadStart m_start;
	void* m_arg;

	void* m_context;
	void* m_stackPointer; //Points to bottom of stack minus ArchStackReserve
	void* m_stackAllocation;//Points to top of stack

	//Scheduler
	ThreadState m_state;
	WaitStatus m_waitStatus;
	nano100_t m_sleepWake;
	void* m_event;

	UserThread* m_userThread;
};

