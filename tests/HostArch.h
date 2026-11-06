#pragma once

//Stands in for the kernel's Arch.h when kernel code is built into the host tests
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "Lib/System.h"
#include "Assert.h"

namespace Arch
{
	static constexpr size_t PageShift = 12;
	static constexpr size_t PageSize  = (1 << PageShift);
	static constexpr size_t PageMask  = PageSize - 1;
}
