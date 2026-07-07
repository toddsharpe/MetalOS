#include "user/MetalOS.h"
#include "NetDevice.h"
#include "Lib/SharedRing.h"
#include "Lib/Buffer.h"
#include "Assert.h"
#include <stdio.h>
#include "user/CRT.cpp" //operator new/delete (malloc/free)

//Pure-virtual fallback (NetDriver is abstract); never called since HostDriver overrides all.
int __cdecl _purecall(void) { Assert(false); return 0; }

#include "Net/Net.h"
#include "Net/NetIf.h"
#include "Net/Packet.h"
#include "Net/KSocket.h"
#include "Net/HostDriver.h"
#include "user/Protocol_net.h"
#include "Lib/ByteSwap.h"

//netstack.exe -- the MetalOS usermode TCP/IP stack.
//
//The kernel owns only the NIC + raw frame I/O (Kernel_net.cpp). This process claims
//the kernel->netstack RX ring and the NIC MAC (published in the endpoint registry),
//runs ARP/IPv4/ICMP/UDP/routing over the frames, and transmits via the NetSend syscall
//(HostDriver). Apps reach it over IPC (Protocol_net.h) -- serviced in the main loop.

//The TCP/IP stack, compiled here as a unity build (the sources #include each other as
//"Net/X.h", which resolves relative to this file).
#include "Net/Util.cpp"
#include "Net/Checksum.cpp"
#include "Net/Net.cpp"
#include "Net/ArpCache.cpp"
#include "Net/Arp.cpp"
#include "Net/Ethernet.cpp"
#include "Net/IPv4.cpp"
#include "Net/Icmp.cpp"
#include "Net/Udp.cpp"
#include "Net/Router.cpp"
#include "Net/Socket.cpp"
#include "Net/Sockets.cpp"

namespace
{
	//Claim a kernel-published shared region by endpoint name (grant token -> handle -> map).
	void* ClaimRegion(const char* const endpoint)
	{
		uint64_t token = 0;
		if (LookupEndpoint(endpoint, &token) != SyscallResult::Success)
			return nullptr;

		Handle handle = nullptr;
		if (ClaimHandle(token, &handle) != SyscallResult::Success)
			return nullptr;

		void* region = nullptr;
		if (MapSharedMemory((HSharedMem)handle, &region) != SyscallResult::Success)
			return nullptr;

		return region;
	}

	/*
	 * App IPC. Each connected app has a duplex channel (Protocol_net.h); we assign it
	 * per-connection socket ids backed by netstack KSockets. sockaddr_in is network
	 * order, the stack is host order, so convert at this boundary (ByteSwap == hton/ntoh).
	 */
	using read_t = Net::Socket::read_t;

	Net::endpoint_t ToEndpoint(const sockaddr_in& sa)
	{
		Net::endpoint_t ep = {};
		ep.addr.addr = ByteSwap<uint32_t>(sa.sin_addr.s_addr);
		ep.port = ByteSwap<uint16_t>(sa.sin_port);
		return ep;
	}

	void FromEndpoint(const Net::endpoint_t& ep, sockaddr_in& sa)
	{
		sa.sin_family = AF_INET;
		sa.sin_port = ByteSwap<uint16_t>(ep.port);
		sa.sin_addr.s_addr = ByteSwap<uint32_t>(ep.addr.addr);
		memset(sa.sin_zero, 0, sizeof(sa.sin_zero));
	}

	//The stack's single interface. Set up in main() and owned by the netstack; the IPv4
	//config (addr/subnet/gateway) is set/read via the interface-config IPC ops below.
	//in_addr is network order; the stack's ipv4_addr_t is host order (ByteSwap converts).
	Net::NetIf* g_netif = nullptr;

	Net::ipv4_addr_t ToIpv4(const in_addr& addr) { return { ByteSwap<uint32_t>(addr.s_addr) }; }
	in_addr FromIpv4(const Net::ipv4_addr_t& ip) { in_addr a = {}; a.s_addr = ByteSwap<uint32_t>(ip.addr); return a; }

	constexpr size_t MaxConns = 16;
	constexpr size_t MaxSocketsPerConn = 32;

