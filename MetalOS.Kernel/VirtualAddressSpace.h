#pragma once

#include "msvc.h"
#include <list>
#include <Kernel.h>

class VirtualAddressSpace
{
public:
	VirtualAddressSpace();



private:
	std::list<VAD_NODE> m_vadRoot;
};

