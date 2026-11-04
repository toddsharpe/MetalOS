#pragma once

#include <windows/types.h>
#include <windows/winnt.h>
#include <coreclr/win64unwind.h>
#include "Assert.h"
#include "WinPE.h"
#include "kernel/AddressSpace.h"

//https://stackoverflow.com/questions/5705650/stackwalk64-on-windows-get-symbol-name
namespace RuntimeSupport
{
	inline bool UnwindPrologue(const uintptr_t ImageBase, const uintptr_t ControlPc, const uintptr_t FrameBase, const RUNTIME_FUNCTION* FunctionEntry, PCONTEXT ContextRecord, PKNONVOLATILE_CONTEXT_POINTERS ContextPointers, const AddressSpace* const Space);
	
	//A user walk checks every read against the process's address space; a kernel walk (no space) reads unchecked
	inline bool IsReadable(const AddressSpace* const space, const uintptr_t address, const size_t length)
	{
		return !space || space->IsValidRange(reinterpret_cast<const void*>(address), length);
	}

	enum x64_NV_REG_NUM
	{
		Rax,
		Rcx,
		Rdx,
		Rbx,
		Rsp,
		Rbp,
		Rsi,
		Rdi,
		R8,
		R9,
		R10,
		R11,
		R12,
		R13,
		R14,
		R15
	};

