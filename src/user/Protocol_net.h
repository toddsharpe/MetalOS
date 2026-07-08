#pragma once

#include <cstdint>
#include "user/MetalOS.h"
#include "user/Protocol_channel.h"

//MetalOS networking shared ABIs: NetDevice (kernel<->netstack raw-frame device) and NetIpc
//(app<->netstack socket IPC), both below.

//Kernel<->netstack raw-frame device ABI: raw ethernet frames come up over the RX ring
//(published as a grant under RxEndpoint); transmit is the KeNetSend syscall.
namespace NetDevice
{
	static constexpr uint32_t Mtu = 1514;       //max ethernet frame (incl. header, excl. FCS)
	static constexpr uint32_t RxCapacity = 256; //kernel->netstack RX ring depth

	//A raw ethernet frame carried over the RX ring (fixed-size record).
	struct Frame
	{
		uint32_t IfIdx;
		uint16_t Length;
		uint8_t Data[Mtu];
	};

	static constexpr const char* RxEndpoint = "net.rx"; //grant token for the RX ring shm
}

//Shared app<->netstack socket IPC (server MetalOS-NetSvr.exe, client MetalOS-NET.dll). Each
//app opens a duplex channel and posts its grant to "net"; calls are synchronous req/reply.
namespace NetIpc
{
	static constexpr const char* ControlEndpoint = "net"; //apps Post their channel grant here

	static constexpr uint32_t RequestCapacity = 8;
	static constexpr uint32_t ReplyCapacity   = 8;
	static constexpr size_t   MaxPayload      = 1500; //max UDP/ICMP datagram carried inline

	//Socket ops plus IPv4 interface config (netstack owns the config; enumeration stays a
	//kernel syscall).
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

	//Duplex channel: request ring (app->netstack) + reply ring (netstack->app), one matching
	//Reply per Request. Channel::Requests()/Replies()/Size()/Init()/Header, see Protocol_channel.h.
	using Channel = DuplexChannel<Request, Reply, RequestCapacity, ReplyCapacity>;
}