	struct Connection
	{
		void* Region;
		KSocket* Sockets[MaxSocketsPerConn];
	};

	Connection g_conns[MaxConns] = {};
	size_t g_connCount = 0;

	void AcceptConnections()
	{
		uint64_t token = 0;
		while (PollEndpoint(NetIpc::ControlEndpoint, &token) == SyscallResult::Success)
		{
			if (g_connCount >= MaxConns)
				break;

			Handle h = nullptr;
			if (ClaimHandle(token, &h) != SyscallResult::Success)
				continue;
			void* region = nullptr;
			if (MapSharedMemory((HSharedMem)h, &region) != SyscallResult::Success)
			{
				CloseHandle(h);
				continue;
			}
			g_conns[g_connCount].Region = region;
			g_connCount++;
		}
	}

	KSocket* GetSock(Connection& conn, const uint32_t id)
	{
		if (id == 0 || id > MaxSocketsPerConn)
			return nullptr;
		return conn.Sockets[id - 1];
	}

	void HandleRequest(Connection& conn, const NetIpc::Request& req, NetIpc::Reply& reply)
	{
		reply.Result = SyscallResult::Failed;

		switch (req.Code)
		{
		case NetIpc::Op::Create:
		{
			KSocket* s = SocketCreate(req.Type, req.Proto);
			if (!s)
				return;
			for (uint32_t i = 0; i < MaxSocketsPerConn; i++)
			{
				if (!conn.Sockets[i])
				{
					conn.Sockets[i] = s;
					reply.Socket = i + 1;
					reply.Result = SyscallResult::Success;
					return;
				}
			}
			SocketClose(*s); //table full
			return;
		}

		case NetIpc::Op::Bind:
		{
			KSocket* s = GetSock(conn, req.Socket);
			if (!s) { reply.Result = SyscallResult::InvalidHandle; return; }
			reply.Result = SocketBind(*s, ToEndpoint(req.Addr)) ? SyscallResult::Success : SyscallResult::Failed;
			return;
		}

		case NetIpc::Op::Connect:
		{
			KSocket* s = GetSock(conn, req.Socket);
			if (!s) { reply.Result = SyscallResult::InvalidHandle; return; }
			reply.Result = SocketConnect(*s, ToEndpoint(req.Addr)) ? SyscallResult::Success : SyscallResult::Failed;
			return;
		}

		case NetIpc::Op::SendTo:
		{
			KSocket* s = GetSock(conn, req.Socket);
			if (!s) { reply.Result = SyscallResult::InvalidHandle; return; }
			const size_t len = req.Length < NetIpc::MaxPayload ? req.Length : NetIpc::MaxPayload;
			reply.Result = SocketSend(*s, ToEndpoint(req.Addr), req.Data, len) ? SyscallResult::Success : SyscallResult::Failed;
			return;
		}

		case NetIpc::Op::Send:
		{
			KSocket* s = GetSock(conn, req.Socket);
			if (!s || !s->Connected) { reply.Result = SyscallResult::InvalidHandle; return; }
			const size_t len = req.Length < NetIpc::MaxPayload ? req.Length : NetIpc::MaxPayload;
			reply.Result = SocketSend(*s, s->Peer, req.Data, len) ? SyscallResult::Success : SyscallResult::Failed;
			return;
		}

		case NetIpc::Op::RecvFrom:
		case NetIpc::Op::Recv:
		{
			KSocket* s = GetSock(conn, req.Socket);
			if (!s) { reply.Result = SyscallResult::InvalidHandle; return; }
			Net::endpoint_t src = {};
			size_t bytes = 0;
			if (SocketRecv(*s, src, reply.Data, NetIpc::MaxPayload, bytes) == read_t::Success)
			{
				reply.Length = (uint32_t)bytes;
				if (req.Code == NetIpc::Op::RecvFrom)
					FromEndpoint(src, reply.From);
				reply.Result = SyscallResult::Success;
			}
			else
			{
				reply.Result = SyscallResult::Failed; //empty: the client retries until timeout
			}
			return;
		}

		case NetIpc::Op::Close:
		{
			KSocket* s = GetSock(conn, req.Socket);
			if (s)
			{
				SocketClose(*s);
				conn.Sockets[req.Socket - 1] = nullptr;
			}
			reply.Result = SyscallResult::Success;
			return;
		}

		//Interface config: index in req.Socket. Single NIC (index 0).
		case NetIpc::Op::GetInterfaceIp:
		{
			if (req.Socket != 0 || !g_netif) { reply.Result = SyscallResult::InvalidArg; return; }
			reply.Ip = FromIpv4(g_netif->ipv4.addr);
			reply.Subnet = FromIpv4(g_netif->ipv4.subnet_mask);
			reply.Result = SyscallResult::Success;
			return;
		}

		case NetIpc::Op::GetGateway:
		{
			if (req.Socket != 0 || !g_netif) { reply.Result = SyscallResult::InvalidArg; return; }
			reply.Gateway = FromIpv4(g_netif->ipv4.gateway);
			reply.Result = SyscallResult::Success;
			return;
		}

		case NetIpc::Op::SetInterfaceIp:
		{
			if (req.Socket != 0 || !g_netif) { reply.Result = SyscallResult::InvalidArg; return; }
			g_netif->ipv4.addr = ToIpv4(req.Ip);
			g_netif->ipv4.subnet_mask = ToIpv4(req.Subnet);
			reply.Result = SyscallResult::Success;
			return;
		}

		case NetIpc::Op::SetGateway:
		{
			if (req.Socket != 0 || !g_netif) { reply.Result = SyscallResult::InvalidArg; return; }
			g_netif->ipv4.gateway = ToIpv4(req.Ip);
			reply.Result = SyscallResult::Success;
			return;
		}

		}
	}

