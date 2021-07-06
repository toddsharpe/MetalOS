#include "RuntimeSupport.h"

#include "Main.h"
#include "PortableExecutable.h"

//TODO: History Table?
PRUNTIME_FUNCTION RuntimeSupport::LookupFunctionEntry(const uint64_t ControlPC, const uintptr_t ImageBase, const void* HistoryTable)
{
	const uint64_t RVA = (ControlPC - (uint64_t)ImageBase);
	
	PIMAGE_SECTION_HEADER pdataHeader = PortableExecutable::GetPESection(".pdata", ImageBase);
	Assert(pdataHeader);

	PRUNTIME_FUNCTION current = (PRUNTIME_FUNCTION)((uint64_t)ImageBase + pdataHeader->VirtualAddress);
	while (current != nullptr && !(current->BeginAddress <= RVA && current->EndAddress >= RVA))
		current++;

	return current;
}

PUNWIND_INFO RuntimeSupport::GetUnwindInfo(const uintptr_t address)
{
	return (PUNWIND_INFO)(address);
}

PUNWIND_INFO RuntimeSupport::GetUnwindInfo(const uintptr_t ImageBase, const PRUNTIME_FUNCTION function)
{
	return (PUNWIND_INFO)(ImageBase + function->UnwindInfoAddress);
}

//src\coreclr\src\unwinder\amd64\unwinder_amd64.cpp
//https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64?view=vs-2019
//Either 1. complete epilog, 2. unwind prolog, or 3. call handler/unwind directly
void RuntimeSupport::VirtualUnwind(UnwindHandlerType HandlerType, uintptr_t ImageBase, uintptr_t ControlPC, PRUNTIME_FUNCTION FunctionEntry, PCONTEXT ContextRecord, PKNONVOLATILE_CONTEXT_POINTERS ContextPointers)
{
	PUNWIND_INFO unwindInfo = GetUnwindInfo(ImageBase, FunctionEntry);
	Assert(unwindInfo->Version == 1);
	Assert(unwindInfo->Flags == UnwindHandlerType::UNW_FLAG_NHANDLER);
	//Print("Unknown FrameRegister: 0x%x\n", unwindInfo->FrameRegister);
	Assert(unwindInfo->FrameRegister == x64_NV_REG_NUM::Rbp);

	const uintptr_t PrologOffset = ContextRecord->Rip - (FunctionEntry->BeginAddress + ImageBase);
	Assert(PrologOffset > unwindInfo->SizeOfProlog);

	//TODO: detect epilog based on bytes in list: https://docs.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-rtlvirtualunwind
	uintptr_t EstablisherFrame = (&ContextRecord->Rax)[unwindInfo->FrameRegister];
	EstablisherFrame -= (uintptr_t)unwindInfo->FrameOffset * 16;

	UnwindPrologue(ImageBase, ContextRecord->Rip, EstablisherFrame, FunctionEntry, ContextRecord, ContextPointers);
}

static ULONG64 MemoryRead64(PULONG64 addr)
{
	return *(PULONG64)((ULONG_PTR)addr);
}

static M128A MemoryRead128(PM128A addr)
{
	return *(PM128A)((ULONG_PTR)addr);
}

static const UCHAR UnwindOpExtraSlotTable[] = {
	0,          // UWOP_PUSH_NONVOL
	1,          // UWOP_ALLOC_LARGE (or 3, special cased in lookup code)
	0,          // UWOP_ALLOC_SMALL
	0,          // UWOP_SET_FPREG
	1,          // UWOP_SAVE_NONVOL
	2,          // UWOP_SAVE_NONVOL_FAR
	1,          // UWOP_EPILOG
	2,          // UWOP_SPARE_CODE      // previously 64-bit UWOP_SAVE_XMM_FAR
	1,          // UWOP_SAVE_XMM128
	2,          // UWOP_SAVE_XMM128_FAR
	0,          // UWOP_PUSH_MACHFRAME
};

ULONG UnwindOpSlots(__in UNWIND_CODE UnwindCode)
/*++

Routine Description:

	This routine determines the number of unwind code slots ultimately
	consumed by an unwind code sequence.

Arguments:

	UnwindCode - Supplies the first unwind code in the sequence.

Return Value:

	Returns the total count of the number of slots consumed by the unwind
	code sequence.

--*/
{

	ULONG Slots;
	ULONG UnwindOp;

	//
	// UWOP_SPARE_CODE may be found in very old x64 images.
	//

	UnwindOp = UnwindCode.UnwindOp;

	Assert(UnwindOp != UWOP_SPARE_CODE);
	Assert(UnwindOp < sizeof(UnwindOpExtraSlotTable));

	Slots = UnwindOpExtraSlotTable[UnwindOp] + 1;
	if ((UnwindOp == UWOP_ALLOC_LARGE) && (UnwindCode.OpInfo != 0)) {
		Slots += 1;
	}

	return Slots;
}

