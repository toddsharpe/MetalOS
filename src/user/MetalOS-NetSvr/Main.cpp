#include "user/MetalOS.h"
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
#include "user/Ipc.h"
#include "Lib/ByteSwap.h"

//MetalOS-NetSvr.exe -- usermode TCP/IP stack. Claims the kernel RX ring + NIC MAC, runs
//ARP/IPv4/ICMP/UDP/routing, transmits via NetSend; apps reach it over IPC (Protocol_net.h).

//The TCP/IP stack, compiled here as a unity build (the sources #include each other as
//"Net/X.h", which resolves relative to this file).
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
	//App IPC: each app's duplex channel gets per-connection socket ids backed by KSockets.
	//sockaddr_in is network order, the stack host order, so convert here (ByteSwap == hton/ntoh).
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

	//The stack's single interface, owned by the netstack; its IPv4 config is set/read via the
	//interface-config IPC ops below. in_addr is network order, ipv4_addr_t host (ByteSwap).
	Net::NetIf* g_netif = nullptr;

	Net::ipv4_addr_t ToIpv4(const in_addr& addr) { return { ByteSwap<uint32_t>(addr.s_addr) }; }
	in_addr FromIpv4(const Net::ipv4_addr_t& ip) { in_addr a = {}; a.s_addr = ByteSwap<uint32_t>(ip.addr); return a; }

	constexpr size_t MaxConns = 16;
	constexpr size_t MaxSocketsPerConn = 32;

	struct Connection
	{
		void* Region;      //mapped channel; null when the slot is free
		HSharedMem Handle; //our handle to the channel; CloseHandle releases it
		KSocket* Sockets[MaxSocketsPerConn];
	};

	Connection g_conns[MaxConns] = {};

	Connection* FreeSlot()
	{
		for (Connection& conn : g_conns)
		{
			if (!conn.Region)
				return &conn;
		}
		return nullptr;
	}

	void AcceptConnections()
	{
		//Only take a token when a slot is free; the rest stay queued on the endpoint
		uint64_t token = 0;
		for (Connection* slot = FreeSlot(); slot && PollEndpoint(NetIpc::ControlEndpoint, &token) == SyscallResult::Success; slot = FreeSlot())
		{
			HSharedMem handle = nullptr;
			void* region = nullptr;
			if (!IpcClaim(token, handle, region))
				continue;

			*slot = {};
			slot->Region = region;
			slot->Handle = handle;
		}
	}

	//Close the sockets and channel of apps that exited (clean or crash), freeing the slot and their ports
	void ReapDeadConnections()
	{
		for (Connection& conn : g_conns)
		{
			if (!conn.Region)
				continue;

			const uint32_t pid = reinterpret_cast<NetIpc::Channel::Header*>(conn.Region)->ProcessId;
			if (pid == 0 || IsProcessAlive(pid))
				continue;

			for (KSocket*& socket : conn.Sockets)
			{
				if (socket)
					SocketClose(*socket);
				socket = nullptr;
			}
			CloseHandle((Handle)conn.Handle);
			conn.Region = nullptr;
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

		//Interface config: index in req.Socket. Single NIC (index 0).
		case NetIpc::Op::GetInterfaceIp:
		{
			if (req.Socket != 0) { reply.Result = SyscallResult::InvalidArg; return; }
			reply.Ip = FromIpv4(g_netif->ipv4.addr);
			reply.Subnet = FromIpv4(g_netif->ipv4.subnet_mask);
			reply.Result = SyscallResult::Success;
			return;
		}

		case NetIpc::Op::GetGateway:
		{
			if (req.Socket != 0) { reply.Result = SyscallResult::InvalidArg; return; }
			reply.Ip = FromIpv4(g_netif->ipv4.gateway);
			reply.Result = SyscallResult::Success;
			return;
		}

		case NetIpc::Op::SetInterfaceIp:
		{
			if (req.Socket != 0) { reply.Result = SyscallResult::InvalidArg; return; }
			g_netif->ipv4.addr = ToIpv4(req.Ip);
			g_netif->ipv4.subnet_mask = ToIpv4(req.Subnet);
			reply.Result = SyscallResult::Success;
			return;
		}

		case NetIpc::Op::SetGateway:
		{
			if (req.Socket != 0) { reply.Result = SyscallResult::InvalidArg; return; }
			g_netif->ipv4.gateway = ToIpv4(req.Ip);
			reply.Result = SyscallResult::Success;
			return;
		}

		default:
			break;
		}

		//The remaining ops act on one of the connection's sockets
		KSocket* const s = GetSock(conn, req.Socket);
		if (!s)
		{
			reply.Result = (req.Code == NetIpc::Op::Close) ? SyscallResult::Success : SyscallResult::InvalidHandle;
			return;
		}

		switch (req.Code)
		{
		case NetIpc::Op::Bind:
			SocketBind(*s, ToEndpoint(req.Addr));
			reply.Result = SyscallResult::Success;
			return;

		case NetIpc::Op::Connect:
			SocketConnect(*s, ToEndpoint(req.Addr));
			reply.Result = SyscallResult::Success;
			return;

		case NetIpc::Op::SendTo:
		case NetIpc::Op::Send:
		{
			//Send goes to the connected peer
			if (req.Code == NetIpc::Op::Send && !s->Connected) { reply.Result = SyscallResult::InvalidHandle; return; }
			const Net::endpoint_t dst = (req.Code == NetIpc::Op::Send) ? s->Peer : ToEndpoint(req.Addr);
			const size_t len = req.Length < NetIpc::MaxPayload ? req.Length : NetIpc::MaxPayload;
			reply.Result = SocketSend(*s, dst, req.Data, len) ? SyscallResult::Success : SyscallResult::Failed;
			return;
		}

		case NetIpc::Op::RecvFrom:
		{
			Net::endpoint_t src = {};
			size_t bytes = 0;
			if (!SocketRecv(*s, src, reply.Data, NetIpc::MaxPayload, bytes))
				return; //empty (Failed): the client retries until timeout
			reply.Length = (uint32_t)bytes;
			FromEndpoint(src, reply.From);
			reply.Result = SyscallResult::Success;
			return;
		}

		case NetIpc::Op::Close:
			SocketClose(*s);
			conn.Sockets[req.Socket - 1] = nullptr;
			reply.Result = SyscallResult::Success;
			return;

		default:
			return;
		}
	}

	void ProcessRequests()
	{
		for (Connection& conn : g_conns)
		{
			if (!conn.Region)
				continue;

			SharedRing<NetIpc::Request> reqRing = NetIpc::Channel::Requests(conn.Region);
			NetIpc::Request req;
			while (reqRing.Dequeue(req))
			{
				NetIpc::Reply reply = {};
				HandleRequest(conn, req, reply);
				NetIpc::Channel::Replies(conn.Region).Enqueue(reply);
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
	void* const rxRegion = IpcLookupRegion(NetDevice::RxEndpoint);
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

	//Own the control endpoint; only its owner can poll it for connections
	AssertSuccess(RegisterEndpoint(NetIpc::ControlEndpoint, 0));

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
		ReapDeadConnections();

		Sleep(1);
	}

	return 0;
}