	//The image's exception directory (its .pdata), or null when the headers can't be read or aren't a PE's
	inline const IMAGE_DATA_DIRECTORY* GetExceptionDirectory(const uintptr_t ImageBase, const AddressSpace* const Space)
	{
		const IMAGE_DOS_HEADER* dosHeader = reinterpret_cast<const IMAGE_DOS_HEADER*>(ImageBase);
		if (!IsReadable(Space, ImageBase, sizeof(IMAGE_DOS_HEADER)) || dosHeader->e_magic != IMAGE_DOS_SIGNATURE)
			return nullptr;

		const uintptr_t ntAddress = ImageBase + (uintptr_t)dosHeader->e_lfanew;
		const IMAGE_NT_HEADERS64* ntHeader = reinterpret_cast<const IMAGE_NT_HEADERS64*>(ntAddress);
		if (!IsReadable(Space, ntAddress, sizeof(IMAGE_NT_HEADERS64)) || ntHeader->Signature != IMAGE_NT_SIGNATURE)
			return nullptr;

		return &ntHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION];
	}

	inline const RUNTIME_FUNCTION* LookupFunctionEntry(const uint64_t ControlPC, const uintptr_t ImageBase, const void* HistoryTable, const AddressSpace* const Space = nullptr)
	{
		const uint64_t RVA = (ControlPC - (uint64_t)ImageBase);

		//Searched within its size, rather than up to an entry of zeroes past the end
		const IMAGE_DATA_DIRECTORY* directory = GetExceptionDirectory(ImageBase, Space);
		if (!directory || directory->VirtualAddress == 0 || !IsReadable(Space, ImageBase + directory->VirtualAddress, directory->Size))
			return nullptr;

		const RUNTIME_FUNCTION* current = reinterpret_cast<const RUNTIME_FUNCTION*>((uint64_t)ImageBase + directory->VirtualAddress);
		const size_t count = directory->Size / sizeof(RUNTIME_FUNCTION);

		//TODO: list is sorted, this can be faster
		for (size_t i = 0; i < count; i++)
		{
			if (current->BeginAddress <= RVA && RVA < current->EndAddress) //EndAddress is one past the function's last byte
				return current;

			current++;
		}
		
		return nullptr;
	}

	//Null when the unwind info, its codes (padded to an even count) or the function entry it chains to can't be read
	inline const UNWIND_INFO* GetUnwindInfo(const uintptr_t ImageBase, const RUNTIME_FUNCTION* function, const AddressSpace* const Space)
	{
		static constexpr size_t HeaderSize = sizeof(UNWIND_INFO) - sizeof(UNWIND_CODE);

		const uintptr_t address = ImageBase + function->UnwindInfoAddress;
		if (!IsReadable(Space, address, HeaderSize))
			return nullptr;

		const UNWIND_INFO* unwindInfo = reinterpret_cast<const UNWIND_INFO*>(address);
		size_t size = HeaderSize + ((unwindInfo->CountOfUnwindCodes + 1) & ~1) * sizeof(UNWIND_CODE);
		if ((unwindInfo->Flags & UNW_FLAG_CHAININFO) != 0)
			size += sizeof(RUNTIME_FUNCTION);

		return IsReadable(Space, address, size) ? unwindInfo : nullptr;
	}

	//src\coreclr\src\unwinder\amd64\unwinder_amd64.cpp
	//https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64?view=vs-2019
	//Either 1. complete epilog, 2. unwind prolog, or 3. call handler/unwind directly
	inline bool VirtualUnwind(DWORD HandlerType, ULONG64 ImageBase, ULONG64 ControlPC, const RUNTIME_FUNCTION* FunctionEntry, PCONTEXT ContextRecord, PKNONVOLATILE_CONTEXT_POINTERS ContextPointers, const AddressSpace* const Space = nullptr)
	{
		const UNWIND_INFO* unwindInfo = GetUnwindInfo(ImageBase, FunctionEntry, Space);
		if (!unwindInfo || unwindInfo->Version != 1)
			return false;
		//AssertEqual(unwindInfo->Flags, UNW_FLAG_NHANDLER);

		const uintptr_t PrologOffset = ContextRecord->Rip - (FunctionEntry->BeginAddress + ImageBase);

		uintptr_t EstablisherFrame;
		if (unwindInfo->FrameRegister == 0)
		{
			EstablisherFrame = (uintptr_t)ContextRecord->Rsp;
		}
		else 
		{
			//The walk starts from RIP, RSP and RBP only, so another frame register's value isn't known
			if (unwindInfo->FrameRegister != x64_NV_REG_NUM::Rbp)
				return false;
			EstablisherFrame = (&ContextRecord->Rax)[unwindInfo->FrameRegister];
			EstablisherFrame -= (uintptr_t)unwindInfo->FrameOffset * 16;
		}

		//TODO: detect epilog based on bytes in list: https://docs.microsoft.com/en-us/windows/win32/api/winnt/nf-winnt-rtlvirtualunwind

		return UnwindPrologue(ImageBase, ContextRecord->Rip, EstablisherFrame, FunctionEntry, ContextRecord, ContextPointers, Space);
	}

	inline bool MemoryRead64(const AddressSpace* const Space, PULONG64 addr, ULONG64& value)
	{
		if (!IsReadable(Space, (uintptr_t)addr, sizeof(ULONG64)))
			return false;

		value = *(PULONG64)((ULONG_PTR)addr);
		return true;
	}

	//Copied as two integers, so the kernel doesn't move it through an XMM register
	inline bool MemoryRead128(const AddressSpace* const Space, PM128A addr, M128A& value)
	{
		if (!IsReadable(Space, (uintptr_t)addr, sizeof(M128A)))
			return false;

		value.Low = addr->Low;
		value.High = addr->High;
		return true;
	}

	inline ULONG UnwindOpSlots(UNWIND_CODE UnwindCode)
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

	//OOPStackUnwinderAMD64::UnwindPrologue, false (instead of an assert) on a read the space doesn't hold or unwind codes it can't follow
	inline bool UnwindPrologue(const uintptr_t ImageBase, const uintptr_t ControlPc, const uintptr_t FrameBase, const RUNTIME_FUNCTION* FunctionEntry, PCONTEXT ContextRecord, PKNONVOLATILE_CONTEXT_POINTERS ContextPointers, const AddressSpace* const Space)
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
		const UNWIND_INFO* UnwindInfo;
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

			UnwindInfo = GetUnwindInfo(ImageBase, FunctionEntry, Space);
			if (!UnwindInfo)
				return false;

			while (Index < UnwindInfo->CountOfUnwindCodes) {

				//
				// If the prologue offset is greater than the next unwind code
				// offset, then simulate the effect of the unwind code.
				//

				//An unknown or spare op, or one whose extra slots run past the codes, is corrupt unwind data
				UnwindOp = UnwindInfo->UnwindCode[Index].UnwindOp;
				if (UnwindOp > UWOP_PUSH_MACHFRAME || UnwindOp == UWOP_SPARE_CODE || Index + UnwindOpSlots(UnwindInfo->UnwindCode[Index]) > UnwindInfo->CountOfUnwindCodes)
					return false;

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
						if (!MemoryRead64(Space, IntegerAddress, IntegerRegister[OpInfo]))
							return false;

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
						if (!MemoryRead64(Space, IntegerAddress, IntegerRegister[OpInfo]))
							return false;

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
						if (!MemoryRead64(Space, IntegerAddress, IntegerRegister[OpInfo]))
							return false;

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
						if (!MemoryRead128(Space, FloatingAddress, FloatingRegister[OpInfo]))
							return false;

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
						if (!MemoryRead128(Space, FloatingAddress, FloatingRegister[OpInfo]))
							return false;

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

						if (!MemoryRead64(Space, ReturnAddress, ContextRecord->Rip) || !MemoryRead64(Space, StackAddress, ContextRecord->Rsp))
							return false;

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
					if (!MemoryRead64(Space, (PULONG64)ContextRecord->Rsp, ContextRecord->Rip))
						return false;
					ContextRecord->Rsp += 8;
				}

				return true;
			}

			//
			// Limit the number of iterations possible for chained function table
			// entries.
			//

			ChainCount += 1;
			if (ChainCount > UNWIND_CHAIN_LIMIT)
				return false;

		} while (TRUE);
	}

};

