#include "Kernel/Kernel.h"
#include <intrin.h>
#include "Kernel/Objects/KSemaphore.h"
#include "MetalOS.Arch.h"
#include "MetalOS.System.h"

#include "Assert.h"

typedef int semaphore_t;
typedef int spinlock_t;

ACPI_PHYSICAL_ADDRESS acpiRoot = 0;
ACPI_PHYSICAL_ADDRESS Kernel::AcpiOsGetRootPointer()
{
	if (acpiRoot == 0)
		acpiRoot = (ACPI_PHYSICAL_ADDRESS)m_configTables.GetAcpiTable();
	return acpiRoot;
}

void* Kernel::AcpiOsMapMemory(ACPI_PHYSICAL_ADDRESS PhysicalAddress, ACPI_SIZE Length)
{
	//Handle unaligned addresses
	const size_t pageOffset = PhysicalAddress & PageMask;
	const size_t pageCount = DivRoundUp(pageOffset + Length, PageSize);

	const uintptr_t physicalBase = PhysicalAddress & ~PageMask;

	PageTables tables;
	tables.OpenCurrent();
	Assert(tables.MapPages(KernelAcpiStart + physicalBase, physicalBase, pageCount, true));
	return (void*)(KernelAcpiStart + physicalBase + pageOffset);
}

void Kernel::AcpiOsVprintf(const char* Format, va_list Args)
{
	//Reduce ACPI talk for now (soon, pump to uart?)
	Printf(Format, Args);
}

UINT64 Kernel::AcpiOsGetTimer()
{
	EFI_TIME time;
	m_runtime.GetTime(&time, nullptr);
	uint64_t timer;
	uint16_t year = time.Year;
	uint8_t month = time.Month;
	uint8_t day = time.Day;
	timer = ((uint64_t)(year / 4 - year / 100 + year / 400 + 367 * month / 12 + day) +
		year * 365 - 719499);
	timer *= 24;
	timer += time.Hour;
	timer *= 60;
	timer += time.Minute;
	timer *= 60;
	timer += time.Second;
	timer *= 10000000;	//100 n intervals
	timer += time.Nanosecond / 100;
	return timer;
}

