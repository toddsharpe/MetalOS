#pragma once

#include "kernel/Arch.h"
#include "kernel/Pmem.h"
#include "MetalOS.Space.h"

struct KProcess;

//All sizes are in bytes; forms without a KProcess act on the kernel's process
void* KeVirtualAlloc(const size_t size);
void* KeVirtualAlloc(KProcess& process, const size_t size);
void* KeVirtualAlloc(KProcess& process, const void* address, const size_t size);

bool KeVirtualFree(const void* const address);
bool KeVirtualFree(KProcess& process, const void* const address);

//Map caller-supplied physical addresses
bool KeVirtualMap(KProcess& process, const void* const virtualAddr, const paddr_t physicalAddr, const size_t size);
void* KeVirtualMap(const paddr_t* addresses, const size_t size);
void* KeVirtualMap(KProcess& process, const paddr_t* addresses, const size_t size);
void* KeVirtualMap(const paddr_t address, const size_t size);
void* KeVirtualMap(KProcess& process, const paddr_t address, const size_t size);

void KeWriteProcessMemory(KProcess& process, void* const destVirtual, const void* const src, const size_t size);
void KeAliasInto(KProcess& process, const void* const source, const void* const dest, const size_t size);
void KeFreePageTables(KProcess& process);

bool KeIsValid(const void* address); //Whether the kernel's address space maps address
paddr_t ResolveImageVA(void* const address); //Physical address of static code or data in the kernel image

static constexpr Arch::PageTablesOps PtOps =
{
	.Resolve = [](const paddr_t address) -> void*
	{
		return reinterpret_cast<void*>(KernelPhysicalStart + address);
	},
	.PhyAlloc = [](paddr_t &address) -> bool
	{
		return KePhysicalTryAlloc(address);
	}
};
using KernelPageTables = Arch::PageTables<PtOps>;
