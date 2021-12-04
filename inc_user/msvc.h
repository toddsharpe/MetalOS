#pragma once

//Invalid param
void __cdecl _invalid_parameter(char const*, char const*, char const*, unsigned int, int);
#define _STL_CRT_SECURE_INVALID_PARAMETER(expr) _invalid_parameter(#expr, __FUNCTION__, __FILE__, __LINE__, 0)
#define _STLCLRDB_REPORT(mesg, file, line)

//extern "C" void __cdecl _invoke_watson(wchar_t const* _Expression, wchar_t const* _FunctionName, wchar_t const* _FileName, unsigned int _LineNo, uintptr_t _Reserved);
__declspec(noreturn) void __stdcall _CxxThrowException(void* pExceptionObject, void* pThrowInfo);

extern "C" void __cdecl _Throw_bad_array_new_length();

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
	[[noreturn]] void __cdecl _XGetLastError();
}

//ChkStk
extern "C" void __chkstk();

