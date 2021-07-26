/******************************************************************************
 *                           Runtime Library Types                            *
 ******************************************************************************/

#pragma once

 /* Exception record flags */
#define EXCEPTION_NONCONTINUABLE  0x01
#define EXCEPTION_UNWINDING       0x02
#define EXCEPTION_EXIT_UNWIND     0x04
#define EXCEPTION_STACK_INVALID   0x08
#define EXCEPTION_NESTED_CALL     0x10
#define EXCEPTION_TARGET_UNWIND   0x20
#define EXCEPTION_COLLIDED_UNWIND 0x40
#define EXCEPTION_UNWIND (EXCEPTION_UNWINDING | EXCEPTION_EXIT_UNWIND | \
                          EXCEPTION_TARGET_UNWIND | EXCEPTION_COLLIDED_UNWIND)

#define IS_UNWINDING(Flag) ((Flag & EXCEPTION_UNWIND) != 0)
#define IS_DISPATCHING(Flag) ((Flag & EXCEPTION_UNWIND) == 0)
#define IS_TARGET_UNWIND(Flag) (Flag & EXCEPTION_TARGET_UNWIND)

#define EXCEPTION_MAXIMUM_PARAMETERS 15

/* Exception records */
typedef struct _EXCEPTION_RECORD {
	NTSTATUS ExceptionCode;
	ULONG ExceptionFlags;
	struct _EXCEPTION_RECORD* ExceptionRecord;
	PVOID ExceptionAddress;
	ULONG NumberParameters;
	ULONG_PTR ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD, * PEXCEPTION_RECORD;

typedef struct _EXCEPTION_RECORD32 {
	NTSTATUS ExceptionCode;
	ULONG ExceptionFlags;
	ULONG ExceptionRecord;
	ULONG ExceptionAddress;
	ULONG NumberParameters;
	ULONG ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD32, * PEXCEPTION_RECORD32;

typedef struct _EXCEPTION_RECORD64 {
	NTSTATUS ExceptionCode;
	ULONG ExceptionFlags;
	ULONG64 ExceptionRecord;
	ULONG64 ExceptionAddress;
	ULONG NumberParameters;
	ULONG __unusedAlignment;
	ULONG64 ExceptionInformation[EXCEPTION_MAXIMUM_PARAMETERS];
} EXCEPTION_RECORD64, * PEXCEPTION_RECORD64;
