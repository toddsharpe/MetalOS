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
	static constexpr uint32_t MessageCapacity = 64;
	static constexpr uint32_t MaxTitle = 32; //window title length (for the taskbar)

	//app -> WM request opcodes
	enum class Op : uint32_t
	{
		AllocWindow,
		MoveWindow,
	};

	//Fixed-size app -> WM request record
	struct Request
	{
		Op       Code;
		uint32_t WindowId;         //client-assigned, unique per connection
		Graphics::Rectangle Rect;  //AllocWindow / MoveWindow position+size
		uint64_t SurfaceGrant;     //AllocWindow: capability token for the shared surface
		uint32_t Width;            //AllocWindow: surface dimensions
		uint32_t Height;
		char     Title[MaxTitle];  //AllocWindow: window title (null-terminated), for the taskbar
		bool     ShowTitle;        //AllocWindow: draw WM chrome (title/minimize/close); false = chromeless
	};

	//Duplex channel: request ring (app->WM) + reply ring carrying the async input Message
	//stream (WM->app). Channel::Requests()/Replies()/Size()/Init()/Header, see Protocol_channel.h.
	using Channel = DuplexChannel<Request, Message, RequestCapacity, MessageCapacity>;
}
