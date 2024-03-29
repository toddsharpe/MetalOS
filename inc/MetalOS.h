#pragma once

/******************************/
/*MetalOS Public Facing Header*/
/******************************/

//System Call interface is kept as C-compatible despite this header not being C-friendly
//This is to allow potential C# interop to be clean

#include <cstdint>

//PixelBlueGreenRedReserved8BitPerColor
struct BGRRPixel
{	
	union
	{
		struct
		{
			uint32_t Blue : 8;
			uint32_t Green : 8;
			uint32_t Red : 8;
			uint32_t Reserved : 8;
		};
		uint32_t AsUint32;
	};
};
static_assert(sizeof(struct BGRRPixel) == sizeof(uint32_t), "Pixel greater than UINT32 in size");
typedef struct BGRRPixel Color;

class Colors
{
public:
	static constexpr Color White = { 0xFF, 0xFF, 0xFF, 0x00 };
	static constexpr Color Blue = { 0xFF, 0x00, 0x00, 0x00 };
	static constexpr Color Green = { 0x00, 0xFF, 0x00, 0x00 };
	static constexpr Color Red = { 0x00, 0x00, 0xFF, 0x00 };
	static constexpr Color Black = { 0x00, 0x00, 0x00, 0x00 };
};

struct Point2D
{
	size_t X;
	size_t Y;
};

struct Rectangle
{
	struct Point2D P1;
	struct Point2D P2;
};

struct ProcessInfo
{
	uint32_t Id;
};

enum class DayOfWeek
{
	Sunday,
	Monday,
	Tuesday,
	Wednesday,
	Thursday,
	Friday,
	Saturday
};

struct SystemTime
{
	uint16_t Year;
	uint16_t Month;
	DayOfWeek DayOfTheWeek;
	uint16_t Day;
	uint16_t Hour;
	uint16_t Minute;
	uint16_t Second;
	uint16_t Milliseconds;
};

enum class SystemArchitecture : uint32_t
{
	Unknown = 0,
	x64 = 1
};

struct SystemInfo
{
	uint32_t PageSize;
	SystemArchitecture Architecture;
	uint32_t NumberOfProcessors;
	uint32_t AllocationGranularity;
	uintptr_t MinimumApplicationAddress;
	uintptr_t MaximumApplicationAddress;
};

enum class GenericAccess
{
	Read = (1 << 0),
	Write = (1 << 1),
};

typedef void* Handle;
typedef uint32_t (*ThreadStart)(void* parameter);

typedef uint16_t VirtualKey;
//Virtual Keys from Windows
#define  _WIN32_WINNT   0x0A00
#define VK_UNMAPPED 00
#ifndef NOVIRTUALKEYCODES


/*
 * Virtual Keys, Standard Set
 */
#define VK_LBUTTON        0x01
#define VK_RBUTTON        0x02
#define VK_CANCEL         0x03
#define VK_MBUTTON        0x04    /* NOT contiguous with L & RBUTTON */

#if(_WIN32_WINNT >= 0x0500)
#define VK_XBUTTON1       0x05    /* NOT contiguous with L & RBUTTON */
#define VK_XBUTTON2       0x06    /* NOT contiguous with L & RBUTTON */
#endif /* _WIN32_WINNT >= 0x0500 */

 /*
  * 0x07 : reserved
  */


#define VK_BACK           0x08
#define VK_TAB            0x09

  /*
   * 0x0A - 0x0B : reserved
   */

#define VK_CLEAR          0x0C
#define VK_RETURN         0x0D

   /*
	* 0x0E - 0x0F : unassigned
	*/

#define VK_SHIFT          0x10
#define VK_CONTROL        0x11
#define VK_MENU           0x12
#define VK_PAUSE          0x13
#define VK_CAPITAL        0x14

#define VK_KANA           0x15
#define VK_HANGEUL        0x15  /* old name - should be here for compatibility */
#define VK_HANGUL         0x15

	/*
	 * 0x16 : unassigned
	 */

#define VK_JUNJA          0x17
#define VK_FINAL          0x18
#define VK_HANJA          0x19
#define VK_KANJI          0x19

	 /*
	  * 0x1A : unassigned
	  */

#define VK_ESCAPE         0x1B

