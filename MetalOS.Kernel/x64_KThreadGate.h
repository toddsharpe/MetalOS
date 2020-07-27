#pragma once
#include "x64_support.h"

class x64_KThreadGate
{
public:
	x64_KThreadGate()
	{
		x64_swapgs();
	}

	~x64_KThreadGate()
	{
		x64_swapgs();
	}
};
