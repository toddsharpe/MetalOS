#pragma once

//Keep in sync with MetalOS.KernalApi syscalls.asm
enum class Syscall : uint64_t
{
	//0x100: System
	GetTickCount = 0x100,
	GetSystemTime,

	//0x200: Threads/Processes
	GetCurrentThread = 0x200,
	CreateProcess,
	CreateThread,
	GetThreadId,
	Sleep,
	SwitchToThread,
	SuspendThread,
	ResumeThread,
	TerminateProcess,
	ExitProcess,
	TerminateThread,
	ExitThread,

	//0x300: Windowing
	AllocWindow = 0x300,
	PaintWindow,
	MoveWindow,
	GetWindowRect,
	GetMessage,
	PeekMessage,
	GetScreenRect,

	//0x400: Files/pipes
	CreateFile = 0x400,
	ReadFile,
	WriteFile,
	SetFilePointer,
	MoveFile,
	DeleteFile,
	CreateDirectory,
	CreatePipe,
	PeekNamedPipe,
	CloseHandle,

	//0x500: Syncronization
	WaitForSingleObject = 0x500,
	CreateEvent,
	SetEvent,
	ResetEvent,

	//0x600: Memory
	VirtualAlloc = 0x600,
	//CreateRingBuffer,
	//CreateSharedMemory,
	//MapObject,
	//MapSharedObject,

	//0x700: Debug
	DebugPrint = 0x700,
	DebugPrintBytes,
	DebugPrintStack,
};
