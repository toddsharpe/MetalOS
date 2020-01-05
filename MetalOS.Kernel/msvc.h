#pragma once

//Invalid param
void __cdecl _invalid_parameter(char const*, char const*, char const*, unsigned int, int);
#define _CRT_SECURE_INVALID_PARAMETER(expr) _invalid_parameter(#expr, __FUNCTION__, __FILE__, __LINE__, 0)

//Disable exceptions in STL
#define _HAS_EXCEPTIONS 0
#define _KERNEL_MODE 1

//Define X errors:
namespace std
{
	[[noreturn]] void __cdecl _Xbad_alloc();
	[[noreturn]] void __cdecl _Xinvalid_argument(const char*);
	[[noreturn]] void __cdecl _Xlength_error(const char*);
	[[noreturn]] void __cdecl _Xout_of_range(const char*);
	[[noreturn]] void __cdecl _Xoverflow_error(const char*);
	[[noreturn]] void __cdecl _Xruntime_error(const char*);
}

//ChkStk
extern "C" void __chkstk();

