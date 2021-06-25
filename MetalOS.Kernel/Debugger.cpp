#include "Debugger.h"
#include "Main.h"

Debugger::Debugger(Driver& driver) : m_state(State::Inactive), m_driver(driver)
{

}

void Debugger::Initialize()
{
	kernel.CreateKernelThread(Debugger::ThreadLoop, this);
}

uint32_t Debugger::ThreadLoop()
{
	switch (m_state)
	{

	}

	return 0;
}
