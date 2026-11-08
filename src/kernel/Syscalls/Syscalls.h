#pragma once

#include "user/MetalOS.h"
#include "kernel/Core/UProcess.h"
#include "kernel/Objects/UObject.h"

//Longer debug output is truncated by Printf's buffer anyway
static constexpr size_t MaxDebugPrint = 1024;

//Length of a NUL-terminated user string, never reading past maxLength or the process's reservations
size_t UserStringLength(const UProcess& proc, const char* const s, const size_t maxLength);

//Look up a caller's handle that must refer to an object of the given type
SyscallResult GetObject(UProcess& proc, const Handle handle, const UObjectType type, UObject*& obj);

//The kernel passes the caller's RIP, so this differs from the user API's DebugPrintStack
SyscallResult DebugPrintStack(const uint64_t rip);
