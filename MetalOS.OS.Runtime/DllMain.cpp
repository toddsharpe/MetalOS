#include <MetalOS.h>
#include "Heap.h"
#include "Debug.h"

Heap CrtHeap;

uint32_t __declspec(dllexport) DllMain(Handle handle, DllEntryReason reason)
{
	switch (reason)
	{
	case DllEntryReason::ProcessAttach:
	{
		const size_t heapSize = 0x2000000; //32MB
		CrtHeap.Initialize(heapSize);
	}
		break;

	default:
		//Do nothing
		break;
	}

	return true;
}
