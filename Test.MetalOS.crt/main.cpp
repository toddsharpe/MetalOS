#include "crt_string.h"
//#include <Windows.h>
//
//extern "C" int __cdecl vprintf(const char* format, va_list args)
//{
//	char buffer[1024];
//	int ret = wvsprintf(buffer, format, args);
//
//	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
//	WriteConsole(hOut, buffer, ret, nullptr, nullptr);
//	CloseHandle(hOut);
//
//	return ret;
//}
//
//extern "C" int __cdecl printf(const char* format, ...)
//{
//	va_list args;
//	va_start(args, format);
//	int ret = vprintf(format, args);
//	va_end(args);
//
//	return ret;
//}

#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#define Assert(x) if (!(x)) { Fail("File: " __FILE__, "Line: " STR(__LINE__),  "Assert: " #x); }

void Fail(const char* file, const char* line, const char* assert)
{
	//printf("%s\n", file);
	//printf("%s\n", line);
	//printf("%s\n", assert);
	__debugbreak();
}

int main(int argc, char** argv)
{
	Assert(strnicmp("test", "Test", 4) == 0);
}

extern "C" void __cdecl mainCRTStartup()
{
	int ret = main(0, nullptr);
	//ExitProcess(ret);
}
