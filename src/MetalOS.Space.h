#pragma once

static constexpr uintptr_t UserStart = 0x0000'0000'0000'0000;
static constexpr uintptr_t UserEnd  = 0x0000'7FFF'FFFF'FFFF;

/*
 * Kernel address space.
 * Processes have their own page tables, which means their own root page table.
 * In order to keep the kernel in each processes root, restrict to one Level-4 entry.
 */

static constexpr uintptr_t KernelStart            = 0xFFFF'8000'0000'0000;
static constexpr uintptr_t KernelBase             = 0xFFFF'8000'0100'0000;
static constexpr uintptr_t KdCom                  = 0xFFFF'8000'0200'0000;
static constexpr uintptr_t KernelPdb              = 0xFFFF'8000'1000'0000;
static constexpr uintptr_t KernelHeap             = 0xFFFF'8000'2000'0000;
static constexpr uintptr_t KernelAllocStart       = 0xFFFF'8001'0000'0000;
static constexpr uintptr_t KernelGraphicsDevice   = 0xFFFF'8002'0000'0000;
static constexpr uintptr_t KernelPageFrameDBStart = 0xFFFF'8003'0000'0000;
static constexpr uintptr_t KernelUefiStart        = 0xFFFF'8004'0000'0000;
static constexpr uintptr_t KernelPhysicalStart    = 0xFFFF'8010'0000'0000;
static constexpr uintptr_t KernelEnd              = 0xFFFF'8080'0000'0000;

//static constexpr uintptr_t KernelMin = 0xFFFF'8000'0000'0000;
//static constexpr uintptr_t KernelMax = 0xFFFF'FFFF'FFFF'FFFF;
