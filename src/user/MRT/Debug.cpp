#include "user/MRT/Debug.h"

#include "core_crt/stdio.h"
#include "user/MetalOS.h"

void DebugBreak()
{
	__debugbreak();
}

void CDebugPrintf(const bool enabled, const char* format, ...)
{
	if (!enabled)
		return;
	
	va_list args;
	va_start(args, format);

	char buffer[512];
	int retval = vsprintf(buffer, format, args);
	buffer[retval] = '\0';
	va_end(args);

	DebugPrint(buffer);
}

void DebugPrintf(const char* format, ...)
{
	va_list args;
	va_start(args, format);

	char buffer[512];
	int retval = vsprintf(buffer, format, args);
	buffer[retval] = '\0';
	va_end(args);

	DebugPrint(buffer);
}

void Bugcheck(const char* file, const char* line, const char* format, ...)
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

	DebugPrintStack();

	ExitProcess(-2);
}
