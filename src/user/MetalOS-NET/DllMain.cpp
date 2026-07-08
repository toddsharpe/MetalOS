#include "user/MetalOS.h"
#include "user/Protocol_net.h"
#include "user/Ipc.h"
#include <cstring>

//MetalOS-NET.dll -- the socket API, backed by IPC to the usermode netstack process.
//Each process opens one duplex channel to netstack (Protocol_net.h) and posts its grant
//to the "net" control endpoint. Calls are synchronous request/reply; RecvFrom polls.

namespace
{
	bool g_connected = false;
	void* g_channel = nullptr;

	bool EnsureConnected()
	{
		if (g_connected)
			return true;

		//Open a channel to netstack (create + Init + grant over the "net" endpoint).
		void* const region = IpcConnect<NetIpc::Channel>(NetIpc::ControlEndpoint);
		if (!region)
			return false;

		g_channel = region;
		g_connected = true;
		return true;
	}

	//Synchronous request/reply: enqueue one request, poll for its reply. Only one
	//request is outstanding at a time, so replies map 1:1 to requests in order.
	bool Call(const NetIpc::Request& req, NetIpc::Reply& reply)
	{
		if (!EnsureConnected())
			return false;

		NetIpc::Channel::Requests(g_channel).Enqueue(req);
		SharedRing<NetIpc::Reply> replies = NetIpc::Channel::Replies(g_channel);

		//Bounded wait so a dead/stuck netstack fails the call instead of hanging forever.
		const milli_t start = GetTickCount();
		while (!replies.Dequeue(reply))
		{
			if (GetTickCount() - start >= 5000)
			{
				return false;
			}
			Sleep(1);
		}
		return true;
	}

	uint32_t ToId(const HSocket sock) { return (uint32_t)(uintptr_t)sock; }
}

extern "C" HSocket SocketCreate(int af, int type, int protocol)
{
	if (af != AF_INET)
		return INVALID_SOCKET;

	const NetIpc::Request req =
	{
		.Code = NetIpc::Op::Create,
		.Af = af,
		.Type = type,
		.Proto = protocol,
	};

	NetIpc::Reply reply = {};
	if (!Call(req, reply) || reply.Result != SyscallResult::Success)
		return INVALID_SOCKET;
	return (HSocket)(uintptr_t)reply.Socket;
}

extern "C" SyscallResult SocketBind(HSocket sock, const sockaddr_in* addr)
{
	if (!addr)
		return SyscallResult::InvalidPointer;

	const NetIpc::Request req =
	{
		.Code = NetIpc::Op::Bind,
		.Socket = ToId(sock),
		.Addr = *addr,
	};

	NetIpc::Reply reply = {};
	return Call(req, reply) ? reply.Result : SyscallResult::Failed;
}

extern "C" SyscallResult SocketConnect(HSocket sock, const sockaddr_in* peer)
{
	if (!peer)
		return SyscallResult::InvalidPointer;

	const NetIpc::Request req =
	{
		.Code = NetIpc::Op::Connect,
		.Socket = ToId(sock),
		.Addr = *peer,
	};

	NetIpc::Reply reply = {};
	return Call(req, reply) ? reply.Result : SyscallResult::Failed;
}

extern "C" SyscallResult SocketSendTo(HSocket sock, const void* buf, size_t len, const sockaddr_in* to)
{
	if (!buf || !to)
		return SyscallResult::InvalidPointer;
	if (!len || len > NetIpc::MaxPayload)
		return SyscallResult::InvalidArg;

	NetIpc::Request req = {};
	req.Code = NetIpc::Op::SendTo;
	req.Socket = ToId(sock);
	req.Addr = *to;
	req.Length = (uint32_t)len;
	memcpy(req.Data, buf, len);

	NetIpc::Reply reply = {};
	return Call(req, reply) ? reply.Result : SyscallResult::Failed;
}

extern "C" SyscallResult SocketSend(HSocket sock, const void* buf, size_t len)
{
	if (!buf)
		return SyscallResult::InvalidPointer;
	if (!len || len > NetIpc::MaxPayload)
		return SyscallResult::InvalidArg;

	NetIpc::Request req = {};
	req.Code = NetIpc::Op::Send;
	req.Socket = ToId(sock);
	req.Length = (uint32_t)len;
	memcpy(req.Data, buf, len);

	NetIpc::Reply reply = {};
	return Call(req, reply) ? reply.Result : SyscallResult::Failed;
}

