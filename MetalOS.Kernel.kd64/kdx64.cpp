/*
 * PROJECT:         ReactOS Kernel
 * LICENSE:         GPL - See COPYING in the top level directory
 * FILE:            ntoskrnl/kd64/amd64/kdx64.c
 * PURPOSE:         KD support routines for AMD64
 * PROGRAMMERS:     Timo Kreuzer (timo.kreuzer@reactos.org)
 */

 /* INCLUDES *****************************************************************/

#include <sal.h>
#include <cstdint>
#include <intrin.h>
#include <ntstatus.h>
#include <algorithm>
#include <windows/types.h>
#include <reactos/ioaccess.h>
#include <windows/winnt.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/windbgkd.h>
#include <coreclr/list.h>
#include <kddll.h>
#include <MetalOS.Arch.h>
#include <MetalOS.Internal.h>
#include <reactos/amd64/ke.h>
#include "kd64.h"
#include "MetalOSkd.h"

#undef UNIMPLEMENTED
#define UNIMPLEMENTED KdpDprintf("%s is unimplemented\n", __FUNCTION__)

#define UNIMPLEMENTED_DBGBREAK()

__inline
unsigned short
PtrToUshort(
	const void* p
)
{
	return((unsigned short)(unsigned long)(ULONG_PTR)p);
}

#define RtlEqualMemory(Destination,Source,Length) (!memcmp((Destination),(Source),(Length)))
#define RtlMoveMemory(Destination,Source,Length) memmove((Destination),(Source),(Length))
#define RtlCopyMemory(Destination,Source,Length) memcpy((Destination),(Source),(Length))
#define RtlFillMemory(Destination,Length,Fill) memset((Destination),(Fill),(Length))
#define RtlZeroMemory(Destination,Length) memset((Destination),0,(Length))

/* FUNCTIONS *****************************************************************/

VOID
NTAPI
KdpGetStateChange(IN PDBGKD_MANIPULATE_STATE64 State,
	IN PCONTEXT Context)
{
	/* Check for success */
	if (NT_SUCCESS(State->u.Continue2.ContinueStatus))
	{
		/* Check if we're tracing */
		if (State->u.Continue2.ControlSet.TraceFlag)
		{
			/* Enable TF */
			Context->EFlags |= EFLAGS_TF;
		}
		else
		{
			/* Remove it */
			Context->EFlags &= ~EFLAGS_TF;
		}

		Context->Dr7 = State->u.Continue2.ControlSet.Dr7;
		Context->Dr6 = 0;

		/* Check if we have new symbol information */
		if (State->u.Continue2.ControlSet.CurrentSymbolStart != 1)
		{
			/* Update it */
			KdpCurrentSymbolStart =
				State->u.Continue2.ControlSet.CurrentSymbolStart;
			KdpCurrentSymbolEnd = State->u.Continue2.ControlSet.CurrentSymbolEnd;
		}
	}
}

VOID
NTAPI
KdpSetContextState(IN PDBGKD_ANY_WAIT_STATE_CHANGE WaitStateChange,
	IN PCONTEXT Context)
{
	/* Copy i386 specific debug registers */
	WaitStateChange->ControlReport.Dr6 = (ULONG64)Context->Dr6;
	WaitStateChange->ControlReport.Dr7 = (ULONG64)Context->Dr7;

	/* Copy i386 specific segments */
	WaitStateChange->ControlReport.SegCs = (USHORT)Context->SegCs;
	WaitStateChange->ControlReport.SegDs = (USHORT)Context->SegDs;
	WaitStateChange->ControlReport.SegEs = (USHORT)Context->SegEs;
	WaitStateChange->ControlReport.SegFs = (USHORT)Context->SegFs;

	/* Copy EFlags */
	WaitStateChange->ControlReport.EFlags = Context->EFlags;

	/* Set Report Flags */
	WaitStateChange->ControlReport.ReportFlags = REPORT_INCLUDES_SEGS;
	if (WaitStateChange->ControlReport.SegCs == KGDT64_R0_CODE)
	{
		WaitStateChange->ControlReport.ReportFlags |= REPORT_STANDARD_CS;
	}
}

