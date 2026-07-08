#pragma once

#include <cstdint>
#include "Graphics/Types.h"
#include "user/MetalOS.UI.h"
#include "Lib/SharedRing.h"

//Shared app<->WM IPC protocol. Included by both the usermode WM (server) and the
//MetalOS-WM.dll windowing client. Channels live entirely in shared memory; the kernel
//only moves ids across via the endpoint registry (see MetalOS.Syscalls.h 0x900).
namespace Wm
{
	//Well-known endpoint names.
	static constexpr const char* ControlEndpoint = "wm";     //Post/Poll connection ids
	static constexpr const char* InputEndpoint   = "input";  //kernel->WM input ring id
	static constexpr const char* ScreenEndpoint  = "screen"; //packed (width<<32)|height

	static constexpr uint32_t RequestCapacity = 64;
	static constexpr uint32_t MessageCapacity = 64;

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
	};

	//A duplex channel is one shared region laid out as:
	//   [ChannelHeader][ Request ring (app->WM) ][ Message ring (WM->app) ]
	//The app creates and Init()s it, then Posts its id to ControlEndpoint.
	struct ChannelHeader
	{
		uint32_t ProcessId;
		uint32_t Reserved;
	};

	static constexpr size_t RequestRingOffset = sizeof(ChannelHeader);

	static constexpr size_t MessageRingOffset()
	{
		return RequestRingOffset + SharedRing<Request>::RegionSize(RequestCapacity);
	}

	static constexpr size_t ChannelSize()
	{
		return MessageRingOffset() + SharedRing<Message>::RegionSize(MessageCapacity);
	}

	//Ring views over a mapped channel region
	static inline SharedRing<Request> RequestRing(void* const region)
	{
		return SharedRing<Request>(reinterpret_cast<uint8_t*>(region) + RequestRingOffset);
	}

	static inline SharedRing<Message> MessageRing(void* const region)
	{
		return SharedRing<Message>(reinterpret_cast<uint8_t*>(region) + MessageRingOffset());
	}

	//Called once by the creator (the app) after mapping the region.
	static inline void InitChannel(void* const region, const uint32_t processId)
	{
		ChannelHeader* const header = reinterpret_cast<ChannelHeader*>(region);
		header->ProcessId = processId;
		header->Reserved = 0;
		SharedRing<Request>::Init(reinterpret_cast<uint8_t*>(region) + RequestRingOffset, RequestCapacity);
		SharedRing<Message>::Init(reinterpret_cast<uint8_t*>(region) + MessageRingOffset(), MessageCapacity);
	}
}
