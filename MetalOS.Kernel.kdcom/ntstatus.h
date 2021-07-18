#pragma once

typedef long NTSTATUS;

//
// Generic test for success on any status value (non-negative numbers
// indicate success).
//

#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)

//
// MessageId: STATUS_INVALID_PARAMETER
//
// MessageText:
//
// An invalid parameter was passed to a service or function.
//
#define STATUS_INVALID_PARAMETER         ((NTSTATUS)0xC000000DL)    // winnt

//
// MessageId: STATUS_NOT_FOUND
//
// MessageText:
//
// The object was not found.
//
#define STATUS_NOT_FOUND                 ((NTSTATUS)0xC0000225L)

//
// MessageId: STATUS_SUCCESS
//
// MessageText:
//
//  STATUS_SUCCESS
//
#define STATUS_SUCCESS                   ((NTSTATUS)0x00000000L)    // ntsubauth
