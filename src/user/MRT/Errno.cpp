#include "User/MRT/Runtime.h"

extern "C"
{
	uint32_t GetErrno()
	{
		return Runtime::GetTEB()->Error;
	}
}
