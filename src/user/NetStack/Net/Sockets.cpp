#include "Net/Net.h"
#include "Net/KSocket.h"
#include "Net/NetIf.h"
#include "Net/Packet.h"
#include "Lib/LinkedList.h"
#include "Lib/Time.h"
#include "Assert.h"

using namespace Net;

//Userland SOCK_*/IPPROTO_* values (kept in sync with src/User/MetalOS.Types.h).
static constexpr int kSockDgram = 2;
static constexpr int kSockRaw   = 3;
static constexpr int kProtoIcmp = 1;
static constexpr int kProtoUdp  = 17;

//Bound-socket registry + inbound demux. netstack is single-threaded (one RX/IPC loop),
//so no lock is needed. Sockets are owned by the IPC layer (one per app socket handle);
//the registry demuxes received datagrams into the matching socket's queue.
namespace
{
	LinkedList<KSocket*> g_sockets;

	bool ProtoMatches(const proto_t bound, const proto_t incoming)
	{
		if (bound == proto_t::Udp)
			return incoming == proto_t::Udp;

		//Raw ICMP sockets accept every ICMP-family message.
		if (bound == proto_t::Icmp)
		{
			return incoming == proto_t::Icmp
				|| incoming == proto_t::EchoReply
				|| incoming == proto_t::EchoRequest
				|| incoming == proto_t::DestUnreachable;
		}

		return false;
	}

	KSocket* FindBound(const proto_t proto, const uint16_t port)
	{
		for (KSocket* socket : g_sockets)
		{
			if (!ProtoMatches(socket->Proto, proto))
				continue;
			if (socket->Proto == proto_t::Udp && socket->Local.port != port)
				continue;
			return socket;
		}
		return nullptr;
	}

	void Register(KSocket& socket)
	{
		if (socket.Registered)
			return;
		Assert(g_sockets.Add(&socket));
		socket.Registered = true;
	}

	void Unregister(KSocket& socket)
	{
		if (!socket.Registered)
			return;
		g_sockets.Remove(&socket);
		socket.Registered = false;
	}
}

/*
 * Socket API (called by the netstack IPC layer that services app requests).
 */
KSocket* SocketCreate(const int type, const int proto)
{
	proto_t p;
	if (proto == kProtoUdp || (proto == 0 && type == kSockDgram))
		p = proto_t::Udp;
	else if (proto == kProtoIcmp || (type == kSockRaw && proto == 0))
		p = proto_t::Icmp;
	else
		return nullptr;

	KSocket* socket = new KSocket(p);
	if (!socket)
		return nullptr;

	//Raw sockets demux by protocol only, so they can register at creation.
	if (p == proto_t::Icmp)
		Register(*socket);

	return socket;
}

bool SocketBind(KSocket& socket, const endpoint_t& local)
{
	socket.Local = local;
	if (socket.Local.port == 0)
		socket.Local.port = rand_port();
	Register(socket);
	return true;
}

bool SocketConnect(KSocket& socket, const endpoint_t& peer)
{
	socket.Peer = peer;
	socket.Connected = true;
	return true;
}

bool SocketSend(KSocket& socket, const endpoint_t& dst, const void* const buffer, const size_t length)
{
	if (socket.Proto == proto_t::Udp)
	{
		//Assign an ephemeral local port on first send so replies can be demuxed back.
		if (socket.Local.port == 0)
		{
			socket.Local.port = rand_port();
			Register(socket);
		}
		return Net::Socket::SendUdp(dst, socket.Local.port, buffer, length);
	}

	//Raw ICMP: the caller supplies the entire ICMP message (type/code/checksum/payload).
	if (socket.Proto == proto_t::Icmp)
		return Net::Socket::SendIcmpRaw(dst, buffer, length);

	return false;
}

//Non-blocking receive: returns one queued datagram or Empty. Apps block by polling
//their reply ring, so netstack never blocks its single loop here.
Net::Socket::read_t SocketRecv(KSocket& socket, endpoint_t& src, void* const buffer, const size_t maxLength, size_t& bytesRead)
{
	bytesRead = 0;
	if (socket.Closed)
		return Net::Socket::read_t::Failure;

	return socket.Dequeue(src, buffer, maxLength, bytesRead)
		? Net::Socket::read_t::Success
		: Net::Socket::read_t::Empty;
}

void SocketClose(KSocket& socket)
{
	socket.MarkClosed();
	Unregister(socket);
	delete &socket;
}

/*
 * Inbound demux (declared in Net.h). Called from Udp::Receive / Icmp::Receive.
 */
namespace Net::Socket
{
	bool Receive(NetIf& net_if, Packet& packet)
	{
		UNUSED(net_if);
		KSocket* socket = FindBound(packet.proto, packet.dst.port);
		if (!socket)
			return false;

		return socket->Enqueue(packet.src, packet.buffer(), packet.length());
	}
}
