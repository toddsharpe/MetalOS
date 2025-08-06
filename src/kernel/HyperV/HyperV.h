#pragma once

#include "kernel/HyperV/Types.h"
#include "kernel/HyperV/Tsc.h"
#include "kernel/HyperV/Platform.h"
#include "kernel/HyperV/Interrupts.h"
#include "kernel/HyperV/Timer.h"

namespace HyperV
{
	void Initialize()
	{
		Platform::Initialize();
		Interrupts::Initialize();
		Tsc::Initialize();
	}
}
