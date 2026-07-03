#pragma once

//
// Efi.h - Self-contained subset of the UEFI types MetalOS actually uses.
//
// Curated from gnu-efi (MSVC/x86_64) so the bootloader and kernel no longer
// need the external/gnu-efi include directory. Each section notes the gnu-efi
// header it was lifted from. ABI-fixed structures (service tables, protocols)
// are reproduced in full so the firmware-provided layouts stay correct.
//

/*============================================================================
 * gnu-efi: x86_64/efibind.h - base integer types and calling convention
 *==========================================================================*/

typedef unsigned __int64    uint64_t;
typedef __int64             int64_t;
typedef unsigned __int32    uint32_t;
typedef __int32             int32_t;
typedef unsigned short      uint16_t;
typedef short               int16_t;
typedef unsigned char       uint8_t;
typedef signed char         int8_t;

typedef uint64_t   UINT64;
typedef int64_t    INT64;
typedef uint32_t   UINT32;
typedef int32_t    INT32;
typedef uint16_t   UINT16;
typedef int16_t    INT16;
typedef uint8_t    UINT8;
typedef int8_t     INT8;
typedef uint16_t   WCHAR;

#undef VOID
#define VOID    void

typedef int64_t    INTN;
typedef uint64_t   UINTN;

#define EFIERR(a)           (0x8000000000000000 | a)
#define EFI_ERROR_MASK      0x8000000000000000

// Force C calling convention for EFI function pointers (MSVC on x64).
#ifndef EFIAPI
#define EFIAPI __cdecl
#endif

#define INTERFACE_DECL(x) struct x

/*============================================================================
 * gnu-efi: efidef.h - core definitions (status, handles, GUID, time, memory)
 *==========================================================================*/

typedef UINT16          CHAR16;
typedef UINT8           CHAR8;
typedef UINT8           BOOLEAN;

#ifndef CONST
   #define CONST const
#endif
#ifndef TRUE
    #define TRUE    ((BOOLEAN) 1)
    #define FALSE   ((BOOLEAN) 0)
#endif
#ifndef NULL
    #define NULL 0
#endif

typedef UINTN           EFI_STATUS;
typedef UINT64          EFI_LBA;
typedef UINTN           EFI_TPL;
typedef VOID            *EFI_HANDLE;
typedef VOID            *EFI_EVENT;

// Prototype argument decoration (direction hints; expand to nothing).
#ifndef IN
    #define IN
    #define OUT
    #define OPTIONAL
#endif

typedef struct {
    UINT32  Data1;
    UINT16  Data2;
    UINT16  Data3;
    UINT8   Data4[8];
} EFI_GUID;

typedef struct {
    UINT16      Year;       // 1998 - 20XX
    UINT8       Month;      // 1 - 12
    UINT8       Day;        // 1 - 31
    UINT8       Hour;       // 0 - 23
    UINT8       Minute;     // 0 - 59
    UINT8       Second;     // 0 - 59
    UINT8       Pad1;
    UINT32      Nanosecond; // 0 - 999,999,999
    INT16       TimeZone;   // -1440 to 1440 or 2047
    UINT8       Daylight;
    UINT8       Pad2;
} EFI_TIME;

typedef UINT64          EFI_PHYSICAL_ADDRESS;
typedef UINT64          EFI_VIRTUAL_ADDRESS;

typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress,
    MaxAllocateType
} EFI_ALLOCATE_TYPE;

typedef enum {
    EfiReservedMemoryType,
    EfiLoaderCode,
    EfiLoaderData,
    EfiBootServicesCode,
    EfiBootServicesData,
    EfiRuntimeServicesCode,
    EfiRuntimeServicesData,
    EfiConventionalMemory,
    EfiUnusableMemory,
    EfiACPIReclaimMemory,
    EfiACPIMemoryNVS,
    EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace,
    EfiPalCode,
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

// range requires a runtime mapping
#define EFI_MEMORY_RUNTIME      0x8000000000000000

#define EFI_MEMORY_DESCRIPTOR_VERSION  1
typedef struct {
    UINT32                          Type;           // 32 bits followed by 32 bit pad
    UINT32                          Pad;
    EFI_PHYSICAL_ADDRESS            PhysicalStart;
    EFI_VIRTUAL_ADDRESS             VirtualStart;
    UINT64                          NumberOfPages;
    UINT64                          Attribute;
} EFI_MEMORY_DESCRIPTOR;

#define EFI_PAGE_SIZE   4096
#define EFI_PAGE_MASK   0xFFF
#define EFI_PAGE_SHIFT  12

/*============================================================================
 * gnu-efi: efidevp.h - device path (only the header + media file path node)
 *==========================================================================*/

typedef struct _EFI_DEVICE_PATH_PROTOCOL {
    UINT8                           Type;
    UINT8                           SubType;
    UINT8                           Length[2];
} EFI_DEVICE_PATH_PROTOCOL;

typedef EFI_DEVICE_PATH_PROTOCOL EFI_DEVICE_PATH;

#define MEDIA_FILEPATH_DP               0x04
typedef struct _FILEPATH_DEVICE_PATH {
    EFI_DEVICE_PATH_PROTOCOL        Header;
    CHAR16                          PathName[1];
} FILEPATH_DEVICE_PATH;

/*============================================================================
 * gnu-efi: efiprot.h - simple file system, file, graphics output, loaded image
 *==========================================================================*/

// --- Simple File System / File protocols ---
#define EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID \
    { 0x964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }

INTERFACE_DECL(_EFI_SIMPLE_FILE_SYSTEM_PROTOCOL);
INTERFACE_DECL(_EFI_FILE_HANDLE);

typedef
EFI_STATUS
(EFIAPI *EFI_VOLUME_OPEN) (
    IN struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL    *This,
    OUT struct _EFI_FILE_HANDLE                   **Root
    );

typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
    UINT64                  Revision;
    EFI_VOLUME_OPEN         OpenVolume;
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_OPEN) (
    IN struct _EFI_FILE_HANDLE  *File,
    OUT struct _EFI_FILE_HANDLE **NewHandle,
    IN CHAR16                   *FileName,
    IN UINT64                   OpenMode,
    IN UINT64                   Attributes
    );