#define VK_CONVERT        0x1C
#define VK_NONCONVERT     0x1D
#define VK_ACCEPT         0x1E
#define VK_MODECHANGE     0x1F

#define VK_SPACE          0x20
#define VK_PRIOR          0x21
#define VK_NEXT           0x22
#define VK_END            0x23
#define VK_HOME           0x24
#define VK_LEFT           0x25
#define VK_UP             0x26
#define VK_RIGHT          0x27
#define VK_DOWN           0x28
#define VK_SELECT         0x29
#define VK_PRINT          0x2A
#define VK_EXECUTE        0x2B
#define VK_SNAPSHOT       0x2C
#define VK_INSERT         0x2D
#define VK_DELETE         0x2E
#define VK_HELP           0x2F

	  /*
	   * VK_0 - VK_9 are the same as ASCII '0' - '9' (0x30 - 0x39)
	   * 0x3A - 0x40 : unassigned
	   * VK_A - VK_Z are the same as ASCII 'A' - 'Z' (0x41 - 0x5A)
	   */

#define VK_LWIN           0x5B
#define VK_RWIN           0x5C
#define VK_APPS           0x5D

	   /*
		* 0x5E : reserved
		*/

#define VK_SLEEP          0x5F

#define VK_NUMPAD0        0x60
#define VK_NUMPAD1        0x61
#define VK_NUMPAD2        0x62
#define VK_NUMPAD3        0x63
#define VK_NUMPAD4        0x64
#define VK_NUMPAD5        0x65
#define VK_NUMPAD6        0x66
#define VK_NUMPAD7        0x67
#define VK_NUMPAD8        0x68
#define VK_NUMPAD9        0x69
#define VK_MULTIPLY       0x6A
#define VK_ADD            0x6B
#define VK_SEPARATOR      0x6C
#define VK_SUBTRACT       0x6D
#define VK_DECIMAL        0x6E
#define VK_DIVIDE         0x6F
#define VK_F1             0x70
#define VK_F2             0x71
#define VK_F3             0x72
#define VK_F4             0x73
#define VK_F5             0x74
#define VK_F6             0x75
#define VK_F7             0x76
#define VK_F8             0x77
#define VK_F9             0x78
#define VK_F10            0x79
#define VK_F11            0x7A
#define VK_F12            0x7B
#define VK_F13            0x7C
#define VK_F14            0x7D
#define VK_F15            0x7E
#define VK_F16            0x7F
#define VK_F17            0x80
#define VK_F18            0x81
#define VK_F19            0x82
#define VK_F20            0x83
#define VK_F21            0x84
#define VK_F22            0x85
#define VK_F23            0x86
#define VK_F24            0x87

#if(_WIN32_WINNT >= 0x0604)

		/*
		 * 0x88 - 0x8F : UI navigation
		 */

#define VK_NAVIGATION_VIEW     0x88 // reserved
#define VK_NAVIGATION_MENU     0x89 // reserved
#define VK_NAVIGATION_UP       0x8A // reserved
#define VK_NAVIGATION_DOWN     0x8B // reserved
#define VK_NAVIGATION_LEFT     0x8C // reserved
#define VK_NAVIGATION_RIGHT    0x8D // reserved
#define VK_NAVIGATION_ACCEPT   0x8E // reserved
#define VK_NAVIGATION_CANCEL   0x8F // reserved

#endif /* _WIN32_WINNT >= 0x0604 */

#define VK_NUMLOCK        0x90
#define VK_SCROLL         0x91

		 /*
		  * NEC PC-9800 kbd definitions
		  */
#define VK_OEM_NEC_EQUAL  0x92   // '=' key on numpad

		  /*
		   * Fujitsu/OASYS kbd definitions
		   */
#define VK_OEM_FJ_JISHO   0x92   // 'Dictionary' key
#define VK_OEM_FJ_MASSHOU 0x93   // 'Unregister word' key
#define VK_OEM_FJ_TOUROKU 0x94   // 'Register word' key
#define VK_OEM_FJ_LOYA    0x95   // 'Left OYAYUBI' key
#define VK_OEM_FJ_ROYA    0x96   // 'Right OYAYUBI' key

		   /*
			* 0x97 - 0x9F : unassigned
			*/

			/*
			 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
			 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
			 * No other API or message will distinguish left and right keys in this way.
			 */
