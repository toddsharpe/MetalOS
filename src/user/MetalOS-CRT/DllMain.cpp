#include "user/MetalOS-CRT/CRT.h"
#include "user/MetalOS.h"

bool DllMain(HModule handle, DllReason reason)
{
	UNUSED(handle);
	switch (reason)
	{
		case DllReason::ProcessAttach:
			if (!CrtOpenHeap())
				return false;
			break;
	}

	return true;
}
