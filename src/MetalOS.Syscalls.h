#pragma once

#include "core_crt/stdint.h"

//Keep in sync with src/user/MetalOS-RT/syscalls.asm; each range is one kernel/Syscalls/*.cpp
enum class Syscall : uint64_t
{
	//0x100: System
	GetTickCount = 0x100,
	GetSystemTime,

	//0x200: Processes
	CreateProcess = 0x200,
	TerminateProcess,
	ExitProcess,
	IsProcessAlive,
	GetProcesses,

	//0x300: Threads
	GetCurrentThread = 0x300,
	CreateThread,
	GetThreadId,
	Sleep,
	SwitchToThread,
	TerminateThread,
	ExitThread,

	//0x400: Files and pipes
	CreateFile = 0x400,
	ReadFile,
	WriteFile,
	SetFilePointer,
	MoveFile,
	DeleteFile,
	CreateDirectory,
	CreatePipe,
	PeekNamedPipe,
	ListDirectory,

	//0x500: Handles (closing, and passing one to another process)
	CloseHandle = 0x500,
	ShareHandle,
	ClaimHandle,

	//0x600: Synchronization
	WaitForSingleObject = 0x600,
	CreateEvent,
	SetEvent,
	ResetEvent,

	//0x700: Virtual memory
	VirtualAlloc = 0x700,
	VirtualFree,

	//0x800: Shared memory
	CreateSharedMemory = 0x800,
	MapSharedMemory,
	DeleteSharedMemory,

	//0x900: Display
	MapFramebuffer = 0x900,
	FlushFramebuffer,

	//0xA00: Network interfaces (TCP/IP is the usermode netstack)
	NetSend = 0xA00,
	GetInterfaces,

	//0xB00: IPC endpoints (name to capability token rendezvous)
	RegisterEndpoint = 0xB00,
	LookupEndpoint,
	PostEndpoint,
	PollEndpoint,

	//0xC00: Debug
	DebugPrint = 0xC00,
	DebugPrintBytes,
	DebugPrintStack,
};
