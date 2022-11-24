#include <user/Debug.h>
#include <shared/MetalOS.Types.h>
#include <user/MetalOS.h>


uint32_t __declspec(dllexport) DllMain(Handle handle, DllEntryReason reason)
{
	switch (reason)
	{
	default:
		//Do nothing
		break;
	}

	return true;
}