#define VK_LSHIFT         0xA0
#define VK_RSHIFT         0xA1
#define VK_LCONTROL       0xA2
#define VK_RCONTROL       0xA3
#define VK_LMENU          0xA4
#define VK_RMENU          0xA5

#if(_WIN32_WINNT >= 0x0500)
#define VK_BROWSER_BACK        0xA6
#define VK_BROWSER_FORWARD     0xA7
#define VK_BROWSER_REFRESH     0xA8
#define VK_BROWSER_STOP        0xA9
#define VK_BROWSER_SEARCH      0xAA
#define VK_BROWSER_FAVORITES   0xAB
#define VK_BROWSER_HOME        0xAC

#define VK_VOLUME_MUTE         0xAD
#define VK_VOLUME_DOWN         0xAE
#define VK_VOLUME_UP           0xAF
#define VK_MEDIA_NEXT_TRACK    0xB0
#define VK_MEDIA_PREV_TRACK    0xB1
#define VK_MEDIA_STOP          0xB2
#define VK_MEDIA_PLAY_PAUSE    0xB3
#define VK_LAUNCH_MAIL         0xB4
#define VK_LAUNCH_MEDIA_SELECT 0xB5
#define VK_LAUNCH_APP1         0xB6
#define VK_LAUNCH_APP2         0xB7

#endif /* _WIN32_WINNT >= 0x0500 */

			 /*
			  * 0xB8 - 0xB9 : reserved
			  */

#define VK_OEM_1          0xBA   // ';:' for US
#define VK_OEM_PLUS       0xBB   // '+' any country
#define VK_OEM_COMMA      0xBC   // ',' any country
#define VK_OEM_MINUS      0xBD   // '-' any country
#define VK_OEM_PERIOD     0xBE   // '.' any country
#define VK_OEM_2          0xBF   // '/?' for US
#define VK_OEM_3          0xC0   // '`~' for US

			  /*
			   * 0xC1 - 0xC2 : reserved
			   */

#if(_WIN32_WINNT >= 0x0604)

			   /*
				* 0xC3 - 0xDA : Gamepad input
				*/

#define VK_GAMEPAD_A                         0xC3 // reserved
#define VK_GAMEPAD_B                         0xC4 // reserved
#define VK_GAMEPAD_X                         0xC5 // reserved
#define VK_GAMEPAD_Y                         0xC6 // reserved
#define VK_GAMEPAD_RIGHT_SHOULDER            0xC7 // reserved
#define VK_GAMEPAD_LEFT_SHOULDER             0xC8 // reserved
#define VK_GAMEPAD_LEFT_TRIGGER              0xC9 // reserved
#define VK_GAMEPAD_RIGHT_TRIGGER             0xCA // reserved
#define VK_GAMEPAD_DPAD_UP                   0xCB // reserved
#define VK_GAMEPAD_DPAD_DOWN                 0xCC // reserved
#define VK_GAMEPAD_DPAD_LEFT                 0xCD // reserved
#define VK_GAMEPAD_DPAD_RIGHT                0xCE // reserved
#define VK_GAMEPAD_MENU                      0xCF // reserved
#define VK_GAMEPAD_VIEW                      0xD0 // reserved
#define VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON    0xD1 // reserved
#define VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON   0xD2 // reserved
#define VK_GAMEPAD_LEFT_THUMBSTICK_UP        0xD3 // reserved
#define VK_GAMEPAD_LEFT_THUMBSTICK_DOWN      0xD4 // reserved
#define VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT     0xD5 // reserved
#define VK_GAMEPAD_LEFT_THUMBSTICK_LEFT      0xD6 // reserved
#define VK_GAMEPAD_RIGHT_THUMBSTICK_UP       0xD7 // reserved
#define VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN     0xD8 // reserved
#define VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT    0xD9 // reserved
#define VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT     0xDA // reserved

#endif /* _WIN32_WINNT >= 0x0604 */