	void ProcessRequests()
	{
		for (size_t i = 0; i < g_connCount; i++)
		{
			if (!g_conns[i].Region)
				continue;

			SharedRing<NetIpc::Request> reqRing = NetIpc::RequestRing(g_conns[i].Region);
			NetIpc::Request req;
			while (reqRing.Dequeue(req))
			{
				NetIpc::Reply reply = {};
				HandleRequest(g_conns[i], req, reply);
				NetIpc::ReplyRing(g_conns[i].Region).Enqueue(reply);
			}
		}
	}
}

int main(int argc, char** argv)
{
	InterfaceInfo ifaces[4] = {};
	size_t ifaceCount = 0;
	AssertSuccess(GetInterfaces(ifaces, 4, &ifaceCount));
	Assert(ifaceCount == 1);

	//Claim the kernel->netstack RX ring of raw ethernet frames.
	void* const rxRegion = ClaimRegion(NetDevice::RxEndpoint);
	Assert(rxRegion);

	//Build the single interface over the host NIC and register it for routing.
	HostDriver driver(ifaces[0]);
	Net::NetIf net_if(driver, Net::l2_t::Ethernet);
	net_if.Init();
	Net::AddNetIf(net_if);
	g_netif = &net_if; //the interface-config IPC ops read/write this

	//IPv4 config starts unset; dhcp.exe configures it via SetInterfaceIp/SetGateway (IPC).
	DebugPrintf("netstack: up rx=%016x mac=%02x:%02x:%02x:%02x:%02x:%02x\n", (uint64_t)rxRegion,
		ifaces[0].mac[5], ifaces[0].mac[4], ifaces[0].mac[3], ifaces[0].mac[2], ifaces[0].mac[1], ifaces[0].mac[0]);

	SharedRing<NetDevice::Frame> rx(rxRegion);

	while (true)
	{
		//Drain received frames up the stack.
		NetDevice::Frame frame;
		while (rx.Dequeue(frame))
		{
			Net::Packet packet(Buffer(frame.Data, frame.Length), frame.Length);
			Net::get_l2(net_if.l2).Receive(net_if, packet);
		}

		//Service app socket IPC: accept new connections, process pending requests.
		AcceptConnections();
		ProcessRequests();

		//Flush any ARP-pending transmits now that replies may have arrived.
		net_if.PostDispatch();

		Sleep(1);
	}

	return 0;
}
