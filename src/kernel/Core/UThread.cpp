#include "kernel/Core/UThread.h"

static uint32_t LastId = 0;

UThread::UThread(UProcess& process, ThreadEnvironmentBlock& teb) :
	Process(process),
	Thread(),
	Stack(),
	Name(),
	Id(++LastId),
	StackBase(),
	Refs(1),
	PipeWriteLink(),
	PipeWriteQueue(),
	PipeWait(),
	EventWait(),
	m_context(),
	m_fpu(),
	m_teb(teb)
{
	ArchInitFpu(m_fpu);
}
