#include "Main.h"

#include "msvc.h"
#include <efi.h>
#include <LoaderParams.h>
#include <crt_stdio.h>
#include <vector>
#include <intrin.h>
#include "MetalOS.Kernel.h"
#include "MetalOS.h"
#include "System.h"
#include "MemoryMap.h"
#include "PageTables.h"
#include "PageTablesPool.h"
#include "x64.h"
#include "KernelHeap.h"
#include "Bitvector.h"
#include "PageFrameAllocator.h"

extern "C"
{
#include <acpi.h>
#include "UartDriver.h"
}

//Kernel members
PageFrameAllocator* frameAllocator;

Kernel kernel;

//Kernel Stack
KERNEL_PAGE_ALIGN volatile UINT8 KERNEL_STACK[KERNEL_STACK_SIZE] = { 0 };
extern "C" UINT64 KERNEL_STACK_STOP = (UINT64)&KERNEL_STACK[KERNEL_STACK_SIZE];

//Kernel Heap
KERNEL_PAGE_ALIGN static volatile UINT8 KERNEL_HEAP[KERNEL_HEAP_SIZE] = { 0 };
KERNEL_GLOBAL_ALIGN static KernelHeap heap((UINT64)KERNEL_HEAP, KERNEL_HEAP_SIZE);

extern "C" void INTERRUPT_HANDLER(size_t vector, PINTERRUPT_FRAME pFrame)
{
	kernel.HandleInterrupt(vector, pFrame);
}

extern "C" void Print(const char* format, ...)
{
	va_list args;

	va_start(args, format);
	kernel.Print(format, args);
	va_end(args);
}

extern "C" void VPrint(const char* format, va_list args)
{
	kernel.Print(format, args);
}

void* operator new(size_t n)
{
	uintptr_t p = heap.Allocate(n);
	//Print("Allocation 0x%016x (0x%x)", p, n);
	return (void*)p;
}

void operator delete(void* p)
{
	//Print("Delete at 0x%16x", p);
	heap.Deallocate((UINT64)p);
}

void operator delete(void* p, size_t n)
{
	//Print("Delete at 0x%16x 0x%x", p, n);
	heap.Deallocate((UINT64)p);
}

extern "C" void syscall()
{
	Print("Syscall!");
}

//Need to get virtual pointers to acpi struct
//Find way to keep RuntimeServicesData/Code in kernel address space

void main(LOADER_PARAMS* loader)
{
	//Initialize platform
	x64::Initialize();
	
	//Initialize kernel
	kernel.Initialize(loader);

	//Initialize Frame Allocator
	//frameAllocator = new PageFrameAllocator(*memoryMap);

	//ACPI - ACPI CA Page 41
	

	//System system(loader->ConfigTables, loader->ConfigTableSizes);
	//system.GetInstalledSystemRam();
	//system.DisplayTableIds();
	//system.DisplayAcpi2();

	__halt();
}

ACPI_STATUS DisplayResource(ACPI_RESOURCE* Resource, void* Context);

