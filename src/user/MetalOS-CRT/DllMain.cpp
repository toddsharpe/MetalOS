#include "user/MetalOS-CRT/CRT.h"
#include "user/MetalOS.h"

const size_t heapSize = 0x1000000; //16MB
bool DllMain(HModule handle, DllReason reason)
{
	switch (reason)
	{
		case DllReason::ProcessAttach:
		{
			//Back the CRT heap with a VirtualAlloc'd region (Lib/StaticHeap.h).
			void* const heap = VirtualAlloc(nullptr, heapSize);
			if (!heap)
				return false;

			CrtHeap.Open(heap, heapSize);
		}
		break;
	}

	return true;
}