#define VK_OEM_4          0xDB  //  '[{' for US
#define VK_OEM_5          0xDC  //  '\|' for US
#define VK_OEM_6          0xDD  //  ']}' for US
#define VK_OEM_7          0xDE  //  ''"' for US
#define VK_OEM_8          0xDF

				/*
				 * 0xE0 : reserved
				 */

				 /*
				  * Various extended or enhanced keyboards
				  */
#define VK_OEM_AX         0xE1  //  'AX' key on Japanese AX kbd
#define VK_OEM_102        0xE2  //  "<>" or "\|" on RT 102-key kbd.
#define VK_ICO_HELP       0xE3  //  Help key on ICO
#define VK_ICO_00         0xE4  //  00 key on ICO

#if(WINVER >= 0x0400)
#define VK_PROCESSKEY     0xE5
#endif /* WINVER >= 0x0400 */

#define VK_ICO_CLEAR      0xE6


#if(_WIN32_WINNT >= 0x0500)
#define VK_PACKET         0xE7
#endif /* _WIN32_WINNT >= 0x0500 */

				  /*
				   * 0xE8 : unassigned
				   */

				   /*
					* Nokia/Ericsson definitions
					*/
#define VK_OEM_RESET      0xE9
#define VK_OEM_JUMP       0xEA
#define VK_OEM_PA1        0xEB
#define VK_OEM_PA2        0xEC
#define VK_OEM_PA3        0xED
#define VK_OEM_WSCTRL     0xEE
#define VK_OEM_CUSEL      0xEF
#define VK_OEM_ATTN       0xF0
#define VK_OEM_FINISH     0xF1
#define VK_OEM_COPY       0xF2
#define VK_OEM_AUTO       0xF3
#define VK_OEM_ENLW       0xF4
#define VK_OEM_BACKTAB    0xF5

#define VK_ATTN           0xF6
#define VK_CRSEL          0xF7
#define VK_EXSEL          0xF8
#define VK_EREOF          0xF9
#define VK_PLAY           0xFA
#define VK_ZOOM           0xFB
#define VK_NONAME         0xFC
#define VK_PA1            0xFD
#define VK_OEM_CLEAR      0xFE

					/*
					 * 0xFF : reserved
					 */


#endif /* !NOVIRTUALKEYCODES */

struct KeyEvent
{
	VirtualKey Key;
	struct
	{
		uint16_t Pressed : 1;
	} Flags;
};

enum class MessageType
{
	KeyEvent
};

struct MessageHeader
{
	MessageType MessageType;
	Handle Window;
};

struct Message
{
	MessageHeader Header;
	union
	{
		KeyEvent KeyEvent;
	};
};

#if !defined(EXPORT)
#define SYSTEMCALL(type) extern "C" __declspec(dllimport) type
#else
#define SYSTEMCALL(type) extern "C" __declspec(dllexport) type
#endif

#if !defined(EXPORT)
#define RUNTIMECALL(type) extern "C" __declspec(dllimport) type
#else
#define RUNTIMECALL(type) extern "C" __declspec(dllexport) type
#endif

enum class WaitStatus
{
	None,
	Signaled,
	Timeout,
	Abandoned,
	//Failed conflicts with macro
};

enum class MemoryAllocationType
{
	Commit = 1,
	Reserve = 2,
	CommitReserve = Commit | Reserve,
};

enum class MemoryProtection
{
	PageRead = 1,
	PageWrite = 2,
	PageExecute = 4,
	PageReadWrite = PageRead | PageWrite,
	PageReadExecute = PageRead | PageExecute,
	PageReadWriteExecute = PageRead | PageWrite | PageExecute
};

#define MAX_LOADED_MODULES 8

struct Module
{
	char Name[16];
	void* Address;
};

struct ProcessEnvironmentBlock
{
	uint32_t ProcessId;
	uintptr_t BaseAddress;
	Module LoadedModules[MAX_LOADED_MODULES];
	size_t ModuleIndex;
};

struct ThreadEnvironmentBlock
{
	ThreadEnvironmentBlock* SelfPointer;
	ProcessEnvironmentBlock* PEB;
	ThreadStart ThreadStart;
	void* Arg;
	uint32_t ThreadId;
	uint32_t Error;
};

enum class FilePointerMove
{
	Begin,
	Current,
	End
};

enum class SystemCallResult
{
	Success = 0,
	Failed
};