//http://quest.bu.edu/qxr/source/kernel/smp/acpi.c#0433
ACPI_STATUS PrintAcpiDevice(ACPI_HANDLE Object, UINT32 NestingLevel, void* Context, void** ReturnValue)
{
	ACPI_STATUS Status;
	char Buffer[1024];
	memset(Buffer, 0, sizeof(Buffer));
	ACPI_BUFFER Path = { sizeof(Buffer), &Buffer };

	Status = AcpiGetName(Object, ACPI_FULL_PATHNAME, &Path);
	if (ACPI_SUCCESS(Status))
		Print("%-16s: ", Path.Pointer);
	else
		Print("<Name>: ");

	ACPI_DEVICE_INFO* Info;
	Status = AcpiGetObjectInfo(Object, &Info);
	if (ACPI_SUCCESS(Status))
	{
		Print("F: 0x%02x V: 0x%04x T: 0x%08x\n", Info->Flags, Info->Valid, Info->Type);

		//if (Info->Valid & ACPI_VALID_ADR)
			//Print("    ADR: 0x%x\n", Info->Address);

		//if (Info->Valid & ACPI_VALID_HID)
			//Print("    HID: %s\n", Info->HardwareId.String);

		//if (Info->Valid & ACPI_VALID_UID)
			//Print("    UID: %s\n", Info->UniqueId.String);

		if (Info->Valid & ACPI_VALID_CID)
			for (size_t i = 0; i < Info->CompatibleIdList.Count; i++)
			{
				Print("    CID: %s\n", Info->CompatibleIdList.Ids[i].String);
			}
	}

	//DDN
	{
		ACPI_BUFFER result = { ACPI_ALLOCATE_BUFFER, nullptr };
		Status = AcpiEvaluateObjectTyped(Object, ACPI_STRING("_DDN"), nullptr, &result, ACPI_TYPE_STRING);
		if (ACPI_SUCCESS(Status))
		{
			ACPI_OBJECT* object = (ACPI_OBJECT*)result.Pointer;
			Assert(object);
			Print("    DDN: %s\n", object->String.Pointer);
			delete result.Pointer;
		}
		else
		{
			Print("    DDN: %s\n", AcpiFormatException(Status));
		}
	}

	//Resources
	//AcpiWalkResources(Object, ACPI_STRING("_CRS"), DisplayResource, NULL);

	return AE_OK;
}
//
//ACPI_STATUS DisplayResource(ACPI_RESOURCE* Resource, void* Context)
//{
//	int i;
//	switch (Resource->Type) {
//	case ACPI_RESOURCE_TYPE_IRQ:
//		Print("    IRQ: dlen=%d trig=%d pol=%d shar=%d intc=%d cnt=%d int=",
//			Resource->Data.Irq.DescriptorLength,
//			Resource->Data.Irq.Triggering,
//			Resource->Data.Irq.Polarity,
//			Resource->Data.Irq.Shareable,
//			Resource->Data.Irq.WakeCapable,
//			Resource->Data.Irq.InterruptCount);
//		for (i = 0; i < Resource->Data.Irq.InterruptCount; i++)
//			Print("%.02X ", Resource->Data.Irq.Interrupts[i]);
//		Print("\n");
//		break;
//	case ACPI_RESOURCE_TYPE_IO:
//	{
//		Print("     IO: Decode=0x%x Align=0x%x AddrLen=%d Min=0x%.04X Max=0x%.04X\n",
//			Resource->Data.Io.IoDecode,
//			Resource->Data.Io.Alignment,
//			Resource->Data.Io.AddressLength,
//			Resource->Data.Io.Minimum,
//			Resource->Data.Io.Maximum);
//
//		UartDriver driver(Resource->Data.Io.Minimum);
//		driver.Write("Testing my driver can send a long ass fucking string\n");
//	}
//		break;
//	case ACPI_RESOURCE_TYPE_END_TAG:
//		Print("    END:\n");
//		break;
//	case ACPI_RESOURCE_TYPE_ADDRESS16:
//		Print("    A16: 0x%04X-0x%04X, Gran=0x%04X, Offset=0x%04X\n",
//			Resource->Data.Address16.Address.Minimum,
//			Resource->Data.Address16.Address.Maximum,
//			Resource->Data.Address16.Address.Granularity,
//			Resource->Data.Address16.Address.TranslationOffset);
//		break;
//	case ACPI_RESOURCE_TYPE_ADDRESS32:
//		//Print("    A32: 0x%08X-0x%08X, Gran=0x%08X, Offset=0x%08X\n",
//		//	Resource->Data.Address32.Address.Minimum,
//		//	Resource->Data.Address32.Address.Maximum,
//		//	Resource->Data.Address32.Address.Granularity,
//		//	Resource->Data.Address32.Address.TranslationOffset);
//		//Print("       : T: %d, PC: %d, Decode=0x%x, Min=0x%02X, Max=0x%02X\n",
//		//	Resource->Data.Address32.ResourceType,
//		//	Resource->Data.Address32.ProducerConsumer,
//		//	Resource->Data.Address32.Decode,
//		//	Resource->Data.Address32.MinAddressFixed,
//		//	Resource->Data.Address32.MaxAddressFixed);
//		break;
//	case ACPI_RESOURCE_TYPE_ADDRESS64:
//		//Print("    A64: 0x%016X-0x%016X, Gran=0x%016X, Offset=0x%016X\n",
//		//	Resource->Data.Address64.Address.Minimum,
//		//	Resource->Data.Address64.Address.Maximum,
//		//	Resource->Data.Address64.Address.Granularity,
//		//	Resource->Data.Address64.Address.TranslationOffset);
//		//Print("       : T: %d, PC: %d, Decode=0x%x, Min=0x%02X, Max=0x%02X\n",
//		//	Resource->Data.Address64.ResourceType,
//		//	Resource->Data.Address64.ProducerConsumer,
//		//	Resource->Data.Address64.Decode,
//		//	Resource->Data.Address64.MinAddressFixed,
//		//	Resource->Data.Address64.MaxAddressFixed);
//		break;
//	case ACPI_RESOURCE_TYPE_EXTENDED_IRQ:
//		//Print("   EIRQ: PC=%d Trig=%d Pol=%d Share=%d Wake=%d Cnt=%d Int=",
//		//	Resource->Data.ExtendedIrq.ProducerConsumer,
//		//	Resource->Data.ExtendedIrq.Triggering,
//		//	Resource->Data.ExtendedIrq.Polarity,
//		//	Resource->Data.ExtendedIrq.Shareable,
//		//	Resource->Data.ExtendedIrq.WakeCapable,
//		//	Resource->Data.ExtendedIrq.InterruptCount);
//		//for (i = 0; i < Resource->Data.Irq.InterruptCount; i++)
//		//	Print("%.02X ", Resource->Data.Irq.Interrupts[i]);
//		//Print("\n");
//		break;
//	case ACPI_RESOURCE_TYPE_FIXED_MEMORY32:
//		//Print("    M32: Addr=0x%016x, Len=0x%x, WP=%d\n",
//		//	Resource->Data.FixedMemory32.Address,
//		//	Resource->Data.FixedMemory32.AddressLength,
//		//	Resource->Data.FixedMemory32.WriteProtect);
//		break;
//	default:
//		Print("    Unknown: Type=%d\n", Resource->Type);
//		break;
//
//	}
//	return AE_OK;
//}