// Open modes
#define EFI_FILE_MODE_READ      0x0000000000000001
#define EFI_FILE_MODE_WRITE     0x0000000000000002
#define EFI_FILE_MODE_CREATE    0x8000000000000000

// File attributes
#define EFI_FILE_READ_ONLY      0x0000000000000001
#define EFI_FILE_HIDDEN         0x0000000000000002
#define EFI_FILE_SYSTEM         0x0000000000000004
#define EFI_FILE_RESERVIED      0x0000000000000008
#define EFI_FILE_DIRECTORY      0x0000000000000010
#define EFI_FILE_ARCHIVE        0x0000000000000020
#define EFI_FILE_VALID_ATTR     0x0000000000000037

typedef EFI_STATUS (EFIAPI *EFI_FILE_CLOSE) (IN struct _EFI_FILE_HANDLE *File);
typedef EFI_STATUS (EFIAPI *EFI_FILE_DELETE) (IN struct _EFI_FILE_HANDLE *File);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_READ) (
    IN struct _EFI_FILE_HANDLE  *File,
    IN OUT UINTN                *BufferSize,
    OUT VOID                    *Buffer
    );

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_WRITE) (
    IN struct _EFI_FILE_HANDLE  *File,
    IN OUT UINTN                *BufferSize,
    IN VOID                     *Buffer
    );

typedef EFI_STATUS (EFIAPI *EFI_FILE_SET_POSITION) (IN struct _EFI_FILE_HANDLE *File, IN UINT64 Position);
typedef EFI_STATUS (EFIAPI *EFI_FILE_GET_POSITION) (IN struct _EFI_FILE_HANDLE *File, OUT UINT64 *Position);

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_GET_INFO) (
    IN struct _EFI_FILE_HANDLE  *File,
    IN EFI_GUID                 *InformationType,
    IN OUT UINTN                *BufferSize,
    OUT VOID                    *Buffer
    );

typedef
EFI_STATUS
(EFIAPI *EFI_FILE_SET_INFO) (
    IN struct _EFI_FILE_HANDLE  *File,
    IN EFI_GUID                 *InformationType,
    IN UINTN                    BufferSize,
    IN VOID                     *Buffer
    );

typedef EFI_STATUS (EFIAPI *EFI_FILE_FLUSH) (IN struct _EFI_FILE_HANDLE *File);

typedef struct {
    EFI_EVENT       Event;
    EFI_STATUS      Status;
    UINTN           BufferSize;
    VOID            *Buffer;
} EFI_FILE_IO_TOKEN;

typedef EFI_STATUS (EFIAPI *EFI_FILE_OPEN_EX)(IN struct _EFI_FILE_HANDLE *File, OUT struct _EFI_FILE_HANDLE **NewHandle, IN CHAR16 *FileName, IN UINT64 OpenMode, IN UINT64 Attributes, IN OUT EFI_FILE_IO_TOKEN *Token);
typedef EFI_STATUS (EFIAPI *EFI_FILE_READ_EX) (IN struct _EFI_FILE_HANDLE *File, IN OUT EFI_FILE_IO_TOKEN *Token);
typedef EFI_STATUS (EFIAPI *EFI_FILE_WRITE_EX) (IN struct _EFI_FILE_HANDLE *File, IN OUT EFI_FILE_IO_TOKEN *Token);
typedef EFI_STATUS (EFIAPI *EFI_FILE_FLUSH_EX) (IN struct _EFI_FILE_HANDLE *File, IN OUT EFI_FILE_IO_TOKEN *Token);

typedef struct _EFI_FILE_HANDLE {
    UINT64                  Revision;
    EFI_FILE_OPEN           Open;
    EFI_FILE_CLOSE          Close;
    EFI_FILE_DELETE         Delete;
    EFI_FILE_READ           Read;
    EFI_FILE_WRITE          Write;
    EFI_FILE_GET_POSITION   GetPosition;
    EFI_FILE_SET_POSITION   SetPosition;
    EFI_FILE_GET_INFO       GetInfo;
    EFI_FILE_SET_INFO       SetInfo;
    EFI_FILE_FLUSH          Flush;
    EFI_FILE_OPEN_EX        OpenEx;
    EFI_FILE_READ_EX        ReadEx;
    EFI_FILE_WRITE_EX       WriteEx;
    EFI_FILE_FLUSH_EX       FlushEx;
} EFI_FILE_PROTOCOL, *EFI_FILE_HANDLE;

typedef EFI_FILE_PROTOCOL EFI_FILE;

#define EFI_FILE_INFO_ID   \
    { 0x9576e92, 0x6d3f, 0x11d2, {0x8e, 0x39, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b} }

