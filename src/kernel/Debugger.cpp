#include "core_crt/stdarg.h"
#include "Assert.h"


#include <windows/types.h>
#include <reactos/ntstatus.h>
#include <windows/types.h>
#include <reactos/ioaccess.h>
#include <windows/winnt.h>
#include <reactos/ketypes.h>
#include <reactos/amd64/ketypes.h>
#include <reactos/amd64/ke.h>
#include <reactos/windbgkd.h>
#include <coreclr/list.h>
#include "kddll.h"
#include "kernel/Kd64/kd64.h"
#include "kernel/Scheduler.h"
#include "core_crt/wchar.h"
#include "core_crt/stdlib.h"
extern "C" KdDll m_dll;
#include "Debugger.h"
#include "Loader.h"
#include "WinPE.h"
#include "MetalOS.Space.h"
#include "kernel/Heap.h"
#include "kernel/Thread.h"
#include "kernel/Process.h"

uint32_t Debugger::ThreadLoop(void* arg)
{
	return static_cast<Debugger*>(arg)->ThreadLoop();
};

void Debugger::Initialize()
{
	/* Fill out the KD Version Block */
	KdVersionBlock.MajorVersion = DBGKD_MAJOR_EFI;
	KdVersionBlock.MinorVersion = 7601;
	
	/* Set protocol limits */
	KdVersionBlock.MaxStateChange = DbgKdMaximumStateChange -
		DbgKdMinimumStateChange;
	KdVersionBlock.MaxManipulate = DbgKdMaximumManipulate -
		DbgKdMinimumManipulate;
	KdVersionBlock.Unused[0] = 0;

	KdVersionBlock.KernBase = KernelBase;
	KdVersionBlock.PsLoadedModuleList = (ULONG64)(LONG_PTR)&PsLoadedModuleList;

	InitializeListHead(&PsLoadedModuleList);

	const KModule* kddll = KeLoadLibrary("kdcom.dll");

	//Load pointers
	m_dll.KdInitialize = static_cast<OnKdInitialize>(WinPE::GetProcAddress(kddll->ImageBase, "KdInitialize"));
	m_dll.KdReceivePacket = static_cast<OnKdReceivePacket>(WinPE::GetProcAddress(kddll->ImageBase, "KdReceivePacket"));
	m_dll.KdSendPacket = static_cast<OnKdSendPacket>(WinPE::GetProcAddress(kddll->ImageBase, "KdSendPacket"));
	m_dll.KdHostPresent = static_cast<OnKdHostPresent>(WinPE::GetProcAddress(kddll->ImageBase, "KdHostPresent"));

	Printf("KdInitialize loaded at 0x%016x\n", m_dll.KdInitialize);
	Printf("KdReceivePacket loaded at 0x%016x\n", m_dll.KdReceivePacket);
	Printf("KdSendPacket loaded at 0x%016x\n", m_dll.KdSendPacket);

	NTSTATUS result = m_dll.KdInitialize(2);
	Printf("KdInitialized %d\n", result);

	KeCreateThread(Debugger::ThreadLoop, this, "Debugger::ThreadLoop");

	//No host has answered yet; each send refreshes this from kdcom
	KdDebuggerEnabled = TRUE;
	KdDebuggerNotPresent = TRUE;
}

void Debugger::AddModule(const KModule& library)
{
	LDR_DATA_TABLE_ENTRY* entry = static_cast<LDR_DATA_TABLE_ENTRY*>(KeAlloc(sizeof(LDR_DATA_TABLE_ENTRY), AllocType::Kernel));
	memset(entry, 0, sizeof(LDR_DATA_TABLE_ENTRY));

	entry->DllBase = (void*)library.ImageBase;
	entry->EntryPoint = (void*)((uintptr_t)library.ImageBase + WinPE::GetEntryPoint(library.ImageBase));
	entry->SizeOfImage = WinPE::GetSizeOfImage(library.ImageBase);
	entry->LoadCount = 1;

	//Allocate name unicode strings
	size_t length = StrLength(library.Name);
	size_t wideLength = (length + 1) * 2;
	wchar_t* s = (wchar_t*)KeAlloc(sizeof(wchar_t) * wideLength, AllocType::Kernel);
	mbstowcs(s, library.Name, wideLength);
	s[length] = '\0';

	entry->FullDllName = { (USHORT)wcslen(s), 0, s };
	entry->BaseDllName = { (USHORT)wcslen(s), 0, s };

	InsertHeadList(&PsLoadedModuleList, &entry->InLoadOrderLinks);
}

void Debugger::DebuggerEvent(Arch::InterruptVector vector, Arch::InterruptFrame& frame)
{
	UNUSED(vector);
	KTRAP_FRAME TrapFrame = { 0 }; //Unused?
	KEXCEPTION_FRAME ExceptionFrame = { 0 }; //Unused?
	EXCEPTION_RECORD ExceptionRecord = { 0 };
	CONTEXT ContextRecord = { 0 };
	KPROCESSOR_MODE PreviousMode = KernelMode;
	BOOLEAN SecondChanceException = false;

	ExceptionRecord.ExceptionCode = static_cast<DWORD>(STATUS_BREAKPOINT);
	ExceptionRecord.NumberParameters = 3;
	ExceptionRecord.ExceptionInformation[0] = BREAKPOINT_BREAK;
	ExceptionRecord.ExceptionInformation[1] = (ULONG64)(LONG_PTR)&Scheduler::GetCurrentThread();
	ExceptionRecord.ExceptionInformation[2] = 0;

	ConvertToContext(&frame, &ContextRecord);

	BOOLEAN handled = KdpTrap(&TrapFrame, &ExceptionFrame, &ExceptionRecord, &ContextRecord, PreviousMode, SecondChanceException);
	Printf("Debugger Event: %d\n", handled);
}

bool Debugger::Enabled()
{
	return KdDebuggerEnabled;
}

bool Debugger::HostPresent()
{
	return !KdDebuggerNotPresent;
}

uint32_t Debugger::ThreadLoop()
{
	while (true)
	{
		if (KdPollBreakIn())
		{
			__debugbreak();
		}
		Sleep(50);
	}

	return 0;
}

void Debugger::ConvertToContext(Arch::InterruptFrame* frame, PCONTEXT context)
{
	context->SegCs = (WORD)frame->CS;
	context->SegFs = (WORD)frame->FS;
	context->SegGs = (WORD)frame->GS;
	context->SegSs = (WORD)frame->SS;
	context->EFlags = (DWORD)frame->RFlags;
	
	context->Rip = frame->RIP;

	context->Rax = frame->RAX;
	context->Rcx = frame->RCX;
	context->Rdx = frame->RDX;
	context->Rbx = frame->RBX;
	context->Rsp = frame->RSP;
	context->Rbp = frame->RBP;
	context->Rsi = frame->RSI;
	context->Rdi = frame->RDI;
	context->R8 = frame->R8;
	context->R9 = frame->R9;
	context->R10 = frame->R10;
	context->R11 = frame->R11;
	context->R12 = frame->R12;
	context->R13 = frame->R13;
	context->R14 = frame->R14;
	context->R15 = frame->R15;
}
