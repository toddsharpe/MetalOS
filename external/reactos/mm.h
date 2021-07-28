#pragma once

//
// MmDbgCopyMemory Flags
//
#define MMDBG_COPY_WRITE            0x00000001
#define MMDBG_COPY_PHYSICAL         0x00000002
#define MMDBG_COPY_UNSAFE           0x00000004
#define MMDBG_COPY_CACHED           0x00000008
#define MMDBG_COPY_UNCACHED         0x00000010
#define MMDBG_COPY_WRITE_COMBINED   0x00000020

//
// Maximum chunk size per copy
//
#define MMDBG_COPY_MAX_SIZE         0x8


//
// Mm copy support for Kd
//
NTSTATUS
NTAPI
MmDbgCopyMemory(
	IN ULONG64 Address,
	IN PVOID Buffer,
	IN ULONG Size,
	IN ULONG Flags
);
