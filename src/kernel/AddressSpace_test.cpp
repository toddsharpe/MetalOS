#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#include "Lib/System.h"
#include "Assert.h"

namespace Arch
{
	static constexpr size_t PageShift = 12;
	static constexpr size_t PageSize  = (1 << PageShift);
	static constexpr size_t PageMask  = PageSize - 1;
}

static constexpr size_t AllocationGranularity = 0x10000; // 64K

void CPrintf(const bool, const char*, ...);

#include "kernel/AddressSpace.h"
#include "kernel/AddressSpace.cpp"

// Small address range to keep tests simple
static constexpr uintptr_t TestStart = 0x10000;  // 64K
static constexpr uintptr_t TestEnd   = 0x100000; // 1MB

void AddressSpace_test()
{
    // Sequential auto-allocation — watermark advances by AllocationGranularity
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        const uintptr_t a = space.Reserve(1);
        AssertOp(a, !=, (uintptr_t)0);
        AssertEqual(a, TestStart + AllocationGranularity);

        const uintptr_t b = space.Reserve(1);
        AssertOp(b, !=, (uintptr_t)0);
        Assert(b == TestStart + 2 * AllocationGranularity);
    }

    // Fixed-address allocation
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        Assert(space.Reserve((uintptr_t)0x20000, 4));
    }

    // IsValidPointer — all pages of a multi-page reservation are valid
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        const uintptr_t addr = space.Reserve(3);
        AssertOp(addr, !=, (uintptr_t)0);
        Assert(space.IsValidPointer((void*)addr));
        Assert(space.IsValidPointer((void*)(addr + Arch::PageSize)));
        Assert(space.IsValidPointer((void*)(addr + 2 * Arch::PageSize)));
    }

    // IsValidPointer — unreserved page within range is not valid
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        Assert(!space.IsValidPointer((void*)0x50000));
    }

    // IsValidPointer — address below Start is not valid (not a kernel pointer)
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        Assert(!space.IsValidPointer((void*)0x0));
        Assert(!space.IsValidPointer((void*)0xf78));
        Assert(!space.IsValidPointer((void*)(TestStart - Arch::PageSize)));
    }

    // IsValidPointer — address at or above End is not valid
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        Assert(!space.IsValidPointer((void*)TestEnd));
        Assert(!space.IsValidPointer((void*)(TestEnd + Arch::PageSize)));
    }

    // Adjacent reservations do not conflict
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        Assert(space.Reserve((uintptr_t)0x20000, 1)); // [0x20000, 0x21000)
        Assert(space.Reserve((uintptr_t)0x21000, 1)); // [0x21000, 0x22000) — adjacent, not overlapping
    }

    // Same address reserved twice fails
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        Assert(space.Reserve((uintptr_t)0x30000, 2));
        Assert(!space.Reserve((uintptr_t)0x30000, 2));
    }

    // Partial overlap — new range starts inside existing reservation
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        Assert(space.Reserve((uintptr_t)0x40000, 4)); // [0x40000, 0x44000)
        Assert(!space.Reserve((uintptr_t)0x42000, 4)); // [0x42000, 0x46000) — starts inside
    }

    // Partial overlap — new range ends inside existing reservation
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        Assert(space.Reserve((uintptr_t)0x50000, 4)); // [0x50000, 0x54000)
        Assert(!space.Reserve((uintptr_t)0x4E000, 4)); // [0x4E000, 0x52000) — ends inside
    }

    // Containment — new range completely contains existing reservation (Bug 1 fix)
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        Assert(space.Reserve((uintptr_t)0x63000, 1)); // [0x63000, 0x64000)
        Assert(!space.Reserve((uintptr_t)0x60000, 8)); // [0x60000, 0x68000) — wraps the inner
    }

    // End-of-space boundary — range whose High == End is accepted (Bug 2 fix)
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        Assert(space.Reserve(TestEnd - Arch::PageSize, 1)); // [0xFF000, 0x100000) — High == TestEnd
    }

    // End-of-space boundary — range extending one page past End is rejected (Bug 2 fix)
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        Assert(!space.Reserve(TestEnd - Arch::PageSize, 2)); // [0xFF000, 0x101000) — High > TestEnd
    }

    // Free — reserved region can be freed and IsValidPointer returns false after
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        const uintptr_t addr = space.Reserve(2);
        AssertOp(addr, !=, (uintptr_t)0);
        Assert(space.IsValidPointer((void*)addr));
        Assert(space.Free(addr, 2));
        Assert(!space.IsValidPointer((void*)addr));
    }

    // Free — freed region can be reserved again
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        Assert(space.Reserve((uintptr_t)0x20000, 4));
        Assert(space.Free((uintptr_t)0x20000, 4));
        Assert(space.Reserve((uintptr_t)0x20000, 4));
    }

    // Free — freeing an unreserved address returns false
    {
        AddressSpace space(TestStart, TestEnd, AllocationGranularity);

        Assert(!space.Free((uintptr_t)0x20000, 1));
    }
}