typedef struct {
    UINT64                  Size;
    UINT64                  FileSize;
    UINT64                  PhysicalSize;
    EFI_TIME                CreateTime;
    EFI_TIME                LastAccessTime;
    EFI_TIME                ModificationTime;
    UINT64                  Attribute;
    CHAR16                  FileName[1];
} EFI_FILE_INFO;

// --- Graphics Output Protocol ---
#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
   { 0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a } }
typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL EFI_GRAPHICS_OUTPUT_PROTOCOL;

typedef struct {
    UINT32            RedMask;
    UINT32            GreenMask;
    UINT32            BlueMask;
    UINT32            ReservedMask;
} EFI_PIXEL_BITMASK;

typedef enum {
    PixelRedGreenBlueReserved8BitPerColor,
    PixelBlueGreenRedReserved8BitPerColor,
    PixelBitMask,
    PixelBltOnly,
    PixelFormatMax
} EFI_GRAPHICS_PIXEL_FORMAT;

typedef struct {
    UINT32                     Version;
    UINT32                     HorizontalResolution;
    UINT32                     VerticalResolution;
    EFI_GRAPHICS_PIXEL_FORMAT  PixelFormat;
    EFI_PIXEL_BITMASK          PixelInformation;
    UINT32                     PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE) (
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
    IN  UINT32                                ModeNumber,
    OUT UINTN                                 *SizeOfInfo,
    OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
    );

typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE) (
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN  UINT32                       ModeNumber
    );

typedef struct {
    UINT8 Blue;
    UINT8 Green;
    UINT8 Red;
    UINT8 Reserved;
} EFI_GRAPHICS_OUTPUT_BLT_PIXEL;

typedef enum {
    EfiBltVideoFill,
    EfiBltVideoToBltBuffer,
    EfiBltBufferToVideo,
    EfiBltVideoToVideo,
    EfiGraphicsOutputBltOperationMax
} EFI_GRAPHICS_OUTPUT_BLT_OPERATION;

typedef
EFI_STATUS
(EFIAPI *EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT) (
    IN  EFI_GRAPHICS_OUTPUT_PROTOCOL            *This,
    IN  EFI_GRAPHICS_OUTPUT_BLT_PIXEL           *BltBuffer,   OPTIONAL
    IN  EFI_GRAPHICS_OUTPUT_BLT_OPERATION       BltOperation,
    IN  UINTN                                   SourceX,
    IN  UINTN                                   SourceY,
    IN  UINTN                                   DestinationX,
    IN  UINTN                                   DestinationY,
    IN  UINTN                                   Width,
    IN  UINTN                                   Height,
    IN  UINTN                                   Delta         OPTIONAL
    );

typedef struct {
    UINT32                                 MaxMode;
    UINT32                                 Mode;
    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION   *Info;
    UINTN                                  SizeOfInfo;
    EFI_PHYSICAL_ADDRESS                   FrameBufferBase;
    UINTN                                  FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

struct _EFI_GRAPHICS_OUTPUT_PROTOCOL {
    EFI_GRAPHICS_OUTPUT_PROTOCOL_QUERY_MODE  QueryMode;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_SET_MODE    SetMode;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_BLT         Blt;
    EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE        *Mode;
};

// --- Loaded Image Protocol ---
#define EFI_LOADED_IMAGE_PROTOCOL_GUID \
    { 0x5B1B31A1, 0x9562, 0x11d2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B} }

typedef
EFI_STATUS
(EFIAPI *EFI_IMAGE_UNLOAD) (
    IN EFI_HANDLE                   ImageHandle
    );

typedef struct {
    UINT32                          Revision;
    EFI_HANDLE                      ParentHandle;
    struct _EFI_SYSTEM_TABLE        *SystemTable;
    EFI_HANDLE                      DeviceHandle;
    EFI_DEVICE_PATH                 *FilePath;
    VOID                            *Reserved;
    UINT32                          LoadOptionsSize;
    VOID                            *LoadOptions;
    VOID                            *ImageBase;
    UINT64                          ImageSize;
    EFI_MEMORY_TYPE                 ImageCodeType;
    EFI_MEMORY_TYPE                 ImageDataType;
    EFI_IMAGE_UNLOAD                Unload;
} EFI_LOADED_IMAGE_PROTOCOL;

typedef EFI_LOADED_IMAGE_PROTOCOL EFI_LOADED_IMAGE;

/*============================================================================
 * gnu-efi: eficon.h - simple text input/output console protocols
 *==========================================================================*/

INTERFACE_DECL(_SIMPLE_TEXT_OUTPUT_INTERFACE);

typedef EFI_STATUS (EFIAPI *EFI_TEXT_RESET) (IN struct _SIMPLE_TEXT_OUTPUT_INTERFACE *This, IN BOOLEAN ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_OUTPUT_STRING) (IN struct _SIMPLE_TEXT_OUTPUT_INTERFACE *This, IN CHAR16 *String);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_TEST_STRING) (IN struct _SIMPLE_TEXT_OUTPUT_INTERFACE *This, IN CHAR16 *String);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_QUERY_MODE) (IN struct _SIMPLE_TEXT_OUTPUT_INTERFACE *This, IN UINTN ModeNumber, OUT UINTN *Columns, OUT UINTN *Rows);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_MODE) (IN struct _SIMPLE_TEXT_OUTPUT_INTERFACE *This, IN UINTN ModeNumber);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_ATTRIBUTE) (IN struct _SIMPLE_TEXT_OUTPUT_INTERFACE *This, IN UINTN Attribute);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_CLEAR_SCREEN) (IN struct _SIMPLE_TEXT_OUTPUT_INTERFACE *This);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_SET_CURSOR_POSITION) (IN struct _SIMPLE_TEXT_OUTPUT_INTERFACE *This, IN UINTN Column, IN UINTN Row);
typedef EFI_STATUS (EFIAPI *EFI_TEXT_ENABLE_CURSOR) (IN struct _SIMPLE_TEXT_OUTPUT_INTERFACE *This, IN BOOLEAN Enable);

