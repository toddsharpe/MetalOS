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