NTSTATUS
NTAPI
KdpSysReadMsr(IN ULONG Msr,
	OUT PLARGE_INTEGER MsrValue)
{
	/* Use SEH to protect from invalid MSRs */
	MsrValue->QuadPart = __readmsr(Msr);

	return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
KdpSysWriteMsr(IN ULONG Msr,
	IN PLARGE_INTEGER MsrValue)
{
	/* Use SEH to protect from invalid MSRs */
	__writemsr(Msr, MsrValue->QuadPart);
	return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
KdpSysReadBusData(IN ULONG BusDataType,
	IN ULONG BusNumber,
	IN ULONG SlotNumber,
	IN ULONG Offset,
	IN PVOID Buffer,
	IN ULONG Length,
	OUT PULONG ActualLength)
{
	UNIMPLEMENTED_DBGBREAK();
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS
NTAPI
KdpSysWriteBusData(IN ULONG BusDataType,
	IN ULONG BusNumber,
	IN ULONG SlotNumber,
	IN ULONG Offset,
	IN PVOID Buffer,
	IN ULONG Length,
	OUT PULONG ActualLength)
{
	UNIMPLEMENTED_DBGBREAK();
	return STATUS_UNSUCCESSFUL;
}

//todo - find a way to store static processor state - KPROCESSOR_STATE

NTSTATUS
NTAPI
KdpSysReadControlSpace(IN ULONG Processor,
	IN ULONG64 BaseAddress,
	IN PVOID Buffer,
	IN ULONG Length,
	OUT PULONG ActualLength)
{
	PVOID ControlStart;
	PKPRCB Prcb = &KiProcessorBlock[Processor];

	switch (BaseAddress)
	{
	//case AMD64_DEBUG_CONTROL_SPACE_KPCR:
	//	/* Copy a pointer to the Pcr */
	//	ControlStart = &Pcr;
	//	*ActualLength = sizeof(PVOID);
	//	break;

	case AMD64_DEBUG_CONTROL_SPACE_KPRCB:
		/* Copy a pointer to the Prcb */
		ControlStart = &Prcb;
		*ActualLength = sizeof(PVOID);
		break;

	case AMD64_DEBUG_CONTROL_SPACE_KSPECIAL:
		/* Copy SpecialRegisters */
		ControlStart = &Prcb->ProcessorState.SpecialRegisters;
		*ActualLength = sizeof(KSPECIAL_REGISTERS);
		break;

	//case AMD64_DEBUG_CONTROL_SPACE_KTHREAD:
	//	/* Copy a pointer to the current Thread */
	//	ControlStart = &Prcb->CurrentThread;
	//	*ActualLength = sizeof(PVOID);
	//	break;

	default:
		*ActualLength = 0;
		//ASSERT(FALSE);
		return STATUS_UNSUCCESSFUL;
	}

	/* Copy the memory */
	RtlCopyMemory(Buffer, ControlStart, std::min(Length, *ActualLength));

	/* Finish up */
	return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
KdpSysWriteControlSpace(IN ULONG Processor,
	IN ULONG64 BaseAddress,
	IN PVOID Buffer,
	IN ULONG Length,
	OUT PULONG ActualLength)
{
	PVOID ControlStart;
	PKPRCB Prcb = &KiProcessorBlock[Processor];

	switch (BaseAddress)
	{
	case AMD64_DEBUG_CONTROL_SPACE_KSPECIAL:
		/* Copy SpecialRegisters */
		ControlStart = &Prcb->ProcessorState.SpecialRegisters;
		*ActualLength = sizeof(KSPECIAL_REGISTERS);
		break;

	default:
		*ActualLength = 0;
		//ASSERT(FALSE);
		return STATUS_UNSUCCESSFUL;
	}

	/* Copy the memory */
	RtlCopyMemory(ControlStart, Buffer, std::min(Length, *ActualLength));

	return STATUS_SUCCESS;
}

#define Isa 1

NTSTATUS
NTAPI
KdpSysReadIoSpace(IN ULONG InterfaceType,
	IN ULONG BusNumber,
	IN ULONG AddressSpace,
	IN ULONG64 IoAddress,
	OUT PVOID DataValue,
	IN ULONG DataSize,
	OUT PULONG ActualDataSize)
{
	/* Verify parameters */
	if (InterfaceType != Isa || BusNumber != 0 || AddressSpace != 1)
	{
		/* No data was read */
		*ActualDataSize = 0;
		return STATUS_INVALID_PARAMETER;
	}

	/* Check for correct alignment */
	if ((IoAddress & (DataSize - 1)))
	{
		/* Invalid alignment */
		*ActualDataSize = 0;
		return STATUS_DATATYPE_MISALIGNMENT;
	}

	switch (DataSize)
	{
		case sizeof(UCHAR) :
			/* Read one UCHAR */
			*(PUCHAR)DataValue = READ_PORT_UCHAR((PUCHAR)IoAddress);
			break;

		case sizeof(USHORT) :
			/* Read one USHORT */
			*(PUSHORT)DataValue = READ_PORT_USHORT((PUSHORT)IoAddress);
			break;

		case sizeof(ULONG) :
			/* Read one ULONG */
			*(PULONG)DataValue = READ_PORT_ULONG((PULONG)IoAddress);
			break;

		default:
			/* Invalid data size */
			*ActualDataSize = 0;
			return STATUS_INVALID_PARAMETER;
	}

	/* Return the size of the data */
	*ActualDataSize = DataSize;

	/* Success! */
	return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
KdpSysWriteIoSpace(IN ULONG InterfaceType,
	IN ULONG BusNumber,
	IN ULONG AddressSpace,
	IN ULONG64 IoAddress,
	IN PVOID DataValue,
	IN ULONG DataSize,
	OUT PULONG ActualDataSize)
{
	/* Verify parameters */
	if (InterfaceType != Isa || BusNumber != 0 || AddressSpace != 1)
	{
		/* No data was written */
		*ActualDataSize = 0;
		return STATUS_INVALID_PARAMETER;
	}

	/* Check for correct alignment */
	if ((IoAddress & (DataSize - 1)))
	{
		/* Invalid alignment */
		*ActualDataSize = 0;
		return STATUS_DATATYPE_MISALIGNMENT;
	}

	switch (DataSize)
	{
		case sizeof(UCHAR) :
			/* Write one UCHAR */
			WRITE_PORT_UCHAR((PUCHAR)IoAddress, *(PUCHAR)DataValue);
			break;

		case sizeof(USHORT) :
			/* Write one USHORT */
			WRITE_PORT_USHORT((PUSHORT)IoAddress, *(PUSHORT)DataValue);
			break;

		case sizeof(ULONG) :
			/* Write one ULONG */
			WRITE_PORT_ULONG((PULONG)IoAddress, *(PULONG)DataValue);
			break;

		default:
			/* Invalid data size */
			*ActualDataSize = 0;
			return STATUS_INVALID_PARAMETER;
	}

	/* Return the size of the data */
	*ActualDataSize = DataSize;

	/* Success! */
	return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
KdpSysCheckLowMemory(IN ULONG Flags)
{
	UNIMPLEMENTED_DBGBREAK();
	return STATUS_UNSUCCESSFUL;
}

NTSTATUS
NTAPI
KdpAllowDisable(VOID)
{
	UNIMPLEMENTED_DBGBREAK();
	return STATUS_ACCESS_DENIED;
}

/* EOF */