typedef struct {
    INT32                           MaxMode;
    INT32                           Mode;
    INT32                           Attribute;
    INT32                           CursorColumn;
    INT32                           CursorRow;
    BOOLEAN                         CursorVisible;
} SIMPLE_TEXT_OUTPUT_MODE;

typedef struct _SIMPLE_TEXT_OUTPUT_INTERFACE {
    EFI_TEXT_RESET                  Reset;
    EFI_TEXT_OUTPUT_STRING          OutputString;
    EFI_TEXT_TEST_STRING            TestString;
    EFI_TEXT_QUERY_MODE             QueryMode;
    EFI_TEXT_SET_MODE               SetMode;
    EFI_TEXT_SET_ATTRIBUTE          SetAttribute;
    EFI_TEXT_CLEAR_SCREEN           ClearScreen;
    EFI_TEXT_SET_CURSOR_POSITION    SetCursorPosition;
    EFI_TEXT_ENABLE_CURSOR          EnableCursor;
    SIMPLE_TEXT_OUTPUT_MODE         *Mode;
} SIMPLE_TEXT_OUTPUT_INTERFACE, EFI_SIMPLE_TEXT_OUT_PROTOCOL;

INTERFACE_DECL(_SIMPLE_INPUT_INTERFACE);

typedef struct {
    UINT16                              ScanCode;
    CHAR16                              UnicodeChar;
} EFI_INPUT_KEY;

typedef EFI_STATUS (EFIAPI *EFI_INPUT_RESET) (IN struct _SIMPLE_INPUT_INTERFACE *This, IN BOOLEAN ExtendedVerification);
typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY) (IN struct _SIMPLE_INPUT_INTERFACE *This, OUT EFI_INPUT_KEY *Key);

typedef struct _SIMPLE_INPUT_INTERFACE {
    EFI_INPUT_RESET                     Reset;
    EFI_INPUT_READ_KEY                  ReadKeyStroke;
    EFI_EVENT                           WaitForKey;
} SIMPLE_INPUT_INTERFACE, EFI_SIMPLE_TEXT_IN_PROTOCOL;

/*============================================================================
 * gnu-efi: efiapi.h - boot/runtime service tables and system table
 *==========================================================================*/

INTERFACE_DECL(_EFI_SYSTEM_TABLE);

// --- Memory services ---
typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_PAGES) (IN EFI_ALLOCATE_TYPE Type, IN EFI_MEMORY_TYPE MemoryType, IN UINTN NoPages, OUT EFI_PHYSICAL_ADDRESS *Memory);
typedef EFI_STATUS (EFIAPI *EFI_FREE_PAGES) (IN EFI_PHYSICAL_ADDRESS Memory, IN UINTN NoPages);
typedef EFI_STATUS (EFIAPI *EFI_GET_MEMORY_MAP) (IN OUT UINTN *MemoryMapSize, IN OUT EFI_MEMORY_DESCRIPTOR *MemoryMap, OUT UINTN *MapKey, OUT UINTN *DescriptorSize, OUT UINT32 *DescriptorVersion);
typedef EFI_STATUS (EFIAPI *EFI_ALLOCATE_POOL) (IN EFI_MEMORY_TYPE PoolType, IN UINTN Size, OUT VOID **Buffer);
typedef EFI_STATUS (EFIAPI *EFI_FREE_POOL) (IN VOID *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_SET_VIRTUAL_ADDRESS_MAP) (IN UINTN MemoryMapSize, IN UINTN DescriptorSize, IN UINT32 DescriptorVersion, IN EFI_MEMORY_DESCRIPTOR *VirtualMap);
typedef EFI_STATUS (EFIAPI *EFI_CONVERT_POINTER) (IN UINTN DebugDisposition, IN OUT VOID **Address);

#define NextMemoryDescriptor(Ptr,Size)  ((EFI_MEMORY_DESCRIPTOR *) (((UINT8 *) Ptr) + Size))

// --- Event & timer services ---
typedef VOID (EFIAPI *EFI_EVENT_NOTIFY) (IN EFI_EVENT Event, IN VOID *Context);
typedef EFI_STATUS (EFIAPI *EFI_CREATE_EVENT) (IN UINT32 Type, IN EFI_TPL NotifyTpl, IN EFI_EVENT_NOTIFY NotifyFunction, IN VOID *NotifyContext, OUT EFI_EVENT *Event);

typedef enum {
    TimerCancel,
    TimerPeriodic,
    TimerRelative,
    TimerTypeMax
} EFI_TIMER_DELAY;

