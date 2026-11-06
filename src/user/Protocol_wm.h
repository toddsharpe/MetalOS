#pragma once

#include <cstdint>
#include "Graphics/Types.h"
#include "user/MetalOS.UI.h"
#include "user/Protocol_channel.h"

//Shared app<->WM IPC protocol (server = usermode WM, client = MetalOS-WM.dll). Channels live
//in shared memory; the kernel only moves ids via the endpoint registry.
namespace Wm
{
	//Well-known endpoint names.
	static constexpr const char* ControlEndpoint = "wm";     //Post/Poll connection ids
	static constexpr const char* InputEndpoint   = "input";  //kernel->WM input ring id
	static constexpr const char* ScreenEndpoint  = "screen"; //packed (width<<32)|height

	static constexpr uint32_t RequestCapacity = 64;
	static constexpr uint32_t MessageCapacity = 512; //holds a full drain of the kernel's input ring, so a typed burst reaches the app whole
	static constexpr uint32_t MaxTitle = 32; //window title length (for the taskbar)

	//app -> WM request opcodes
	enum class Op : uint32_t
	{
		AllocWindow,
		ResizeWindow, //a new surface for the window, after a ResizeEvent
	};

	//A window's 8x8 icon as the app sends it: two bits per cell, column n at bit 2n (0 transparent, 1 to 3 a palette color); all zero means the app sent none
	struct IconBits
	{
		uint16_t Rows[8];
		Graphics::Color Palette[3];
	};

	//Fixed-size app -> WM request record
	struct Request
	{
		Op       Code;
		uint16_t WindowId;         //client-assigned, unique per connection, nonzero; messages for the window carry it in Header.Window
		Graphics::Rectangle Rect;  //client area position and size (ResizeWindow: only the size)
		uint64_t SurfaceGrant;     //capability token for the shared surface (Rect gives its size)
		char     Title[MaxTitle];  //AllocWindow: window title (null-terminated), for the title bar and taskbar
		WindowStyle Style;         //AllocWindow: how the WM frames it
		IconBits Icon;             //AllocWindow: shown in the title bar, taskbar and window switcher
	};

	//Duplex channel: request ring (app->WM) + reply ring carrying the async input Message
	//stream (WM->app). Channel::Requests()/Replies()/Size()/Init()/Header, see Protocol_channel.h.
	using Channel = DuplexChannel<Request, Message, RequestCapacity, MessageCapacity>;
}
