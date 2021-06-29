#pragma once
//ReactOS::sdk\include\reactos\windbgkd.h

//
// Packet Size and Control Stream Size
//
#define PACKET_MAX_SIZE                     4000
#define DBGKD_MAXSTREAM                     16

//
// Magic Packet IDs
//
#define INITIAL_PACKET_ID                   0x80800000
#define SYNC_PACKET_ID                      0x00000800

//
// Magic Packet bytes
//
#define BREAKIN_PACKET                      0x62626262
#define BREAKIN_PACKET_BYTE                 0x62
#define PACKET_LEADER                       0x30303030
#define PACKET_LEADER_BYTE                  0x30
#define CONTROL_PACKET_LEADER               0x69696969
#define CONTROL_PACKET_LEADER_BYTE          0x69
#define PACKET_TRAILING_BYTE                0xAA

//
// Packet Types
//
#define PACKET_TYPE_UNUSED                  0
#define PACKET_TYPE_KD_STATE_CHANGE32       1
#define PACKET_TYPE_KD_STATE_MANIPULATE     2
#define PACKET_TYPE_KD_DEBUG_IO             3
#define PACKET_TYPE_KD_ACKNOWLEDGE          4
#define PACKET_TYPE_KD_RESEND               5
#define PACKET_TYPE_KD_RESET                6
#define PACKET_TYPE_KD_STATE_CHANGE64       7
#define PACKET_TYPE_KD_POLL_BREAKIN         8
#define PACKET_TYPE_KD_TRACE_IO             9
#define PACKET_TYPE_KD_CONTROL_REQUEST      10
#define PACKET_TYPE_KD_FILE_IO              11
#define PACKET_TYPE_MAX                     12

//
// Wait State Change Types
//
#define DbgKdMinimumStateChange             0x00003030
#define DbgKdExceptionStateChange           0x00003030
#define DbgKdLoadSymbolsStateChange         0x00003031
#define DbgKdCommandStringStateChange       0x00003032
#define DbgKdMaximumStateChange             0x00003033

//
// This is combined with the basic state change code
// if the state is from an alternate source
//
#define DbgKdAlternateStateChange           0x00010000

//
// Manipulate Types
//
#define DbgKdMinimumManipulate              0x00003130
#define DbgKdReadVirtualMemoryApi           0x00003130
#define DbgKdWriteVirtualMemoryApi          0x00003131
#define DbgKdGetContextApi                  0x00003132
#define DbgKdSetContextApi                  0x00003133
#define DbgKdWriteBreakPointApi             0x00003134
#define DbgKdRestoreBreakPointApi           0x00003135
#define DbgKdContinueApi                    0x00003136
#define DbgKdReadControlSpaceApi            0x00003137
#define DbgKdWriteControlSpaceApi           0x00003138
#define DbgKdReadIoSpaceApi                 0x00003139
#define DbgKdWriteIoSpaceApi                0x0000313A
#define DbgKdRebootApi                      0x0000313B
#define DbgKdContinueApi2                   0x0000313C
#define DbgKdReadPhysicalMemoryApi          0x0000313D
#define DbgKdWritePhysicalMemoryApi         0x0000313E
#define DbgKdQuerySpecialCallsApi           0x0000313F
#define DbgKdSetSpecialCallApi              0x00003140
#define DbgKdClearSpecialCallsApi           0x00003141
#define DbgKdSetInternalBreakPointApi       0x00003142
#define DbgKdGetInternalBreakPointApi       0x00003143
#define DbgKdReadIoSpaceExtendedApi         0x00003144
#define DbgKdWriteIoSpaceExtendedApi        0x00003145
#define DbgKdGetVersionApi                  0x00003146
#define DbgKdWriteBreakPointExApi           0x00003147
#define DbgKdRestoreBreakPointExApi         0x00003148
#define DbgKdCauseBugCheckApi               0x00003149
#define DbgKdSwitchProcessor                0x00003150
#define DbgKdPageInApi                      0x00003151
#define DbgKdReadMachineSpecificRegister    0x00003152
#define DbgKdWriteMachineSpecificRegister   0x00003153
#define OldVlm1                             0x00003154
#define OldVlm2                             0x00003155
#define DbgKdSearchMemoryApi                0x00003156
#define DbgKdGetBusDataApi                  0x00003157
#define DbgKdSetBusDataApi                  0x00003158
#define DbgKdCheckLowMemoryApi              0x00003159
#define DbgKdClearAllInternalBreakpointsApi 0x0000315A
#define DbgKdFillMemoryApi                  0x0000315B
#define DbgKdQueryMemoryApi                 0x0000315C
#define DbgKdSwitchPartition                0x0000315D
#define DbgKdWriteCustomBreakpointApi       0x0000315E
#define DbgKdGetContextExApi                0x0000315F
#define DbgKdSetContextExApi                0x00003160
#define DbgKdMaximumManipulate              0x00003161

