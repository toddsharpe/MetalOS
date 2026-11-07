#pragma once

#include "Lib/System.h"
#include "new.h"
#include "Assert.h"

enum class AllocType
{
	Kernel,
	Acpi,
	Malloc,
	User
};

void* KeAlloc(size_t size, AllocType type);
void KeFree(void* ptr, AllocType type);

template <typename T, typename... Args>
T* KeAlloc(AllocType type, Args&&... args)
{
	void* const mem = KeAlloc(sizeof(T), type);
	Assert(mem);
	return mem ? new (mem) T(static_cast<Args&&>(args)...) : nullptr;
}
