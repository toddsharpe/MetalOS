#include <user/Debug.h>

#include <stdarg.h>
#include <stdio.h>
#include <intrin.h>
#include <shared/MetalOS.Types.h>
#include <user/MetalOS.h>

extern "C" void DebugBreak()
{
	__debugbreak();
}

extern "C" void DebugPrintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	char buffer[512];
	int retval = vsprintf(buffer, format, args);
	buffer[retval] = '\0';
	va_end(args);

	DebugPrint(buffer);
}

extern "C" void Bugcheck(const char* file, const char* line, const char* format, ...)
{
	va_list args;
	va_start(args, format);

	char buffer[512];
	int retval = vsprintf(buffer, format, args);
	buffer[retval] = '\0';
	va_end(args);
	
	DebugPrintf("Usermode Bugcheck\n");
	DebugPrintf("%s\n", file);
	DebugPrintf("%s\n", line);
	DebugPrintf("%s\n", buffer);
	ExitProcess(-1);
}
