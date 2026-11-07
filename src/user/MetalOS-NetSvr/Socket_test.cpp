//Sockets, ported from Nighthawk's test: the real layer over the real chain down to a recording driver, with the pools whole at the end
#include "Net/Net.h"
#include "Net/NetDriver.h"
#include "Net/NetIf.h"
#include "Net/Packet.h"

namespace
{
	//Stands in for the NIC: keeps the last frame it sends, or refuses as a busy one does
	class SocketTestDriver : public Net::NetDriver
	{
	public:
		SocketTestDriver() :
			Sent(),
			Refuse(),
			LinkUp(true),
			Frame(),
			Length(),
			m_mac{ .bytes = {0x00, 0x15, 0x5D, 0x00, 0x02, 0x01} }
		{

		}

		void Receive(Net::NetIf&) override
		{

		}

		bool Send(Net::NetIf&, Net::Packet& packet) override
		{
			if (Refuse)
				return false;

			Assert(packet.length() <= sizeof(Frame));
			memcpy(Frame, packet.buffer(), packet.length());
			Length = packet.length();
			Net::free_tx(packet.backing);
			Sent++;
			return true;
		}

		const Net::eth_mac_t& GetMac() const override
		{
			return m_mac;
		}

		bool IsLinkUp() const override
		{
			return LinkUp;
		}

		milli_t GetTicks() const override
		{
			return 0;
		}

		size_t Sent;
		bool Refuse;
		bool LinkUp;
		uint8_t Frame[Net::buffer_size];
		size_t Length;

	private:
		Net::eth_mac_t m_mac;
	};

	SocketTestDriver socketDriver;
	Net::NetIf socketIf(socketDriver, Net::l2_t::Ethernet);

	constexpr uint8_t SocketTestPayload[] = { 'h', 'i', '!' };

	//Where the frame's layers start: Ethernet, then IPv4, then UDP or ICMP
	constexpr size_t IpOffset = Net::eth_hdr_size;
	constexpr size_t TransportOffset = IpOffset + Net::ipv4_hdr_size;

	//A datagram as UDP hands one up: in the rx pool, which the socket keeps or the test frees
	bool SocketTestDeliver(const Net::endpoint_t src, const Net::endpoint_t dst)
	{
		const buffer_t backing = Net::alloc_rx(sizeof(SocketTestPayload));
		Assert(backing);
		memcpy(backing.Data, SocketTestPayload, sizeof(SocketTestPayload));

		Net::Packet packet(backing);
		packet.src = src;
		packet.dst = dst;
		packet.proto = Net::proto_t::Udp;
		const bool taken = Net::Socket::Receive(socketIf, packet);
		Assert(taken == packet.taken);
		if (!packet.taken)
			Net::free_rx(backing);
		return taken;
	}

	//An echo reply as IPv4 hands one to Icmp, carrying the id and seq given
	void SocketTestEchoReply(const Net::endpoint_t src, const Net::ipv4_addr_t dst, const uint16_t id, const uint16_t seq)
	{
		const uint8_t message[] = { Net::icmp_type_echo_reply, 0, 0, 0, static_cast<uint8_t>(id >> 8), static_cast<uint8_t>(id), static_cast<uint8_t>(seq >> 8), static_cast<uint8_t>(seq) };
		const buffer_t backing = Net::alloc_rx(sizeof(message));
		Assert(backing);
		memcpy(backing.Data, message, sizeof(message));

		Net::Packet packet(backing);
		packet.src = src;
		packet.dst = { dst, 0 };
		Net::Icmp::Receive(socketIf, packet);
		if (!packet.taken)
			Net::free_rx(backing);
	}

	uint16_t SocketTestU16(const uint8_t* const bytes)
	{
		return static_cast<uint16_t>((bytes[0] << 8) | bytes[1]);
	}
}

