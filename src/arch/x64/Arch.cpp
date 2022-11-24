#include <kernel/MetalOS.Arch.h>
#include <intrin.h>
#include "x64.h"
#include "Support.h"

void ArchInitialize()
{
	x64::Initialize();
}

size_t ArchStackReserve()
{
	return 32;
}

void ArchSetPagingRoot(paddr_t root)
{
	if (__readcr3() != root)
	{
		__writecr3(root);
	}
}

void ArchSetInterruptStack(void* stack)
{
	x64::SetKernelInterruptStack(stack);
}

void ArchSetKernelCpuContext(void* context)
{
	x64::SetKernelCpuContext(context);
}

void ArchSetUserCpuContext(void* context)
{
	x64::SetUserCpuContext(context);
}

void ArchPause()
{
	_pause();
}

void ArchWait()
{
	__halt();
}

uint32_t ArchReadPort(uint32_t port, uint8_t width)
{
	switch (width)
	{
	case 8:
		return __inbyte(port);
	case 16:
		return __inword(port);
	case 32:
		return __indword(port);
	default:
		return 0;
	}
}

void ArchWritePort(uint32_t port, uint32_t value, uint8_t width)
{
	switch (width)
	{
	case 8:
		return __outbyte(port, value);
	case 16:
		return __outword(port, value);
	case 32:
		return __outdword(port, value);
	}
}
