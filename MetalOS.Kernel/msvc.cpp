#include "msvc.h"

#include "Kernel.h"
#include "Assert.h"

namespace std
{
	void __cdecl _Xbad_alloc()
	{
		Assert(false);
	}
	void __cdecl _Xinvalid_argument(const char*)
	{
		Assert(false);
	}
	void __cdecl _Xlength_error(const char*)
	{
		Assert(false);
	}
	void __cdecl _Xout_of_range(const char*)
	{
		Assert(false);
	}
	void __cdecl _Xoverflow_error(const char*)
	{
		Assert(false);
	}
	void __cdecl _Xruntime_error(const char*)
	{
		Assert(false);
	}

}

extern "C" void __cdecl __CxxFrameHandler3()
{
	Assert(false);
}

extern "C" void __cdecl __CxxFrameHandler4()
{
	Assert(false);
}

__declspec(noreturn) void __stdcall _CxxThrowException(void* pExceptionObject, void* pThrowInfo)
{
	Assert(false);
}

void __cdecl _invoke_watson(char const*, char const*, char const*, unsigned int, int)
{
	Assert(false);
}

void __cdecl _invalid_parameter(char const* expression, char const* function, char const* file, unsigned int line, int reserved)
{
	kernel.Printf("_invalid_parameter\n");
	kernel.Printf("  Expr: %s\n", expression);
	kernel.Printf("  Func: %s\n", function);
	kernel.Printf("  File: %s\n", file);
	kernel.Printf("  Line: %d\n", line);
	kernel.Printf("  Rsvd: %d\n", reserved);
	Assert(false);
}

extern "C" void __chkstk()
{

}

extern "C" int _cdecl _purecall(void)
{
	return 0;
}
