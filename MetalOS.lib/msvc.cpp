#include "msvc.h"


namespace std
{
	void __cdecl _Xbad_alloc()
	{
	}
	void __cdecl _Xinvalid_argument(const char*)
	{
	}
	void __cdecl _Xlength_error(const char*)
	{
	}
	void __cdecl _Xout_of_range(const char*)
	{
	}
	void __cdecl _Xoverflow_error(const char*)
	{
	}
	void __cdecl _Xruntime_error(const char*)
	{
	}

}

extern "C" void __cdecl __CxxFrameHandler3()
{
}

__declspec(noreturn) void __stdcall _CxxThrowException(void* pExceptionObject, void* pThrowInfo)
{
}

void __cdecl _invoke_watson(char const*, char const*, char const*, unsigned int, int)
{

}

void __cdecl _invalid_parameter(char const*, char const*, char const*, unsigned int, int)
{
}

extern "C" void __chkstk()
{

}

extern "C" int _cdecl _purecall(void)
{
	return 0;
}