void Socket_test()
{
	Assert(Net::AddNetIf(socketIf));
	socketIf.Init();
	socketIf.ipv4.addr = Net::build_ip(10, 8, 0, 1);
	socketIf.ipv4.subnet_mask = Net::build_ip(255, 255, 255, 0);
	const Net::endpoint_t here = { socketIf.ipv4.addr, 5000 };
	const Net::endpoint_t there = { Net::build_ip(10, 8, 0, 2), 53 };

	//The peer's MAC is known, so frames go straight out rather than waiting on ARP
	Net::ArpCache::Update(there.addr, { .bytes = {0x00, 0x15, 0x5D, 0x00, 0x02, 0x02} });

	//A socket is an fd from 100 up, one more each time; closing it twice is a mistake the second time
	{
		const int first = Net::Socket::Create(Net::proto_t::Udp);
		const int second = Net::Socket::Create(Net::proto_t::Udp);
		Assert(first >= 100);
		AssertEqual(second, first + 1);
		Assert(Net::Socket::Close(first));
		Assert(!Net::Socket::Close(first));
		Assert(Net::Socket::Close(second));
	}

	//A closed fd names nothing, even once its slot is reused
	{
		const int closed = Net::Socket::Create(Net::proto_t::Udp);
		Assert(Net::Socket::Close(closed));
		const int reused = Net::Socket::Create(Net::proto_t::Udp);
		Assert(reused != closed);
		Assert(!Net::Socket::Bind(closed, here));
		Assert(Net::Socket::Close(reused));
	}

	//A bound socket takes a datagram for its endpoint, and Recv hands back the payload and who sent it
	{
		const int fd = Net::Socket::Create(Net::proto_t::Udp);
		Assert(Net::Socket::Bind(fd, here));
		Assert(SocketTestDeliver(there, here));

		uint8_t out[16] = {};
		size_t bytes = 0;
		Net::endpoint_t from = {};
		Assert(Net::Socket::Recv(fd, out, sizeof(out), bytes, &from));
		AssertEqual(bytes, sizeof(SocketTestPayload));
		Assert(memcmp(out, SocketTestPayload, sizeof(SocketTestPayload)) == 0);
		Assert(from == there);
		Assert(!Net::Socket::Recv(fd, out, sizeof(out), bytes, &from));
		Assert(Net::Socket::Close(fd));
	}

	//An unbound socket matches nothing, and a bound one only its own port
	{
		const int fd = Net::Socket::Create(Net::proto_t::Udp);
		Assert(!SocketTestDeliver(there, here));
		Assert(Net::Socket::Bind(fd, here));
		Assert(!SocketTestDeliver(there, { here.addr, 5001 }));
		Assert(Net::Socket::Close(fd));
	}

	//Eight datagrams queue and a ninth is refused and counted
	{
		const int fd = Net::Socket::Create(Net::proto_t::Udp);
		Assert(Net::Socket::Bind(fd, here));
		for (int i = 0; i < 8; i++)
			Assert(SocketTestDeliver(there, here));

		const uint32_t full = Net::stack_stats.socket_rx_full;
		Assert(!SocketTestDeliver(there, here));
		AssertEqual(Net::stack_stats.socket_rx_full, full + 1);

		uint8_t out[16] = {};
		size_t bytes = 0;
		for (int i = 0; i < 8; i++)
			Assert(Net::Socket::Recv(fd, out, sizeof(out), bytes, nullptr));
		Assert(!Net::Socket::Recv(fd, out, sizeof(out), bytes, nullptr));
		Assert(Net::Socket::Close(fd));
	}

	//A datagram larger than the buffer is dropped, not truncated, and the next one still comes
	{
		const int fd = Net::Socket::Create(Net::proto_t::Udp);
		Assert(Net::Socket::Bind(fd, here));
		Assert(SocketTestDeliver(there, here));

		uint8_t out[16] = {};
		size_t bytes = 0;
		Assert(!Net::Socket::Recv(fd, out, sizeof(SocketTestPayload) - 1, bytes, nullptr));
		Assert(!Net::Socket::Recv(fd, out, sizeof(out), bytes, nullptr));

		Assert(SocketTestDeliver(there, here));
		Assert(Net::Socket::Recv(fd, out, sizeof(SocketTestPayload), bytes, nullptr));
		AssertEqual(bytes, sizeof(SocketTestPayload));
		Assert(Net::Socket::Close(fd));
	}

	//SendTo goes through the whole chain: the driver gets the Ethernet, IPv4 and UDP headers and the payload
	{
		const int fd = Net::Socket::Create(Net::proto_t::Udp);
		Assert(Net::Socket::Bind(fd, here));
		const size_t sent = socketDriver.Sent;
		Assert(Net::Socket::SendTo(fd, SocketTestPayload, sizeof(SocketTestPayload), there));
		AssertEqual(socketDriver.Sent, sent + 1);

		const uint8_t* const frame = socketDriver.Frame;
		AssertEqual(socketDriver.Length, TransportOffset + Net::udp_hdr_size + sizeof(SocketTestPayload));
		AssertEqual(SocketTestU16(&frame[12]), Net::eth_type_ipv4);
		AssertEqual(frame[IpOffset + 9], Net::ip_proto_udp);
		const uint8_t dst[] = { 10, 8, 0, 2 };
		Assert(memcmp(&frame[IpOffset + 16], dst, sizeof(dst)) == 0);
		AssertEqual(SocketTestU16(&frame[TransportOffset]), here.port);
		AssertEqual(SocketTestU16(&frame[TransportOffset + 2]), there.port);
		AssertEqual(SocketTestU16(&frame[TransportOffset + 4]), static_cast<uint16_t>(Net::udp_hdr_size + sizeof(SocketTestPayload)));
		Assert(memcmp(&frame[TransportOffset + Net::udp_hdr_size], SocketTestPayload, sizeof(SocketTestPayload)) == 0);
		Assert(Net::Socket::Close(fd));
	}

	//An unbound socket sends from an ephemeral port, which then takes the replies
	{
		const int fd = Net::Socket::Create(Net::proto_t::Udp);
		Assert(Net::Socket::SendTo(fd, SocketTestPayload, sizeof(SocketTestPayload), there));
		const uint16_t port = SocketTestU16(&socketDriver.Frame[TransportOffset]);
		Assert(port != 0);
		Assert(SocketTestDeliver(there, { here.addr, port }));
		Assert(Net::Socket::Close(fd));
	}

	//A driver that won't take the frame keeps it queued, and the next dispatch delivers it
	{
		const int fd = Net::Socket::Create(Net::proto_t::Udp);
		Assert(Net::Socket::Bind(fd, here));
		const size_t sent = socketDriver.Sent;
		socketDriver.Refuse = true;
		Assert(Net::Socket::SendTo(fd, SocketTestPayload, sizeof(SocketTestPayload), there));
		Net::Socket::Dispatch();
		AssertEqual(socketDriver.Sent, sent);

		socketDriver.Refuse = false;
		Net::Socket::Dispatch();
		AssertEqual(socketDriver.Sent, sent + 1);
		Assert(Net::Socket::Close(fd));
	}

	//Four sends wait on a busy driver and a fifth is refused and counted
	{
		const int fd = Net::Socket::Create(Net::proto_t::Udp);
		Assert(Net::Socket::Bind(fd, here));
		const size_t sent = socketDriver.Sent;
		socketDriver.Refuse = true;
		for (int i = 0; i < 4; i++)
			Assert(Net::Socket::SendTo(fd, SocketTestPayload, sizeof(SocketTestPayload), there));

		const uint32_t full = Net::stack_stats.socket_tx_full;
		Assert(!Net::Socket::SendTo(fd, SocketTestPayload, sizeof(SocketTestPayload), there));
		AssertEqual(Net::stack_stats.socket_tx_full, full + 1);

		socketDriver.Refuse = false;
		Net::Socket::Dispatch();
		AssertEqual(socketDriver.Sent, sent + 4);
		Assert(Net::Socket::Close(fd));
	}

	//A frame for a link that is down drops once, rather than being retried every dispatch and jamming the queue behind it
	{
		const int fd = Net::Socket::Create(Net::proto_t::Udp);
		Assert(Net::Socket::Bind(fd, here));
		const size_t sent = socketDriver.Sent;
		socketDriver.LinkUp = false;
		Assert(Net::Socket::SendTo(fd, SocketTestPayload, sizeof(SocketTestPayload), there));
		socketDriver.LinkUp = true;
		Net::Socket::Dispatch();
		AssertEqual(socketDriver.Sent, sent);
		Assert(Net::Socket::Close(fd));
	}

	//A datagram too big for one frame is refused
	{
		const int fd = Net::Socket::Create(Net::proto_t::Udp);
		static uint8_t big[Net::mtu] = {};
		Assert(!Net::Socket::SendTo(fd, big, Net::mtu - Net::ipv4_hdr_size - Net::udp_hdr_size + 1, there));
		Assert(Net::Socket::Close(fd));
	}

	//An ICMP socket sends only echo requests, with its port as the echo id and the checksum filled in
	{
		const int fd = Net::Socket::Create(Net::proto_t::Icmp);
		const uint8_t reply[] = { Net::icmp_type_echo_reply, 0, 0, 0, 0, 0, 0, 1 };
		Assert(!Net::Socket::SendTo(fd, reply, sizeof(reply), there));
		const uint8_t shorter[] = { Net::icmp_type_echo_request, 0, 0, 0 };
		Assert(!Net::Socket::SendTo(fd, shorter, sizeof(shorter), there));

		const uint8_t request[] = { Net::icmp_type_echo_request, 0, 0, 0, 0, 0, 0, 1, 'p', 'i', 'n', 'g' };
		Assert(Net::Socket::SendTo(fd, request, sizeof(request), there));
		const uint8_t* const frame = socketDriver.Frame;
		AssertEqual(socketDriver.Length, TransportOffset + sizeof(request));
		AssertEqual(frame[IpOffset + 9], Net::ip_proto_icmp);
		AssertEqual(frame[TransportOffset], Net::icmp_type_echo_request);
		AssertEqual(Net::checksum(&frame[TransportOffset], sizeof(request)), static_cast<uint16_t>(0));
		const uint16_t id = SocketTestU16(&frame[TransportOffset + 4]);
		Assert(id != 0);
		AssertEqual(SocketTestU16(&frame[TransportOffset + 6]), static_cast<uint16_t>(1));
		Assert(memcmp(&frame[TransportOffset + 8], "ping", 4) == 0);

		//The reply to that id comes back whole; one to another id goes to no socket
		SocketTestEchoReply(there, here.addr, id, 1);
		SocketTestEchoReply(there, here.addr, static_cast<uint16_t>(id + 1), 2);

		uint8_t out[32] = {};
		size_t bytes = 0;
		Net::endpoint_t from = {};
		Assert(Net::Socket::Recv(fd, out, sizeof(out), bytes, &from));
		AssertEqual(bytes, static_cast<size_t>(8));
		AssertEqual(out[0], Net::icmp_type_echo_reply);
		AssertEqual(SocketTestU16(&out[4]), id);
		AssertEqual(SocketTestU16(&out[6]), static_cast<uint16_t>(1));
		Assert(from.addr == there.addr);
		Assert(!Net::Socket::Recv(fd, out, sizeof(out), bytes, &from));
		Assert(Net::Socket::Close(fd));
	}

	//The table holds 64 sockets; the 65th is refused until one closes
	{
		int fds[64] = {};
		for (int& fd : fds)
		{
			fd = Net::Socket::Create(Net::proto_t::Udp);
			Assert(fd != Net::Socket::closed_fd);
		}
		AssertEqual(Net::Socket::Create(Net::proto_t::Udp), Net::Socket::closed_fd);

		Assert(Net::Socket::Close(fds[0]));
		fds[0] = Net::Socket::Create(Net::proto_t::Udp);
		Assert(fds[0] != Net::Socket::closed_fd);
		for (const int fd : fds)
			Assert(Net::Socket::Close(fd));
	}

	//Closing a socket that still holds datagrams and sends gives their buffers back, which the pools below show
	{
		const int fd = Net::Socket::Create(Net::proto_t::Udp);
		Assert(Net::Socket::Bind(fd, here));
		Assert(SocketTestDeliver(there, here));
		socketDriver.Refuse = true;
		Assert(Net::Socket::SendTo(fd, SocketTestPayload, sizeof(SocketTestPayload), there));
		socketDriver.Refuse = false;
		Assert(Net::Socket::Close(fd));
	}

	//Everything taken from the pools has gone back: both are first fit, so each allocates whole
	const buffer_t rx = Net::alloc_rx(Net::pool_size);
	Assert(rx);
	Net::free_rx(rx);
	const buffer_t tx = Net::alloc_tx(Net::pool_size);
	Assert(tx);
	Net::free_tx(tx);
}