//
// Debug I/O Types
//
#define DbgKdPrintStringApi                 0x00003230
#define DbgKdGetStringApi                   0x00003231

//
// Trace I/O Types
//
#define DbgKdPrintTraceApi                  0x00003330

//
// Control Request Types
//
#define DbgKdRequestHardwareBp              0x00004300
#define DbgKdReleaseHardwareBp              0x00004301

//
// File I/O Types
//
#define DbgKdCreateFileApi                 0x00003430
#define DbgKdReadFileApi                   0x00003431
#define DbgKdWriteFileApi                  0x00003432
#define DbgKdCloseFileApi                  0x00003433

//
// Control Report Flags
//
#define REPORT_INCLUDES_SEGS                0x0001
#define REPORT_STANDARD_CS                  0x0002

//
// Protocol Versions
//
#define DBGKD_64BIT_PROTOCOL_VERSION1       5
#define DBGKD_64BIT_PROTOCOL_VERSION2       6

//
// Query Memory Address Spaces
//
#define DBGKD_QUERY_MEMORY_VIRTUAL          0
#define DBGKD_QUERY_MEMORY_PROCESS          0
#define DBGKD_QUERY_MEMORY_SESSION          1
#define DBGKD_QUERY_MEMORY_KERNEL           2

//
// Query Memory Flags
//
#define DBGKD_QUERY_MEMORY_READ             0x01
#define DBGKD_QUERY_MEMORY_WRITE            0x02
#define DBGKD_QUERY_MEMORY_EXECUTE          0x04
#define DBGKD_QUERY_MEMORY_FIXED            0x08

//
// Internal Breakpoint Flags
//
#define DBGKD_INTERNAL_BP_FLAG_COUNTONLY    0x01
#define DBGKD_INTERNAL_BP_FLAG_INVALID      0x02
#define DBGKD_INTERNAL_BP_FLAG_SUSPENDED    0x04
#define DBGKD_INTERNAL_BP_FLAG_DYING        0x08

//
// Fill Memory Flags
//
#define DBGKD_FILL_MEMORY_VIRTUAL           0x01
#define DBGKD_FILL_MEMORY_PHYSICAL          0x02

//
// Physical Memory Caching Flags
//
#define DBGKD_CACHING_DEFAULT               0
#define DBGKD_CACHING_CACHED                1
#define DBGKD_CACHING_UNCACHED              2
#define DBGKD_CACHING_WRITE_COMBINED        3

//
// Partition Switch Flags
//
#define DBGKD_PARTITION_DEFAULT             0x00
#define DBGKD_PARTITION_ALTERNATE           0x01

//
// AMD64 Control Space types
//
#define AMD64_DEBUG_CONTROL_SPACE_KPCR 0
#define AMD64_DEBUG_CONTROL_SPACE_KPRCB 1
#define AMD64_DEBUG_CONTROL_SPACE_KSPECIAL 2
#define AMD64_DEBUG_CONTROL_SPACE_KTHREAD 3


//
// KD Packet Structure
//
typedef struct _KD_PACKET
{
	ULONG PacketLeader;
	USHORT PacketType;
	USHORT ByteCount;
	ULONG PacketId;
	ULONG Checksum;
} KD_PACKET, * PKD_PACKET;

//
// KD Context
//
typedef struct _KD_CONTEXT
{
	ULONG KdpDefaultRetries;
	BOOLEAN KdpControlCPending;
} KD_CONTEXT, * PKD_CONTEXT;

//
// DBGKD Structure for Debug I/O Type Print String
//
typedef struct _DBGKD_PRINT_STRING
{
	ULONG LengthOfString;
} DBGKD_PRINT_STRING, * PDBGKD_PRINT_STRING;

//
// DBGKD Structure for Debug I/O Type Get String
//
typedef struct _DBGKD_GET_STRING
{
	ULONG LengthOfPromptString;
	ULONG LengthOfStringRead;
} DBGKD_GET_STRING, * PDBGKD_GET_STRING;

//
// DBGKD Structure for Debug I/O
//
typedef struct _DBGKD_DEBUG_IO
{
	ULONG ApiNumber;
	USHORT ProcessorLevel;
	USHORT Processor;
	union
	{
		DBGKD_PRINT_STRING PrintString;
		DBGKD_GET_STRING GetString;
	} u;
} DBGKD_DEBUG_IO, * PDBGKD_DEBUG_IO;
