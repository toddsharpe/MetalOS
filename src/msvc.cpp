#include <stdint.h>
#include <exception>

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

	_Prhand _Raise_handler = nullptr; // define raise handler pointer
}

extern "C" void _cdecl _invoke_watson(wchar_t const* _Expression, wchar_t const* _FunctionName, wchar_t const* _FileName, unsigned int _LineNo, uintptr_t _Reserved)
{

}

void __cdecl _invalid_parameter(char const*, char const*, char const*, unsigned int, int)
{

}


extern "C" void __cdecl _invalid_parameter(
	wchar_t const* const expression,
	wchar_t const* const function_name,
	wchar_t const* const file_name,
	unsigned int   const line_number,
	uintptr_t      const reserved
)
{

}

extern "C" __declspec(noreturn) void __cdecl _invalid_parameter_noinfo_noreturn()
{
	_invalid_parameter(nullptr, nullptr, nullptr, 0, 0);
	_invoke_watson(nullptr, nullptr, nullptr, 0, 0);
}

extern "C" int _cdecl _purecall(void)
{
	return 0;
}


extern "C" void _cdecl __std_exception_copy(const struct __std_exception_data* src,
	struct __std_exception_data* dst)
{
	
}

extern "C" void _cdecl __std_exception_destroy(struct __std_exception_data* data)
{

}

extern "C" int _CrtDbgReport(int _ReportType, char const* _FileName, int _Linenumber, char const* _ModuleName, char const* _Format, ...)
{
	return 0;
}
