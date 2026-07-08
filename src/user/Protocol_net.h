#pragma once

#include <cstdint>
#include "user/MetalOS.Types.h"
#include "Lib/SharedRing.h"

//Shared app<->netstack IPC protocol. Included by both MetalOS-NetSvr.exe (server) and the
//MetalOS-NET lib (the socket API). Each app opens one duplex channel (request ring
//app->netstack, reply ring netstack->app) in shared memory and posts its channel grant
//token to the "net" control endpoint. Calls are synchronous: the client enqueues one
//Request and polls the reply ring for the matching Reply.
namespace NetIpc
{
	static constexpr const char* ControlEndpoint = "net"; //apps Post their channel grant here

	static constexpr uint32_t RequestCapacity = 8;
	static constexpr uint32_t ReplyCapacity   = 8;
	static constexpr size_t   MaxPayload      = 1500; //max UDP/ICMP datagram carried inline

	//Socket ops plus IPv4 interface config. Hardware enumeration (GetInterfaces) stays a
	//kernel syscall; the netstack owns the interface's IPv4 config, so Get/SetInterfaceIp
	//and SetGateway are IPC ops here.
	enum class Op : uint32_t
	{
		Create, Bind, Connect, SendTo, Send, RecvFrom, Recv, Close,
		GetInterfaceIp, SetInterfaceIp, GetGateway, SetGateway,
	};

	//Fixed-size app -> netstack request.
	struct Request
	{
		Op       Code;
		uint32_t Socket;   //socket id (netstack-assigned) for socket ops; interface index for config ops
		int32_t  Af;       //Create
		int32_t  Type;
		int32_t  Proto;
		sockaddr_in Addr;  //Bind / Connect / SendTo destination
		uint32_t Length;   //SendTo / Send payload length
		milli_t  Timeout;  //RecvFrom / Recv
		in_addr  Ip;       //SetInterfaceIp addr / SetGateway gateway (network order)
		in_addr  Subnet;   //SetInterfaceIp subnet (network order)
		uint8_t  Data[MaxPayload]; //SendTo / Send payload
	};

	//Fixed-size netstack -> app reply.
	struct Reply
	{
		SyscallResult Result;
		uint32_t Socket;   //Create: assigned socket id
		sockaddr_in From;  //RecvFrom: source
		uint32_t Length;   //RecvFrom: payload length
		in_addr  Ip;       //GetInterfaceIp: addr (network order)
		in_addr  Subnet;   //GetInterfaceIp: subnet (network order)
		in_addr  Gateway;  //GetGateway: gateway (network order)
		uint8_t  Data[MaxPayload]; //RecvFrom payload
	};

	//A duplex channel is one shared region: [ChannelHeader][request ring][reply ring].
	struct ChannelHeader
	{
		uint32_t ProcessId;
		uint32_t Reserved;
	};

	static constexpr size_t RequestRingOffset = sizeof(ChannelHeader);

	static constexpr size_t ReplyRingOffset()
	{
		return RequestRingOffset + SharedRing<Request>::RegionSize(RequestCapacity);
	}

	static constexpr size_t ChannelSize()
	{
		return ReplyRingOffset() + SharedRing<Reply>::RegionSize(ReplyCapacity);
	}

	static inline SharedRing<Request> RequestRing(void* const region)
	{
		return SharedRing<Request>(reinterpret_cast<uint8_t*>(region) + RequestRingOffset);
	}

	static inline SharedRing<Reply> ReplyRing(void* const region)
	{
		return SharedRing<Reply>(reinterpret_cast<uint8_t*>(region) + ReplyRingOffset());
	}

	//Called once by the creator (the app) after mapping the region.
	static inline void InitChannel(void* const region, const uint32_t processId)
	{
		ChannelHeader* const header = reinterpret_cast<ChannelHeader*>(region);
		header->ProcessId = processId;
		header->Reserved = 0;
		SharedRing<Request>::Init(reinterpret_cast<uint8_t*>(region) + RequestRingOffset, RequestCapacity);
		SharedRing<Reply>::Init(reinterpret_cast<uint8_t*>(region) + ReplyRingOffset(), ReplyCapacity);
	}
}
