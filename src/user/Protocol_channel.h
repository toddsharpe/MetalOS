#pragma once

#include <cstdint>
#include "Lib/SharedRing.h"

//A duplex IPC channel in one shared region: [Header][request ring (client->server)][reply
//ring (server->client)]. Parameterized on the two record types + ring depths per protocol.
template <typename TRequest, typename TReply, uint32_t RequestCapacity, uint32_t ReplyCapacity>
struct DuplexChannel
{
	struct Header
	{
		uint32_t ProcessId;
		uint32_t Reserved;
	};

	static constexpr size_t RequestRingOffset = sizeof(Header);

	static constexpr size_t ReplyRingOffset()
	{
		return RequestRingOffset + SharedRing<TRequest>::RegionSize(RequestCapacity);
	}

	//Total bytes of shared memory the channel needs.
	static constexpr size_t Size()
	{
		return ReplyRingOffset() + SharedRing<TReply>::RegionSize(ReplyCapacity);
	}

	//Ring views over a mapped channel region.
	static inline SharedRing<TRequest> Requests(void* const region)
	{
		return SharedRing<TRequest>(reinterpret_cast<uint8_t*>(region) + RequestRingOffset);
	}

	static inline SharedRing<TReply> Replies(void* const region)
	{
		return SharedRing<TReply>(reinterpret_cast<uint8_t*>(region) + ReplyRingOffset());
	}

	//Called once by the creator (the client) after mapping the region.
	static inline void Init(void* const region, const uint32_t processId)
	{
		Header* const header = reinterpret_cast<Header*>(region);
		header->ProcessId = processId;
		header->Reserved = 0;

		SharedRing<TRequest>::Init(reinterpret_cast<uint8_t*>(region) + RequestRingOffset, RequestCapacity);
		SharedRing<TReply>::Init(reinterpret_cast<uint8_t*>(region) + ReplyRingOffset(), ReplyCapacity);
	}
};
