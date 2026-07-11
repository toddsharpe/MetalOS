#include "user/MetalOS-CRT/CRT.h"
#include "user/MetalOS.h"

//Core crt
#include "core_crt/core_crt.c"

#include "user/MetalOS-CRT/stdio.cpp"
#include "user/MetalOS-CRT/stdlib.cpp"
#include "user/MetalOS-CRT/string.cpp"

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
