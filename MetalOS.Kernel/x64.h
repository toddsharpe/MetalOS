#pragma once

#include "Kernel.h"

class x64
{
public:
	x64() = delete;
	static void Initialize();

private:
	KERNEL_PAGE_ALIGN static volatile UINT8 DOUBLEFAULT_STACK[IST_STACK_SIZE];
	KERNEL_PAGE_ALIGN static volatile UINT8 NMI_Stack[IST_STACK_SIZE];
	KERNEL_PAGE_ALIGN static volatile UINT8 DEBUG_STACK[IST_STACK_SIZE];
	KERNEL_PAGE_ALIGN static volatile UINT8 MCE_STACK[IST_STACK_SIZE];

	KERNEL_GLOBAL_ALIGN static volatile TASK_STATE_SEGMENT_64 TSS64;
	KERNEL_GLOBAL_ALIGN static volatile KERNEL_GDTS KernelGDT;
	KERNEL_GLOBAL_ALIGN static volatile IDT_GATE IDT[IDT_COUNT];

	//Aligned on word boundary so address load is on correct boundary
	__declspec(align(2)) static DESCRIPTOR_TABLE GDTR;
	__declspec(align(2)) static DESCRIPTOR_TABLE IDTR;
};

