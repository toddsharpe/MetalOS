#pragma once

#include <MetalOS.h>

class Thread
{
public:
	static Thread* GetCurrent()
	{
		Thread* ret = new Thread(GetCurrentThread());
		return ret;
	}

	static void Sleep(const uint32_t milliseconds)
	{
		Sleep(milliseconds);
	}

	Thread(size_t stackSize, ThreadStart startAddress, void* arg)
	{
		m_thread = CreateThread(stackSize, startAddress, arg);
	}

	bool IsValid()
	{
		return m_thread != nullptr;
	}

	const uint32_t GetId() const
	{
		return GetThreadId(m_thread);
	}

private:
	Thread(HThread thread) :
		m_thread(thread)
	{

	}

	HThread m_thread;
};