#pragma once

typedef struct LIST_ENTRY32 {
	ULONG Flink;
	ULONG Blink;
} LIST_ENTRY32, * PLIST_ENTRY32;

typedef struct LIST_ENTRY64 {
	ULONGLONG Flink;
	ULONGLONG Blink;
} LIST_ENTRY64, * PLIST_ENTRY64;