//Blocking recv: netstack replies immediately (data or empty); we poll until data or
//the timeout elapses (timeoutMs == 0 polls once).
static SyscallResult RecvImpl(HSocket sock, void* buf, size_t maxLen, size_t* bytesRecv, sockaddr_in* from, milli_t timeoutMs, bool wantFrom)
{
	if (!buf)
		return SyscallResult::InvalidPointer;

	const milli_t start = GetTickCount();
	for (;;)
	{
		const NetIpc::Request req =
		{
			.Code = wantFrom ? NetIpc::Op::RecvFrom : NetIpc::Op::Recv,
			.Socket = ToId(sock),
			.Timeout = timeoutMs,
		};

		NetIpc::Reply reply = {};
		if (!Call(req, reply))
			return SyscallResult::Failed;

		if (reply.Result == SyscallResult::Success)
		{
			const size_t n = reply.Length < maxLen ? reply.Length : maxLen;
			memcpy(buf, reply.Data, n);
			if (bytesRecv)
				*bytesRecv = n;
			if (from && wantFrom)
				*from = reply.From;
			return SyscallResult::Success;
		}

		//No datagram yet.
		if (timeoutMs == 0 || GetTickCount() - start >= timeoutMs)
			return SyscallResult::Failed;
		Sleep(5);
	}
}

extern "C" SyscallResult SocketRecvFrom(HSocket sock, void* buf, size_t maxLen, size_t* bytesRecv, sockaddr_in* from, milli_t timeoutMs)
{
	return RecvImpl(sock, buf, maxLen, bytesRecv, from, timeoutMs, true);
}

extern "C" SyscallResult SocketRecv(HSocket sock, void* buf, size_t maxLen, size_t* bytesRecv, milli_t timeoutMs)
{
	return RecvImpl(sock, buf, maxLen, bytesRecv, nullptr, timeoutMs, false);
}

extern "C" SyscallResult SocketClose(HSocket sock)
{
	const NetIpc::Request req =
	{
		.Code = NetIpc::Op::Close,
		.Socket = ToId(sock),
	};

	NetIpc::Reply reply = {};
	return Call(req, reply) ? reply.Result : SyscallResult::Failed;
}

//Interface IPv4 config. Enumeration (GetInterfaces) is a kernel syscall (MetalOS-RT); the IPv4
//config is owned by netstack, so these are IPC. Single NIC: index is passed through.
extern "C" SyscallResult GetInterfaceIp(uint32_t index, in_addr* addr, in_addr* subnet)
{
	if (!addr || !subnet)
		return SyscallResult::InvalidPointer;

	const NetIpc::Request req =
	{
		.Code = NetIpc::Op::GetInterfaceIp,
		.Socket = index,
	};

	NetIpc::Reply reply = {};
	if (!Call(req, reply) || reply.Result != SyscallResult::Success)
		return SyscallResult::Failed;

	*addr = reply.Ip;
	*subnet = reply.Subnet;
	return SyscallResult::Success;
}

extern "C" SyscallResult GetGateway(uint32_t index, in_addr* gateway)
{
	if (!gateway)
		return SyscallResult::InvalidPointer;

	const NetIpc::Request req =
	{
		.Code = NetIpc::Op::GetGateway,
		.Socket = index,
	};

	NetIpc::Reply reply = {};
	if (!Call(req, reply) || reply.Result != SyscallResult::Success)
		return SyscallResult::Failed;

	*gateway = reply.Gateway;
	return SyscallResult::Success;
}

extern "C" SyscallResult SetInterfaceIp(uint32_t index, const in_addr* addr, const in_addr* subnet)
{
	if (!addr || !subnet)
		return SyscallResult::InvalidPointer;

	const NetIpc::Request req =
	{
		.Code = NetIpc::Op::SetInterfaceIp,
		.Socket = index,
		.Ip = *addr,
		.Subnet = *subnet,
	};

	NetIpc::Reply reply = {};
	return Call(req, reply) ? reply.Result : SyscallResult::Failed;
}

extern "C" SyscallResult SetGateway(uint32_t index, const in_addr* gateway)
{
	if (!gateway)
		return SyscallResult::InvalidPointer;

	const NetIpc::Request req =
	{
		.Code = NetIpc::Op::SetGateway,
		.Socket = index,
		.Ip = *gateway,
	};

	NetIpc::Reply reply = {};
	return Call(req, reply) ? reply.Result : SyscallResult::Failed;
}

bool DllMain(HModule handle, DllReason reason)
{
	//Do nothing
	return true;
}
