#pragma once

#include "MetalOS.h"

class Runtime
{
public:
	static ThreadEnvironmentBlock* GetTEB();
	static ProcessEnvironmentBlock* GetPEB();
};

