#include "MetalOS-RT.h"

//Core crt
#include "core_crt/core_crt.c"

#include "user/MetalOS-RT/Init.cpp"
#include "user/MetalOS-RT/Debug.cpp"
#include "user/MetalOS-RT/Loader.cpp"
#include "user/MetalOS-RT/Runtime.cpp"
#include "user/MetalOS-RT/Errno.cpp"

bool DllMain(HModule handle, DllReason reason)
{
	//Do nothing
	return true;
}