typedef EFI_STATUS (EFIAPI *EFI_SET_TIMER) (IN EFI_EVENT Event, IN EFI_TIMER_DELAY Type, IN UINT64 TriggerTime);
typedef EFI_STATUS (EFIAPI *EFI_SIGNAL_EVENT) (IN EFI_EVENT Event);
typedef EFI_STATUS (EFIAPI *EFI_WAIT_FOR_EVENT) (IN UINTN NumberOfEvents, IN EFI_EVENT *Event, OUT UINTN *Index);
typedef EFI_STATUS (EFIAPI *EFI_CLOSE_EVENT) (IN EFI_EVENT Event);
typedef EFI_STATUS (EFIAPI *EFI_CHECK_EVENT) (IN EFI_EVENT Event);

typedef EFI_TPL (EFIAPI *EFI_RAISE_TPL) (IN EFI_TPL NewTpl);
typedef VOID (EFIAPI *EFI_RESTORE_TPL) (IN EFI_TPL OldTpl);

// --- Variable services ---
typedef EFI_STATUS (EFIAPI *EFI_GET_VARIABLE) (IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid, OUT UINT32 *Attributes OPTIONAL, IN OUT UINTN *DataSize, OUT VOID *Data);
typedef EFI_STATUS (EFIAPI *EFI_GET_NEXT_VARIABLE_NAME) (IN OUT UINTN *VariableNameSize, IN OUT CHAR16 *VariableName, IN OUT EFI_GUID *VendorGuid);
typedef EFI_STATUS (EFIAPI *EFI_SET_VARIABLE) (IN CHAR16 *VariableName, IN EFI_GUID *VendorGuid, IN UINT32 Attributes, IN UINTN DataSize, IN VOID *Data);

// --- Time services ---
typedef struct {
    UINT32                      Resolution;
    UINT32                      Accuracy;
    BOOLEAN                     SetsToZero;
} EFI_TIME_CAPABILITIES;

typedef EFI_STATUS (EFIAPI *EFI_GET_TIME) (OUT EFI_TIME *Time, OUT EFI_TIME_CAPABILITIES *Capabilities OPTIONAL);
typedef EFI_STATUS (EFIAPI *EFI_SET_TIME) (IN EFI_TIME *Time);
typedef EFI_STATUS (EFIAPI *EFI_GET_WAKEUP_TIME) (OUT BOOLEAN *Enabled, OUT BOOLEAN *Pending, OUT EFI_TIME *Time);
typedef EFI_STATUS (EFIAPI *EFI_SET_WAKEUP_TIME) (IN BOOLEAN Enable, IN EFI_TIME *Time OPTIONAL);

// --- Image services ---
typedef EFI_STATUS (EFIAPI *EFI_IMAGE_LOAD) (IN BOOLEAN BootPolicy, IN EFI_HANDLE ParentImageHandle, IN EFI_DEVICE_PATH *FilePath, IN VOID *SourceBuffer OPTIONAL, IN UINTN SourceSize, OUT EFI_HANDLE *ImageHandle);
typedef EFI_STATUS (EFIAPI *EFI_IMAGE_START) (IN EFI_HANDLE ImageHandle, OUT UINTN *ExitDataSize, OUT CHAR16 **ExitData OPTIONAL);
typedef EFI_STATUS (EFIAPI *EFI_EXIT) (IN EFI_HANDLE ImageHandle, IN EFI_STATUS ExitStatus, IN UINTN ExitDataSize, IN CHAR16 *ExitData OPTIONAL);
typedef EFI_STATUS (EFIAPI *EFI_EXIT_BOOT_SERVICES) (IN EFI_HANDLE ImageHandle, IN UINTN MapKey);

// --- Misc services ---
typedef EFI_STATUS (EFIAPI *EFI_STALL) (IN UINTN Microseconds);
typedef EFI_STATUS (EFIAPI *EFI_SET_WATCHDOG_TIMER) (IN UINTN Timeout, IN UINT64 WatchdogCode, IN UINTN DataSize, IN CHAR16 *WatchdogData OPTIONAL);
typedef EFI_STATUS (EFIAPI *EFI_CONNECT_CONTROLLER) (IN EFI_HANDLE ControllerHandle, IN EFI_HANDLE *DriverImageHandle OPTIONAL, IN EFI_DEVICE_PATH *RemainingDevicePath OPTIONAL, IN BOOLEAN Recursive);
typedef EFI_STATUS (EFIAPI *EFI_DISCONNECT_CONTROLLER) (IN EFI_HANDLE ControllerHandle, IN EFI_HANDLE DriverImageHandle OPTIONAL, IN EFI_HANDLE ChildHandle OPTIONAL);

// --- Open/close protocol services ---
#define EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL  0x00000001
#define EFI_OPEN_PROTOCOL_GET_PROTOCOL        0x00000002
#define EFI_OPEN_PROTOCOL_TEST_PROTOCOL       0x00000004
#define EFI_OPEN_PROTOCOL_BY_CHILD_CONTROLLER 0x00000008
#define EFI_OPEN_PROTOCOL_BY_DRIVER           0x00000010
#define EFI_OPEN_PROTOCOL_EXCLUSIVE           0x00000020

typedef EFI_STATUS (EFIAPI *EFI_OPEN_PROTOCOL) (IN EFI_HANDLE Handle, IN EFI_GUID *Protocol, OUT VOID **Interface OPTIONAL, IN EFI_HANDLE AgentHandle, IN EFI_HANDLE ControllerHandle, IN UINT32 Attributes);
typedef EFI_STATUS (EFIAPI *EFI_CLOSE_PROTOCOL) (IN EFI_HANDLE Handle, IN EFI_GUID *Protocol, IN EFI_HANDLE AgentHandle, IN EFI_HANDLE ControllerHandle);

