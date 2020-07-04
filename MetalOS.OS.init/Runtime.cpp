#include "Runtime.h"

#include <intrin.h>
#include <cstddef>

ThreadEnvironmentBlock* Runtime::GetTEB()
{
	return (ThreadEnvironmentBlock*)__readgsqword(offsetof(ThreadEnvironmentBlock, SelfPointer));
}

ProcessEnvironmentBlock* Runtime::GetPEB()
{
	ThreadEnvironmentBlock* teb = GetTEB();
	return teb->PEB;
}
