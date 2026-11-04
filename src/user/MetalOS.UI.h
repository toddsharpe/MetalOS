#pragma once

#include "core_crt/stdint.h"

//Input and window message types only; the kernel's input drivers include this, so no UI or drawing code
typedef uint16_t VirtualKey;

enum class MessageType : uint16_t
{
	KeyEvent,
	MouseEvent,
	PaintEvent,
	CloseEvent, //WM asked the window to close; an app whose last window closes should exit
};

struct MessageHeader
{
	MessageType MessageType;
	uint16_t Window; //the window the WM sent it for (see WindowId in MetalOS.h); 0 in the kernel's input ring
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

	//Equal buttons on consecutive mouse events make the later one a move
	bool operator==(const MouseButtonState& other) const
	{
		return LeftPressed == other.LeftPressed && RightPressed == other.RightPressed;
	}
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