typedef struct {
    EFI_HANDLE                  AgentHandle;
    EFI_HANDLE                  ControllerHandle;
    UINT32                      Attributes;
    UINT32                      OpenCount;
} EFI_OPEN_PROTOCOL_INFORMATION_ENTRY;

typedef EFI_STATUS (EFIAPI *EFI_OPEN_PROTOCOL_INFORMATION) (IN EFI_HANDLE Handle, IN EFI_GUID *Protocol, OUT EFI_OPEN_PROTOCOL_INFORMATION_ENTRY **EntryBuffer, OUT UINTN *EntryCount);
typedef EFI_STATUS (EFIAPI *EFI_PROTOCOLS_PER_HANDLE) (IN EFI_HANDLE Handle, OUT EFI_GUID ***ProtocolBuffer, OUT UINTN *ProtocolBufferCount);

typedef enum {
    AllHandles,
    ByRegisterNotify,
    ByProtocol
} EFI_LOCATE_SEARCH_TYPE;

typedef EFI_STATUS (EFIAPI *EFI_LOCATE_HANDLE_BUFFER) (IN EFI_LOCATE_SEARCH_TYPE SearchType, IN EFI_GUID *Protocol OPTIONAL, IN VOID *SearchKey OPTIONAL, IN OUT UINTN *NoHandles, OUT EFI_HANDLE **Buffer);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_PROTOCOL) (IN EFI_GUID *Protocol, IN VOID *Registration OPTIONAL, OUT VOID **Interface);
typedef EFI_STATUS (EFIAPI *EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES) (IN OUT EFI_HANDLE *Handle, ...);
typedef EFI_STATUS (EFIAPI *EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES) (IN OUT EFI_HANDLE Handle, ...);
typedef EFI_STATUS (EFIAPI *EFI_CALCULATE_CRC32) (IN VOID *Data, IN UINTN DataSize, OUT UINT32 *Crc32);
typedef VOID (EFIAPI *EFI_COPY_MEM) (IN VOID *Destination, IN VOID *Source, IN UINTN Length);
typedef VOID (EFIAPI *EFI_SET_MEM) (IN VOID *Buffer, IN UINTN Size, IN UINT8 Value);
typedef EFI_STATUS (EFIAPI *EFI_CREATE_EVENT_EX) (IN UINT32 Type, IN EFI_TPL NotifyTpl, IN EFI_EVENT_NOTIFY NotifyFunction OPTIONAL, IN const VOID *NotifyContext OPTIONAL, IN const EFI_GUID EventGroup OPTIONAL, OUT EFI_EVENT *Event);

typedef enum {
    EfiResetCold,
    EfiResetWarm,
    EfiResetShutdown
} EFI_RESET_TYPE;

typedef EFI_STATUS (EFIAPI *EFI_RESET_SYSTEM) (IN EFI_RESET_TYPE ResetType, IN EFI_STATUS ResetStatus, IN UINTN DataSize, IN CHAR16 *ResetData OPTIONAL);
typedef EFI_STATUS (EFIAPI *EFI_GET_NEXT_MONOTONIC_COUNT) (OUT UINT64 *Count);
typedef EFI_STATUS (EFIAPI *EFI_GET_NEXT_HIGH_MONO_COUNT) (OUT UINT32 *HighCount);

typedef struct {
    EFI_GUID                    CapsuleGuid;
    UINT32                      HeaderSize;
    UINT32                      Flags;
    UINT32                      CapsuleImageSize;
} EFI_CAPSULE_HEADER;

typedef EFI_STATUS (EFIAPI *EFI_UPDATE_CAPSULE) (IN EFI_CAPSULE_HEADER **CapsuleHeaderArray, IN UINTN CapsuleCount, IN EFI_PHYSICAL_ADDRESS ScatterGatherList OPTIONAL);
typedef EFI_STATUS (EFIAPI *EFI_QUERY_CAPSULE_CAPABILITIES) (IN EFI_CAPSULE_HEADER **CapsuleHeaderArray, IN UINTN CapsuleCount, OUT UINT64 *MaximumCapsuleSize, OUT EFI_RESET_TYPE *ResetType);
typedef EFI_STATUS (EFIAPI *EFI_QUERY_VARIABLE_INFO) (IN UINT32 Attributes, OUT UINT64 *MaximumVariableStorageSize, OUT UINT64 *RemainingVariableStorageSize, OUT UINT64 *MaximumVariableSize);

// --- Protocol handler services ---
typedef enum {
    EFI_NATIVE_INTERFACE,
    EFI_PCODE_INTERFACE
} EFI_INTERFACE_TYPE;

