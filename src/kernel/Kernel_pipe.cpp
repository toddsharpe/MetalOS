#include "kernel/Kernel_pipe.h"
#include "kernel/Kernel_heap.h"
#include "kernel/Objects/KPipe.h"

void KePipeUnref(KPipe& pipe, KRef& ref)
{
	ref.Decrement();
	if (!pipe.IsUnused())
		return;

	//No handle and no blocked thread can reach it any more
	pipe.Free();
	KeFree(&pipe, AllocType::Kernel); //Matches CreatePipe's allocation
}
