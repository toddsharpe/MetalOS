#include "MetalOS.lib.h"

bool MetalOsLibDebug = false;

void LibPrint(const char* format, ...)
{
	if (!MetalOsLibDebug)
		return;

	va_list args;
	va_start(args, format);
	Print(format, args);
	va_end(args);
}