typedef EFI_STATUS (EFIAPI *EFI_INSTALL_PROTOCOL_INTERFACE) (IN OUT EFI_HANDLE *Handle, IN EFI_GUID *Protocol, IN EFI_INTERFACE_TYPE InterfaceType, IN VOID *Interface);
typedef EFI_STATUS (EFIAPI *EFI_REINSTALL_PROTOCOL_INTERFACE) (IN EFI_HANDLE Handle, IN EFI_GUID *Protocol, IN VOID *OldInterface, IN VOID *NewInterface);
typedef EFI_STATUS (EFIAPI *EFI_UNINSTALL_PROTOCOL_INTERFACE) (IN EFI_HANDLE Handle, IN EFI_GUID *Protocol, IN VOID *Interface);
typedef EFI_STATUS (EFIAPI *EFI_HANDLE_PROTOCOL) (IN EFI_HANDLE Handle, IN EFI_GUID *Protocol, OUT VOID **Interface);
typedef EFI_STATUS (EFIAPI *EFI_REGISTER_PROTOCOL_NOTIFY) (IN EFI_GUID *Protocol, IN EFI_EVENT Event, OUT VOID **Registration);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_HANDLE) (IN EFI_LOCATE_SEARCH_TYPE SearchType, IN EFI_GUID *Protocol OPTIONAL, IN VOID *SearchKey OPTIONAL, IN OUT UINTN *BufferSize, OUT EFI_HANDLE *Buffer);
typedef EFI_STATUS (EFIAPI *EFI_LOCATE_DEVICE_PATH) (IN EFI_GUID *Protocol, IN OUT EFI_DEVICE_PATH **DevicePath, OUT EFI_HANDLE *Device);
typedef EFI_STATUS (EFIAPI *EFI_INSTALL_CONFIGURATION_TABLE) (IN EFI_GUID *Guid, IN VOID *Table);

// --- Standard table header ---
typedef struct _EFI_TABLE_HEADER {
    UINT64                      Signature;
    UINT32                      Revision;
    UINT32                      HeaderSize;
    UINT32                      CRC32;
    UINT32                      Reserved;
} EFI_TABLE_HEADER;

// --- Runtime Services table ---
typedef struct {
    EFI_TABLE_HEADER                Hdr;

    EFI_GET_TIME                    GetTime;
    EFI_SET_TIME                    SetTime;
    EFI_GET_WAKEUP_TIME             GetWakeupTime;
    EFI_SET_WAKEUP_TIME             SetWakeupTime;

    EFI_SET_VIRTUAL_ADDRESS_MAP     SetVirtualAddressMap;
    EFI_CONVERT_POINTER             ConvertPointer;

    EFI_GET_VARIABLE                GetVariable;
    EFI_GET_NEXT_VARIABLE_NAME      GetNextVariableName;
    EFI_SET_VARIABLE                SetVariable;

    EFI_GET_NEXT_HIGH_MONO_COUNT    GetNextHighMonotonicCount;
    EFI_RESET_SYSTEM                ResetSystem;

    EFI_UPDATE_CAPSULE              UpdateCapsule;
    EFI_QUERY_CAPSULE_CAPABILITIES  QueryCapsuleCapabilities;
    EFI_QUERY_VARIABLE_INFO         QueryVariableInfo;
} EFI_RUNTIME_SERVICES;

// --- Boot Services table ---
typedef struct _EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER                Hdr;

    EFI_RAISE_TPL                   RaiseTPL;
    EFI_RESTORE_TPL                 RestoreTPL;

    EFI_ALLOCATE_PAGES              AllocatePages;
    EFI_FREE_PAGES                  FreePages;
    EFI_GET_MEMORY_MAP              GetMemoryMap;
    EFI_ALLOCATE_POOL               AllocatePool;
    EFI_FREE_POOL                   FreePool;

    EFI_CREATE_EVENT                CreateEvent;
    EFI_SET_TIMER                   SetTimer;
    EFI_WAIT_FOR_EVENT              WaitForEvent;
    EFI_SIGNAL_EVENT                SignalEvent;
    EFI_CLOSE_EVENT                 CloseEvent;
    EFI_CHECK_EVENT                 CheckEvent;

    EFI_INSTALL_PROTOCOL_INTERFACE  InstallProtocolInterface;
    EFI_REINSTALL_PROTOCOL_INTERFACE ReinstallProtocolInterface;
    EFI_UNINSTALL_PROTOCOL_INTERFACE UninstallProtocolInterface;
    EFI_HANDLE_PROTOCOL             HandleProtocol;
    EFI_HANDLE_PROTOCOL             PCHandleProtocol;
    EFI_REGISTER_PROTOCOL_NOTIFY    RegisterProtocolNotify;
    EFI_LOCATE_HANDLE               LocateHandle;
    EFI_LOCATE_DEVICE_PATH          LocateDevicePath;
    EFI_INSTALL_CONFIGURATION_TABLE InstallConfigurationTable;

    EFI_IMAGE_LOAD                  LoadImage;
    EFI_IMAGE_START                 StartImage;
    EFI_EXIT                        Exit;
    EFI_IMAGE_UNLOAD                UnloadImage;
    EFI_EXIT_BOOT_SERVICES          ExitBootServices;

    EFI_GET_NEXT_MONOTONIC_COUNT    GetNextMonotonicCount;
    EFI_STALL                       Stall;
    EFI_SET_WATCHDOG_TIMER          SetWatchdogTimer;

    EFI_CONNECT_CONTROLLER          ConnectController;
    EFI_DISCONNECT_CONTROLLER       DisconnectController;

    EFI_OPEN_PROTOCOL               OpenProtocol;
    EFI_CLOSE_PROTOCOL              CloseProtocol;
    EFI_OPEN_PROTOCOL_INFORMATION   OpenProtocolInformation;

    EFI_PROTOCOLS_PER_HANDLE        ProtocolsPerHandle;
    EFI_LOCATE_HANDLE_BUFFER        LocateHandleBuffer;
    EFI_LOCATE_PROTOCOL             LocateProtocol;
    EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES InstallMultipleProtocolInterfaces;
    EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES UninstallMultipleProtocolInterfaces;

    EFI_CALCULATE_CRC32             CalculateCrc32;

    EFI_COPY_MEM                    CopyMem;
    EFI_SET_MEM                     SetMem;
    EFI_CREATE_EVENT_EX             CreateEventEx;
} EFI_BOOT_SERVICES;

