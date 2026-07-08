#include "core_crt/stdarg.h"

void DebugBreak();
void DebugPrintf(const char* format, ...);
void Bugcheck(const char* file, const char* line, const char* format, ...);
