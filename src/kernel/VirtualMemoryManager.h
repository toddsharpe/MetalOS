#pragma once

#include "kernel/KProcess.h"

//Reserves space in calling process, acquires physical pages, maps into page tables
namespace VMM
{
	void* Allocate(KProcess& process, const size_t count);

	bool MapContiguous(KProcess& process, const void* const virtualAddr, const paddr_t physicaAddr, const size_t count);
	void* VirtualMap(KProcess& process, const paddr_t* addresses, const size_t count);
};