// --- Configuration table + well-known GUIDs ---
#define ACPI_20_TABLE_GUID  \
    { 0x8868e871, 0xe4f1, 0x11d3, {0xbc, 0x22, 0x0, 0x80, 0xc7, 0x3c, 0x88, 0x81} }
#define ACPI_TABLE_GUID    \
    { 0xeb9d2d30, 0x2d88, 0x11d3, {0x9a, 0x16, 0x0, 0x90, 0x27, 0x3f, 0xc1, 0x4d} }

typedef struct _EFI_CONFIGURATION_TABLE {
    EFI_GUID                VendorGuid;
    VOID                    *VendorTable;
} EFI_CONFIGURATION_TABLE;

// --- System table ---
typedef struct _EFI_SYSTEM_TABLE {
    EFI_TABLE_HEADER                Hdr;

    CHAR16                          *FirmwareVendor;
    UINT32                          FirmwareRevision;

    EFI_HANDLE                      ConsoleInHandle;
    SIMPLE_INPUT_INTERFACE          *ConIn;

    EFI_HANDLE                      ConsoleOutHandle;
    SIMPLE_TEXT_OUTPUT_INTERFACE    *ConOut;

    EFI_HANDLE                      StandardErrorHandle;
    SIMPLE_TEXT_OUTPUT_INTERFACE    *StdErr;

    EFI_RUNTIME_SERVICES            *RuntimeServices;
    EFI_BOOT_SERVICES               *BootServices;

    UINTN                           NumberOfTableEntries;
    EFI_CONFIGURATION_TABLE         *ConfigurationTable;
} EFI_SYSTEM_TABLE;

/*============================================================================
 * gnu-efi: efierr.h - status codes
 *==========================================================================*/

#define EFIWARN(a)                            (a)
#define EFI_ERROR(a)              (((INTN) a) < 0)

#define EFI_SUCCESS                             0
#define EFI_LOAD_ERROR                  EFIERR(1)
#define EFI_INVALID_PARAMETER           EFIERR(2)
#define EFI_UNSUPPORTED                 EFIERR(3)
#define EFI_BAD_BUFFER_SIZE             EFIERR(4)
#define EFI_BUFFER_TOO_SMALL            EFIERR(5)
#define EFI_NOT_READY                   EFIERR(6)
#define EFI_DEVICE_ERROR                EFIERR(7)
#define EFI_WRITE_PROTECTED             EFIERR(8)
#define EFI_OUT_OF_RESOURCES            EFIERR(9)
#define EFI_VOLUME_CORRUPTED            EFIERR(10)
#define EFI_VOLUME_FULL                 EFIERR(11)
#define EFI_NO_MEDIA                    EFIERR(12)
#define EFI_MEDIA_CHANGED               EFIERR(13)
#define EFI_NOT_FOUND                   EFIERR(14)
#define EFI_ACCESS_DENIED               EFIERR(15)
#define EFI_NO_RESPONSE                 EFIERR(16)
#define EFI_NO_MAPPING                  EFIERR(17)
#define EFI_TIMEOUT                     EFIERR(18)
#define EFI_NOT_STARTED                 EFIERR(19)
#define EFI_ALREADY_STARTED             EFIERR(20)
#define EFI_ABORTED                     EFIERR(21)
#define EFI_ICMP_ERROR                  EFIERR(22)
#define EFI_TFTP_ERROR                  EFIERR(23)
#define EFI_PROTOCOL_ERROR              EFIERR(24)
#define EFI_INCOMPATIBLE_VERSION        EFIERR(25)
#define EFI_SECURITY_VIOLATION          EFIERR(26)
#define EFI_CRC_ERROR                   EFIERR(27)
#define EFI_END_OF_MEDIA                EFIERR(28)
#define EFI_END_OF_FILE                 EFIERR(31)
#define EFI_INVALID_LANGUAGE            EFIERR(32)
#define EFI_COMPROMISED_DATA            EFIERR(33)

#define EFI_WARN_UNKOWN_GLYPH           EFIWARN(1)
#define EFI_WARN_DELETE_FAILURE         EFIWARN(2)
#define EFI_WARN_WRITE_FAILURE          EFIWARN(3)
#define EFI_WARN_BUFFER_TOO_SMALL       EFIWARN(4)

/*============================================================================
 * gnu-efi: efilib.h - global aliases and protocol-GUID handles
 *
 * The bootloader defines the gEfi*Guid storage itself (EfiMain.cpp) and passes
 * ST/BS/RT explicitly; these aliases/externs preserve the gnu-efi spellings the
 * boot sources reference.
 *==========================================================================*/

extern EFI_GUID gEfiLoadedImageProtocolGuid;
#define LoadedImageProtocol gEfiLoadedImageProtocolGuid

extern EFI_GUID gEfiGraphicsOutputProtocolGuid;
#define GraphicsOutputProtocol gEfiGraphicsOutputProtocolGuid

extern EFI_GUID gEfiSimpleFileSystemProtocolGuid;
#define FileSystemProtocol gEfiSimpleFileSystemProtocolGuid

extern EFI_GUID gEfiFileInfoGuid;
#define GenericFileInfo gEfiFileInfoGuid
