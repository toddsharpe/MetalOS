#pragma once

//Include UI libraries
#include "UI/Window.h"
#include "UI/Button.h"
#include "UI/Label.h"

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

struct MouseEvent
{
	MouseButtonState Buttons;
	uint16_t XPosition;
	uint16_t YPosition;
};

struct PaintEvent
{
	Graphics::Rectangle Region;
};

struct Message
{
	MessageHeader Header;
	union
	{
		KeyEvent KeyEvent;
		MouseEvent MouseEvent;
		PaintEvent PaintEvent;
	};
};
