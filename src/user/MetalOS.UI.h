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
	ResizeEvent, //WM asked the window to change size (maximize or restore); the app calls ResizeWindow with the new size
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

//The client size the WM wants the window to take
struct ResizeEvent
{
	uint16_t Width;
	uint16_t Height;
};

struct Message
{
	MessageHeader Header;
	union
	{
		KeyEvent KeyEvent;
		MouseEvent MouseEvent;
		ResizeEvent ResizeEvent;
	};
};

//How the WM frames a window (AllocWindow): flags
enum class WindowStyle : uint32_t
{
	Framed = 0, //title bar, border, minimize and close, drawn by the WM around the app's client area
	Resizable = 1, //also maximize, which sends a ResizeEvent
	Chromeless = 2, //no frame, fixed in place
	Fullscreen = 4, //no frame, scaled to fill the screen (over the taskbar) while it has focus
};

constexpr WindowStyle operator|(const WindowStyle a, const WindowStyle b)
{
	return static_cast<WindowStyle>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));
}

constexpr bool HasStyle(const WindowStyle style, const WindowStyle flag)
{
	return (static_cast<uint32_t>(style) & static_cast<uint32_t>(flag)) != 0;
}
