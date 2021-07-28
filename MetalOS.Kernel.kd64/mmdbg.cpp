#include <sal.h>
#include <cstdint>
#include <algorithm>
#include <ntstatus.h>
#include <windows/types.h>
#include <windows/winnt.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/windbgkd.h>
#include <reactos/mm.h>
#include <coreclr/list.h>
#include <kddll.h>
#include <reactos/amd64/ke.h>
#include "kd64.h"

#define DEFAULT_UNREACHABLE default: __assume(0)

NTSTATUS
NTAPI
MmDbgCopyMemory(IN ULONG64 Address,
	IN PVOID Buffer,
	IN ULONG Size,
	IN ULONG Flags)
{
	PVOID TargetAddress;
	ULONG64 PhysicalAddress;
	//PMMPTE PointerPte;
	PVOID CopyDestination, CopySource;

	/* No local kernel debugging support yet, so don't worry about locking */
	//ASSERT(Flags & MMDBG_COPY_UNSAFE);

	/* We only handle 1, 2, 4 and 8 byte requests */
	if ((Size != 1) &&
		(Size != 2) &&
		(Size != 4) &&
		(Size != 8))
	{
		/* Invalid size, fail */
		KdpDprintf("MmDbgCopyMemory: Received Illegal Size 0x%lx\n",
			Size);
		return STATUS_INVALID_PARAMETER_3;
	}

	/* The copy must be aligned */
	if ((Address & (Size - 1)) != 0)
	{
		/* Not allowing unaligned access */
		KdpDprintf("MmDbgCopyMemory: Received Unaligned Address 0x%I64x Size %lx\n",
			Address,
			Size);
		return STATUS_INVALID_PARAMETER_3;
	}

	/* Check for physical or virtual copy */
	//if (Flags & MMDBG_COPY_PHYSICAL)
	//{
	//	///* Physical: translate and map it to our mapping space */
	//	//TargetAddress = MiDbgTranslatePhysicalAddress(Address,
	//	//	Flags);

	//	///* Check if translation failed */
	//	//if (!TargetAddress)
	//	//{
	//	//	/* Fail */
	//	//	KdpDprintf("MmDbgCopyMemory: Failed to Translate Physical Address %I64x\n",
	//	//		Address);
	//	//	return STATUS_UNSUCCESSFUL;
	//	//}

	//	///* The address we received must be valid! */
	//	//ASSERT(MmIsAddressValid(TargetAddress));
	//}
	//else
	//{
		/* Virtual; truncate it to avoid casts later down */
		TargetAddress = (PVOID)(ULONG_PTR)Address;

		/* Make sure address is valid */
		if (false /*!MmIsAddressValid(TargetAddress)*/)
		{
			/* Fail */
			KdpDprintf("MmDbgCopyMemory: Failing %s for invalid Virtual Address 0x%p\n",
				Flags & MMDBG_COPY_WRITE ? "write" : "read",
				TargetAddress);
			return STATUS_UNSUCCESSFUL;
		}

		/* Not handling session space correctly yet */
		//if (MmIsSessionAddress(TargetAddress))
		//{
		//	/* FIXME */
		//}

		/* If we are going to write to the address, then check if its writable */
		//PointerPte = MiAddressToPte(TargetAddress);
		//if ((Flags & MMDBG_COPY_WRITE) &&
		//	(!MI_IS_PAGE_WRITEABLE(PointerPte)))
		//{
		//	/* Not writable, we need to do a physical copy */
		//	Flags |= MMDBG_COPY_PHYSICAL;

		//	/* Calculate the physical address */
		//	PhysicalAddress = PointerPte->u.Hard.PageFrameNumber << PAGE_SHIFT;
		//	PhysicalAddress += BYTE_OFFSET(Address);

		//	/* Translate the physical address */
		//	TargetAddress = MiDbgTranslatePhysicalAddress(PhysicalAddress,
		//		Flags);

		//	/* Check if translation failed */
		//	if (!TargetAddress)
		//	{
		//		/* Fail */
		//		KdpDprintf("MmDbgCopyMemory: Failed to translate for write %I64x (%I64x)\n",
		//			PhysicalAddress,
		//			Address);
		//		return STATUS_UNSUCCESSFUL;
		//	}
		//}
	//}

	/* Check what kind of operation this is */
	if (Flags & MMDBG_COPY_WRITE)
	{
		/* Write */
		CopyDestination = TargetAddress;
		CopySource = Buffer;
	}
	else
	{
		/* Read */
		CopyDestination = Buffer;
		CopySource = TargetAddress;
	}

	/* Do the copy  */
	switch (Size)
	{
	case 1:

		/* 1 byte */
		*(PUCHAR)CopyDestination = *(PUCHAR)CopySource;
		break;

	case 2:

		/* 2 bytes */
		*(PUSHORT)CopyDestination = *(PUSHORT)CopySource;
		break;

	case 4:

		/* 4 bytes */
		*(PULONG)CopyDestination = *(PULONG)CopySource;
		break;

	case 8:

		/* 8 bytes */
		*(PULONGLONG)CopyDestination = *(PULONGLONG)CopySource;
		break;

		/* Size is sanitized above */
		DEFAULT_UNREACHABLE;
	}

	/* Get rid of the mapping if this was a physical copy */
	//if (Flags & MMDBG_COPY_PHYSICAL)
	//{
	//	/* Unmap and flush it */
	//	MiDbgUnTranslatePhysicalAddress();
	//}

	/* And we are done */
	return STATUS_SUCCESS;
}
