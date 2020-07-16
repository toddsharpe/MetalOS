#include "Debug.h"
#include <cstdarg>
#include <crt_stdio.h>
#include <MetalOS.h>

extern "C" void DebugPrintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	char buffer[255];
	int retval = crt_vsprintf(buffer, format, args);
	buffer[retval] = '\0';
	va_end(args);

	DebugPrint(buffer);
}

extern "C" void Bugcheck(const char* file, const char* line, const char* assert)
{
	DebugPrintf("Usermode Bugcheck\n");
	DebugPrintf("%s\n", file);
	DebugPrintf("%s\n", line);
	DebugPrintf("%s\n", assert);
	ExitProcess(-1);
}
