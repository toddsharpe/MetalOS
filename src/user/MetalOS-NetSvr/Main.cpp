#include "user/MetalOS.h"
#include "Lib/SharedRing.h"
#include "Lib/Span.h"
#include "Assert.h"
#include <stdio.h>

#include "Net/Net.h"
#include "Net/NetIf.h"
#include "Net/Packet.h"
#include "Net/HostDriver.h"
#include "user/Protocol_net.h"
#include "user/Ipc.h"
#include "Lib/ByteSwap.h"
#include "Lib/Math.h"

//MetalOS-NetSvr.exe -- usermode TCP/IP stack. Claims the kernel RX ring + NIC MAC, runs
//ARP/IPv4/ICMP/UDP/routing, transmits via NetSend; apps reach it over IPC (Protocol_net.h).


namespace
{
	//App IPC: each app's duplex channel holds the fds of the sockets it created; an fd names a socket only to the app that owns it.
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

	static constexpr size_t MaxConns = 16;
	static constexpr size_t MaxSocketsPerConn = 32;

	struct Connection
	{
		void* Region;      //mapped channel; null when the slot is free
		HSharedMem Handle; //our handle to the channel; CloseHandle releases it
		int Sockets[MaxSocketsPerConn]; //closed_fd when the slot is free
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

			for (int& fd : conn.Sockets)
			{
				if (fd != Net::Socket::closed_fd)
					Net::Socket::Close(fd);
				fd = Net::Socket::closed_fd;
			}
			CloseHandle((Handle)conn.Handle);
			conn.Region = nullptr;
		}
	}

	int* FindSlot(Connection& conn, const int fd)
	{
		for (int& slot : conn.Sockets)
		{
			if (slot == fd)
				return &slot;
		}
		return nullptr;
	}

	//The connection's slot holding the fd a request names, or null when the app doesn't own it
	int* FindSocket(Connection& conn, const uint32_t fd)
	{
		const int named = static_cast<int>(fd);
		return named == Net::Socket::closed_fd ? nullptr : FindSlot(conn, named);
	}

	//UDP for datagram sockets, ICMP echo for ICMP ones (raw or datagram, as Linux ping sockets take either)
	bool ToProto(const int type, const int proto, Net::proto_t& result)
	{
		if (proto == IPPROTO_UDP || (proto == 0 && type == SOCK_DGRAM))
			result = Net::proto_t::Udp;
		else if (proto == IPPROTO_ICMP || (proto == 0 && type == SOCK_RAW))
			result = Net::proto_t::Icmp;
		else
			return false;
		return true;
	}

	void HandleRequest(Connection& conn, const NetIpc::Request& req, NetIpc::Reply& reply)
	{
		reply.Result = SyscallResult::Failed;

		switch (req.Code)
		{
		case NetIpc::Op::Create:
		{
			Net::proto_t proto = {};
			if (!ToProto(req.Type, req.Proto, proto))
			{
				reply.Result = SyscallResult::InvalidArg;
				return;
			}

			//The app's own cap comes first, so a full app takes nothing from the stack's table
			int* const slot = FindSlot(conn, Net::Socket::closed_fd);
			if (!slot)
				return;

			const int fd = Net::Socket::Create(proto);
			if (fd == Net::Socket::closed_fd)
				return;

			*slot = fd;
			reply.Socket = static_cast<uint32_t>(fd);
			reply.Result = SyscallResult::Success;
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
			RegisterEndpoint(NetIpc::AddressEndpoint, req.Ip.s_addr);
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
		int* const slot = FindSocket(conn, req.Socket);
		if (!slot)
		{
			reply.Result = (req.Code == NetIpc::Op::Close) ? SyscallResult::Success : SyscallResult::InvalidHandle;
			return;
		}
		const int fd = *slot;

		switch (req.Code)
		{
		case NetIpc::Op::Bind:
			reply.Result = Net::Socket::Bind(fd, ToEndpoint(req.Addr)) ? SyscallResult::Success : SyscallResult::Failed;
			return;

		case NetIpc::Op::SendTo:
		{
			const Net::endpoint_t dst = ToEndpoint(req.Addr);
			const size_t len = Min<size_t>(req.Length, NetIpc::MaxPayload);
			reply.Result = Net::Socket::SendTo(fd, req.Data, len, dst) ? SyscallResult::Success : SyscallResult::Failed;
			return;
		}

		//Length is the app's buffer, so a datagram larger than it is dropped here rather than truncated
		case NetIpc::Op::RecvFrom:
		{
			Net::endpoint_t src = {};
			size_t bytes = 0;
			if (!Net::Socket::Recv(fd, reply.Data, Min<size_t>(req.Length, NetIpc::MaxPayload), bytes, &src))
				return; //empty (Failed): the client retries until timeout
			reply.Length = static_cast<uint32_t>(bytes);
			FromEndpoint(src, reply.From);
			reply.Result = SyscallResult::Success;
			return;
		}

		case NetIpc::Op::Close:
			Net::Socket::Close(fd);
			*slot = Net::Socket::closed_fd;
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
	UNUSED(argc);
	UNUSED(argv);
	InterfaceInfo ifaces[4] = {};
	size_t ifaceCount = 0;
	AssertSuccess(GetInterfaces(ifaces, 4, &ifaceCount));
	Assert(ifaceCount == 1);

	//Claim the kernel->netstack RX ring of raw ethernet frames.
	void* const rxRegion = IpcLookupRegion(NetDevice::RxEndpoint);
	Assert(rxRegion);

	//Build the single interface over the host NIC and register it for routing.
	HostDriver driver(ifaces[0], rxRegion);
	Net::NetIf net_if(driver, Net::l2_t::Ethernet);
	net_if.Init();
	Net::AddNetIf(net_if);
	g_netif = &net_if; //the interface-config IPC ops read/write this

	//IPv4 config starts unset; dhcp.exe configures it via SetInterfaceIp/SetGateway (IPC).
	DebugPrintf("netstack: up rx=%016x mac=%02x:%02x:%02x:%02x:%02x:%02x\n", (uint64_t)rxRegion,
		ifaces[0].mac[5], ifaces[0].mac[4], ifaces[0].mac[3], ifaces[0].mac[2], ifaces[0].mac[1], ifaces[0].mac[0]);

	//Own the control endpoint; only its owner can poll it for connections
	AssertSuccess(RegisterEndpoint(NetIpc::ControlEndpoint, 0));

	while (true)
	{
		//Drain received frames up the stack.
		net_if.PreDispatch();

		//Service app socket IPC: accept new connections, process pending requests.
		AcceptConnections();
		ProcessRequests();
		ReapDeadConnections();

		//Send what a busy driver refused before
		Net::Socket::Dispatch();

		Sleep(1);
	}

	return 0;
}
