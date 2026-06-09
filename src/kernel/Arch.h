#pragma once

#include "kernel/Arch_x64/Arch.h"
#include "kernel/Arch_x64/StackWalk.h"

extern "C"
{
	void ArchInitialize();

	//KThreads
	void ArchInitContext(void *context, void *const entry, void *const stack);
	bool ArchSaveContext(void *context); // Returns 0
	void ArchLoadContext(void *context); // Returns 1

	//UThreads
	 void ArchUserThreadStart(void* context, void* teb);
}
