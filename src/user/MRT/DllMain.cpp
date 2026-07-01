#include "MRT.h"

//Core crt
#include "core_crt/core_crt.c"

#include "user/MRT/Init.cpp"
#include "user/MRT/Debug.cpp"
#include "user/MRT/Loader.cpp"
#include "user/MRT/Runtime.cpp"
#include "user/MRT/Errno.cpp"
#include "user/MRT/Sockets.cpp"

size_t DllMain(HModule handle)
{
	//Do nothing
	return true;
}
