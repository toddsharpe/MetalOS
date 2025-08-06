#include "user/crt/CRT.h"

//Core crt
#include "core_crt/core_crt.c"

#include "user/crt/Heap.cpp"
#include "user/crt/stdio.cpp"
#include "user/crt/stdlib.cpp"
#include "user/crt/string.cpp"

const size_t heapSize = 0x2000000; //32MB
size_t DllMain(HModule handle)
{
	//Initialize heap
	CrtHeap.Initialize(heapSize);

	//Do nothing
	return true;
}