#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )

#define UNWIND_CHAIN_LIMIT 32

//OOPStackUnwinderAMD64::UnwindPrologue
void RuntimeSupport::UnwindPrologue(const uintptr_t ImageBase, const uintptr_t ControlPc, const uintptr_t FrameBase, PRUNTIME_FUNCTION FunctionEntry, PCONTEXT ContextRecord, PKNONVOLATILE_CONTEXT_POINTERS ContextPointers)
{
	ULONG ChainCount;
	PM128A FloatingAddress;
	PM128A FloatingRegister;
	ULONG FrameOffset;
	ULONG Index;
	PULONG64 IntegerAddress;
	PULONG64 IntegerRegister;
	BOOLEAN MachineFrame;
	ULONG OpInfo;
	ULONG PrologOffset;
	PULONG64 ReturnAddress;
	PULONG64 StackAddress;
	PUNWIND_INFO UnwindInfo;
	ULONG UnwindOp;

	//
	// Process the unwind codes for the specified function entry and all its
	// descendent chained function entries.
	//

	ChainCount = 0;
	FloatingRegister = &ContextRecord->Xmm0;
	IntegerRegister = &ContextRecord->Rax;
	do {
		Index = 0;
		MachineFrame = FALSE;
		PrologOffset = (ULONG)(ControlPc - (FunctionEntry->BeginAddress + ImageBase));

		UnwindInfo = GetUnwindInfo(ImageBase + FunctionEntry->UnwindInfoAddress);
		Assert(UnwindInfo);

		while (Index < UnwindInfo->CountOfUnwindCodes) {

			//
			// If the prologue offset is greater than the next unwind code
			// offset, then simulate the effect of the unwind code.
			//

			UnwindOp = UnwindInfo->UnwindCode[Index].UnwindOp;
			Assert(UnwindOp <= UWOP_PUSH_MACHFRAME);

			OpInfo = UnwindInfo->UnwindCode[Index].OpInfo;
			if (PrologOffset >= UnwindInfo->UnwindCode[Index].CodeOffset) {
				switch (UnwindOp) {

					//
					// Push nonvolatile integer register.
					//
					// The operation information is the register number of
					// the register than was pushed.
					//

				case UWOP_PUSH_NONVOL:
					IntegerAddress = (PULONG64)ContextRecord->Rsp;
					IntegerRegister[OpInfo] = MemoryRead64(IntegerAddress);

					if (ARGUMENT_PRESENT(ContextPointers)) {
						ContextPointers->IntegerContext[OpInfo] = IntegerAddress;
					}

					ContextRecord->Rsp += 8;
					break;

					//
					// Allocate a large sized area on the stack.
					//
					// The operation information determines if the size is
					// 16- or 32-bits.
					//

				case UWOP_ALLOC_LARGE:
					Index += 1;
					FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset;
					if (OpInfo != 0) {
						Index += 1;
						FrameOffset += (UnwindInfo->UnwindCode[Index].FrameOffset << 16);

					}
					else {
						// The 16-bit form is scaled.
						FrameOffset *= 8;
					}

					ContextRecord->Rsp += FrameOffset;
					break;

					//
					// Allocate a small sized area on the stack.
					//
					// The operation information is the size of the unscaled
					// allocation size (8 is the scale factor) minus 8.
					//

				case UWOP_ALLOC_SMALL:
					ContextRecord->Rsp += (OpInfo * 8) + 8;
					break;

					//
					// Establish the the frame pointer register.
					//
					// The operation information is not used.
					//

				case UWOP_SET_FPREG:
					ContextRecord->Rsp = IntegerRegister[UnwindInfo->FrameRegister];
					ContextRecord->Rsp -= UnwindInfo->FrameOffset * 16;
					break;

#ifdef TARGET_UNIX

					//
					// Establish the the frame pointer register using a large size displacement.
					// UNWIND_INFO.FrameOffset must be 15 (the maximum value, corresponding to a scaled
					// offset of 15 * 16 == 240). The next two codes contain a 32-bit offset, which
					// is also scaled by 16, since the stack must remain 16-bit aligned.
					//

				case UWOP_SET_FPREG_LARGE:
					UNWINDER_ASSERT(UnwindInfo->FrameOffset == 15);
					Index += 2;
					FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
					FrameOffset += UnwindInfo->UnwindCode[Index].FrameOffset << 16;
					UNWINDER_ASSERT((FrameOffset & 0xF0000000) == 0);
					ContextRecord->Rsp = IntegerRegister[UnwindInfo->FrameRegister];
					ContextRecord->Rsp -= FrameOffset * 16;
					break;

#endif // TARGET_UNIX

					//
					// Save nonvolatile integer register on the stack using a
					// 16-bit displacment.
					//
					// The operation information is the register number.
					//

				case UWOP_SAVE_NONVOL:
					Index += 1;
					FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset * 8;
					IntegerAddress = (PULONG64)(FrameBase + FrameOffset);
					IntegerRegister[OpInfo] = MemoryRead64(IntegerAddress);

					if (ARGUMENT_PRESENT(ContextPointers)) {
						ContextPointers->IntegerContext[OpInfo] = IntegerAddress;
					}

					break;

					//
					// Save nonvolatile integer register on the stack using a
					// 32-bit displacment.
					//
					// The operation information is the register number.
					//

				case UWOP_SAVE_NONVOL_FAR:
					Index += 2;
					FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
					FrameOffset += UnwindInfo->UnwindCode[Index].FrameOffset << 16;
					IntegerAddress = (PULONG64)(FrameBase + FrameOffset);
					IntegerRegister[OpInfo] = MemoryRead64(IntegerAddress);

					if (ARGUMENT_PRESENT(ContextPointers)) {
						ContextPointers->IntegerContext[OpInfo] = IntegerAddress;
					}

					break;

					//
					// Function epilog marker (ignored for prologue unwind).
					//

				case UWOP_EPILOG:
					Index += 1;
					break;

					//
					// Spare unused codes.
					//


				case UWOP_SPARE_CODE:
					Index += 2;
					break;

					//
					// Save a nonvolatile XMM(128) register on the stack using a
					// 16-bit displacement.
					//
					// The operation information is the register number.
					//

				case UWOP_SAVE_XMM128:
					Index += 1;
					FrameOffset = UnwindInfo->UnwindCode[Index].FrameOffset * 16;
					FloatingAddress = (PM128A)(FrameBase + FrameOffset);
					FloatingRegister[OpInfo] = MemoryRead128(FloatingAddress);

					if (ARGUMENT_PRESENT(ContextPointers)) {
						ContextPointers->FloatingContext[OpInfo] = FloatingAddress;
					}

					break;

					//
					// Save a nonvolatile XMM(128) register on the stack using
					// a 32-bit displacement.
					//
					// The operation information is the register number.
					//

				case UWOP_SAVE_XMM128_FAR:
					Index += 2;
					FrameOffset = UnwindInfo->UnwindCode[Index - 1].FrameOffset;
					FrameOffset += UnwindInfo->UnwindCode[Index].FrameOffset << 16;
					FloatingAddress = (PM128A)(FrameBase + FrameOffset);
					FloatingRegister[OpInfo] = MemoryRead128(FloatingAddress);

					if (ARGUMENT_PRESENT(ContextPointers)) {
						ContextPointers->FloatingContext[OpInfo] = FloatingAddress;
					}

					break;

					//
					// Push a machine frame on the stack.
					//
					// The operation information determines whether the
					// machine frame contains an error code or not.
					//

				case UWOP_PUSH_MACHFRAME:
					MachineFrame = TRUE;
					ReturnAddress = (PULONG64)ContextRecord->Rsp;
					StackAddress = (PULONG64)(ContextRecord->Rsp + (3 * 8));
					if (OpInfo != 0) {
						ReturnAddress += 1;
						StackAddress += 1;
					}

					ContextRecord->Rip = MemoryRead64(ReturnAddress);
					ContextRecord->Rsp = MemoryRead64(StackAddress);

					break;

					//
					// Unused codes.
					//

				default:
					//RtlRaiseStatus(STATUS_BAD_FUNCTION_TABLE);
					break;
				}

				Index += 1;

			}
			else {

				//
				// Skip this unwind operation by advancing the slot index
				// by the number of slots consumed by this operation.
				//

				Index += UnwindOpSlots(UnwindInfo->UnwindCode[Index]);
			}
		}

		//
		// If chained unwind information is specified, then set the function
		// entry address to the chained function entry and continue the scan.
		// Otherwise, determine the return address if a machine frame was not
		// encountered during the scan of the unwind codes and terminate the
		// scan.
		//

		if ((UnwindInfo->Flags & UNW_FLAG_CHAININFO) != 0) {

			Index = UnwindInfo->CountOfUnwindCodes;
			if ((Index & 1) != 0) {
				Index += 1;
			}

			// GetUnwindInfo looks for CHAININFO and reads
			// the trailing RUNTIME_FUNCTION so we can just
			// directly use the data sitting in UnwindInfo.
			FunctionEntry = (_PIMAGE_RUNTIME_FUNCTION_ENTRY)
				& UnwindInfo->UnwindCode[Index];
		}
		else {

			if (MachineFrame == FALSE) {
				ContextRecord->Rip = MemoryRead64((PULONG64)ContextRecord->Rsp);
				ContextRecord->Rsp += 8;
			}

			break;
		}

		//
		// Limit the number of iterations possible for chained function table
		// entries.
		//

		ChainCount += 1;
		Assert(ChainCount <= UNWIND_CHAIN_LIMIT);

	} while (TRUE);
}
