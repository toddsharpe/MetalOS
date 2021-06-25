#pragma once
#include "Driver.h"

class Debugger
{
public:
	Debugger(Driver& driver);
	void Initialize();

	static uint32_t ThreadLoop(void* arg) { return ((Debugger*)arg)->ThreadLoop(); };

private:
	enum class State
	{
		Inactive,
		Listening
	};

	uint32_t ThreadLoop();

	State m_state;
	Driver& m_driver;
};

