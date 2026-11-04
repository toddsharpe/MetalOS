#pragma once

#include "core_crt/stdint.h"

//Input and window message types only; the kernel's input drivers include this, so no UI or drawing code
typedef uint16_t VirtualKey;

enum class MessageType
{
	KeyEvent,
	MouseEvent,
	PaintEvent,
	CloseEvent, //WM asked the window to close; the app should exit
};

struct MessageHeader
{
	MessageType MessageType;
};

struct KeyEvent
{
	VirtualKey Key;
	struct
	{
		uint16_t Pressed : 1;
	} Flags;
};

struct MouseButtonState
{
	uint8_t LeftPressed : 1;
	uint8_t RightPressed : 1;
};

//The kernel reports the HID position (0 to 32767 across the screen); the WM hands apps window coordinates, negative left of or above the window
struct MouseEvent
{
	MouseButtonState Buttons;
	int16_t XPosition;
	int16_t YPosition;
};

struct Message
{
	MessageHeader Header;
	union
	{
		KeyEvent KeyEvent;
		MouseEvent MouseEvent;
	};
};
