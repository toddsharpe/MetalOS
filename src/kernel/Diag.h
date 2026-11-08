#pragma once

#include "core_crt/stdarg.h"
#include "kernel/Arch.h"

struct KProcess;
class KPipe;

void VPrintf(const char* format, va_list args); //Not a Printf overload: va_list is char*, so it would capture Printf("%s", buffer)
void TracePrint(KPipe* const output, const char* const format, ...); //Kernel console, and output too when it isn't null
void PrintStack();
void PrintStack(const Arch::Context* context, const KProcess& process, KPipe* const output = nullptr); //Prints like TracePrint
void PrintBytes(const void* data, const size_t length);
