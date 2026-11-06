#pragma once

#include "kernel/Plat_HyperV/Types.h"
#include "kernel/Plat_HyperV/Tsc.h"
#include "kernel/Plat_HyperV/Platform.h"
#include "kernel/Plat_HyperV/Interrupts.h"
#include "kernel/Plat_HyperV/Timer.h"

namespace HyperV
{
	inline void Initialize()
	{
		Platform::Initialize();
		Interrupts::Initialize();
		Tsc::Initialize();
	}
}
