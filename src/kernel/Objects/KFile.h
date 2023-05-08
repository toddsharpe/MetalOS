#pragma once

#include <cstdint>
#include <shared/MetalOS.Types.h>

#include "Assert.h"

class KFile
{
public:
	void Display() const
	{
		Printf("Context: 0x%016x\n", Context);
		Printf("Position: 0x%016x\n", Position);
		Printf("Length: 0x%016x\n", Length);
		Printf("Access: %d\n", Access);
	}

	void* Context;
	size_t Position;
	size_t Length;
	GenericAccess Access;
};
