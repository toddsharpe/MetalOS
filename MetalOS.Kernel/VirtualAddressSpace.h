#pragma once

#include "msvc.h"
#include <list>
#include "MetalOS.Kernel.h"

class VirtualAddressSpace
{
public:
	VirtualAddressSpace();



private:
	std::list<VAD_NODE> m_vadRoot;
};

