#pragma once

#include "UserProcess.h"

class UserThread
{
public:
	static uint32_t LastId;

	UserThread(ThreadStart startAddress, void* arg, void* stack, UserProcess& process);

	UserProcess& GetProcess()
	{
		return m_process;
	}

	void* GetContext()
	{
		return m_context;
	}

	ThreadEnvironmentBlock* GetTEB() const
	{
		return m_teb;
	}

	void Display();
	void DisplayDetails();

private:
	uint32_t m_id;
	UserProcess& m_process;
	ThreadEnvironmentBlock* m_teb;
	void* m_context;
};

