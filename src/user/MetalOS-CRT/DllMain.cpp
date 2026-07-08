#include "user/MetalOS-CRT/CRT.h"

//Core crt
#include "core_crt/core_crt.c"

#include "user/MetalOS-CRT/Heap.cpp"
#include "user/MetalOS-CRT/stdio.cpp"
#include "user/MetalOS-CRT/stdlib.cpp"
#include "user/MetalOS-CRT/string.cpp"

const size_t heapSize = 0x1000000; //16MB
size_t DllMain(HModule handle)
{
	//Initialize heap
	CrtHeap.Initialize(heapSize);

	//Do nothing
	return true;
}
