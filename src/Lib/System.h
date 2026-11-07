#pragma once

#include <cstdint>
#include <cstring>
#include "Assert.h"

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)

#define KERNEL_PAGE_ALIGN __declspec(align(Arch::PageSize))

typedef uintptr_t paddr_t;

template<typename T>
constexpr T MakePointer(const void* base, const ptrdiff_t offset = 0)
{
	return reinterpret_cast<T>((char*)base + offset);
}

template <typename T>
constexpr T ByteAlign(const T size, const size_t alignment)
{
	const T mask = static_cast<T>(alignment - 1);
	return static_cast<T>((size + mask) & ~mask);
}

template <typename T, size_t N>
constexpr size_t ArraySize(T (&)[N]) {
	return N;
}
