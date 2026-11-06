#include "user/MetalOS.h"
#include "x64/intrin.h"
#include "core_crt/stddef.h"
#include "errno.h"

extern "C"
{
	//errno lives in the calling thread's TEB; RT's GetTEB isn't exported, so read it the same way
	int* _errno(void)
	{
		ThreadEnvironmentBlock* const teb = (ThreadEnvironmentBlock*)__readgsqword(offsetof(ThreadEnvironmentBlock, SelfPointer));
		return &teb->Errno;
	}
}