//Info
SYSTEMCALL(SystemCallResult) GetSystemInfo(SystemInfo& info);
RUNTIMECALL(SystemCallResult) GetProcessInfo(ProcessInfo& info);
SYSTEMCALL(size_t) GetTickCount();

//Process/Thread
SYSTEMCALL(Handle) GetCurrentThread();
RUNTIMECALL(uint64_t) GetCurrentThreadId();
SYSTEMCALL(Handle) CreateThread(size_t stackSize, ThreadStart startAddress, void* arg);
SYSTEMCALL(void) Sleep(const uint32_t milliseconds);
SYSTEMCALL(void) SwitchToThread();
SYSTEMCALL(SystemCallResult) SuspendThread(Handle thread, size_t& prevCount);
SYSTEMCALL(SystemCallResult) ResumeThread(Handle thread, size_t& prevCount);
SYSTEMCALL(SystemCallResult) ExitProcess(const uint32_t exitCode);
SYSTEMCALL(SystemCallResult) ExitThread(const uint32_t exitCode);

RUNTIMECALL(uint32_t) GetLastError();
RUNTIMECALL(void) SetLastError(uint32_t errorCode);

//Semaphores
//Handle CreateSemaphore(size_t initial, size_t maximum, const char* name);
//SYSTEMCALL ReleaseSemaphore(Handle hSemaphore, size_t releaseCount, size_t* previousCount);

//Windows
SYSTEMCALL(Handle) CreateWindow(const char* name);
SYSTEMCALL(SystemCallResult) GetWindowRect(const Handle handle, Rectangle& rect);
SYSTEMCALL(SystemCallResult) GetMessage(Message& message);
SYSTEMCALL(SystemCallResult) PeekMessage(Message& message);
SYSTEMCALL(SystemCallResult) SetScreenBuffer(const void* buffer);

SYSTEMCALL(Handle) CreateFile(const char* path, const GenericAccess access);
SYSTEMCALL(SystemCallResult) ReadFile(const Handle handle, void* buffer, const size_t bufferSize, size_t* bytesRead);
SYSTEMCALL(SystemCallResult) WriteFile(const Handle handle, const void* buffer, const size_t bufferSize, size_t* bytesWritten);
SYSTEMCALL(SystemCallResult) SetFilePointer(const Handle handle, const __int64 position, const FilePointerMove moveType, size_t* newPosition);
SYSTEMCALL(SystemCallResult) CloseFile(const Handle handle);
SYSTEMCALL(SystemCallResult) MoveFile(const char* existingFileName, const char* newFileName);
SYSTEMCALL(SystemCallResult) DeleteFile(const char* fileName);
SYSTEMCALL(SystemCallResult) CreateDirectory(const char* path);

SYSTEMCALL(void*) VirtualAlloc(const void* address, const size_t size, const MemoryAllocationType allocationType, const MemoryProtection protect);

SYSTEMCALL(SystemCallResult) DebugPrint(const char* s);

RUNTIMECALL(Handle) LoadLibrary(const char* lpLibFileName);
RUNTIMECALL(uintptr_t) GetProcAddress(Handle hModule, 
	const char* lpProcName);

class Thread
{
public:
	static Thread* GetCurrent()
	{
		Thread* ret = new Thread();
		ret->m_thread = GetCurrentThread();
		return ret;
	}

	static void Sleep(const uint32_t milliseconds)
	{
		Sleep(milliseconds);
	}

	Thread(size_t stackSize, ThreadStart startAddress, void* arg)
	{
		m_thread = CreateThread(stackSize, startAddress, arg);
	}

	bool IsValid()
	{
		return m_thread != nullptr;
	}

private:
	Thread();
	Handle m_thread;
};

#define MakePtr( cast, ptr, addValue ) (cast)( (uintptr_t)(ptr) + (uintptr_t)(addValue))

enum class DllEntryReason
{
	ProcessAttach,
	ProcessDetach,
	ThreadAttach,
	ThreadDetach
};

//True - LoadLibrary returns handle
//False - DLL is unloaded
typedef uint32_t (*DllMainCall)(Handle hinstDLL, enum DllEntryReason fdwReason);
const char DllMainName[] = "DllMain";
