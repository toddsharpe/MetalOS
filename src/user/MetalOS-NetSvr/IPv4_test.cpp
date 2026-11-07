//The stack's receive path, counting what reaches a socket
#include "Net/Net.h"
#include "Net/NetDriver.h"
#include "Net/NetIf.h"
#include "Net/Packet.h"
#include "Lib/Serializer.h"

namespace
{
	//Stands in for the NIC: counts the frames the stack sends through it, and frees them as a driver does
	class Ipv4TestDriver : public Net::NetDriver
	{
	public:
		Ipv4TestDriver(const uint8_t id) :
			Sent(),
			Ticks(),
			m_mac{ .bytes = {0x00, 0x15, 0x5D, 0x00, 0x01, id} }
		{

		}

		void Receive(Net::NetIf&) override
		{

		}

		bool Send(Net::NetIf&, Net::Packet& packet) override
		{
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
			return true;
		}

		milli_t GetTicks() const override
		{
			return Ticks;
		}

		size_t Sent;
		milli_t Ticks;

	private:
		Net::eth_mac_t m_mac;
	};

	//Hands the stack an IPv4 datagram as Ethernet::Receive would, in the rx pool with the ethernet header already masked
	template <size_t N>
	void Ipv4TestReceive(Net::NetIf& net_if, const uint8_t proto, const Net::ipv4_addr_t dst, const uint8_t (&payload)[N])
	{
		const buffer_t backing = Net::alloc_rx(Net::buffer_size);
		Assert(backing);
		Net::Packet packet(backing);
		packet.mask(Net::eth_hdr_size);

		Serializer ser(packet.buffer(), packet.length());
		ser.Write<uint8_t>(0x45);
		ser.Write<uint8_t>(0);
		ser.Write<uint16_t>((uint16_t)(Net::ipv4_hdr_size + N));
		ser.Write<uint16_t>(0);
		ser.Write<uint16_t>(0);
		ser.Write<uint8_t>(64);
		ser.Write<uint8_t>(proto);
		ser.Write<uint16_t>(0);
		ser.Write<uint32_t>(Net::build_ip(192, 168, 0, 20).addr);
		ser.Write<uint32_t>(dst.addr);
		ser.WriteBytes(payload);
		packet.trim(ser.Bytes());

		Net::IPv4::Receive(net_if, packet);
		if (!packet.taken)
			Net::free_rx(backing);
	}

	//The datagrams waiting on a socket, taken off it
	size_t Ipv4TestDrain(const int fd)
	{
		size_t count = 0;
		uint8_t data[Net::mtu] = {};
		size_t bytes = 0;
		while (Net::Socket::Recv(fd, data, sizeof(data), bytes, nullptr))
			count++;
		return count;
	}

	//A UDP header and 4 bytes of data
	void Ipv4TestUdp(Net::NetIf& net_if, const Net::ipv4_addr_t dst)
	{
		static constexpr uint8_t udp[12] = { 0x04, 0x00, 0x04, 0x00, 0x00, 0x0C, 0x00, 0x00, 'd', 'a', 't', 'a' };
		Ipv4TestReceive(net_if, Net::ip_proto_udp, dst, udp);
	}

	//An ICMP echo request header, its id and sequence number, and 4 bytes of data
	void Ipv4TestPing(Net::NetIf& net_if, const Net::ipv4_addr_t dst)
	{
		static constexpr uint8_t echo[12] = { Net::icmp_type_echo_request, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 'p', 'i', 'n', 'g' };
		Ipv4TestReceive(net_if, Net::ip_proto_icmp, dst, echo);
	}

	//Two interfaces, the first registered being the router's fallback for addresses no interface owns
	Ipv4TestDriver driverB(2);
	Net::NetIf netB(driverB, Net::l2_t::Ethernet);
	Ipv4TestDriver driverA(1);
	Net::NetIf netA(driverA, Net::l2_t::Ethernet);
}

void IPv4_test()
{
	Assert(Net::AddNetIf(netB));
	Assert(Net::AddNetIf(netA));
	netA.Init();
	netB.Init();
	netA.ipv4.addr = Net::build_ip(192, 168, 0, 10);
	netA.ipv4.subnet_mask = Net::build_ip(255, 255, 255, 0);
	netB.ipv4.addr = Net::build_ip(172, 16, 0, 1);
	netB.ipv4.subnet_mask = Net::build_ip(255, 255, 255, 0);

	//A socket on the test datagrams' port counts what the stack delivers
	const int fd = Net::Socket::Create(Net::proto_t::Udp);
	Assert(Net::Socket::Bind(fd, { Net::empty, 0x0400 }));
	size_t delivered = 0;

	//An interface doesn't forward until it is configured to
	Assert(!netA.ipv4.ip_forwarding);
	Assert(!netB.ipv4.ip_forwarding);

	//The interface's directed broadcast is accepted and counted as a broadcast, like the limited broadcast
	Ipv4TestUdp(netA, Net::build_ip(192, 168, 0, 255));
	delivered += Ipv4TestDrain(fd);
	AssertEqual(delivered, (size_t)1);
	AssertEqual(netA.ipv4.rx_broadcast, (uint32_t)1);
	Ipv4TestUdp(netA, Net::broadcast);
	delivered += Ipv4TestDrain(fd);
	AssertEqual(delivered, (size_t)2);
	AssertEqual(netA.ipv4.rx_broadcast, (uint32_t)2);
	AssertEqual(netA.ipv4.rx_accepted, (uint32_t)2);

	//Another subnet's broadcast and another host's address aren't ours
	Ipv4TestUdp(netA, Net::build_ip(192, 168, 1, 255));
	Ipv4TestUdp(netA, Net::build_ip(172, 16, 0, 5));
	delivered += Ipv4TestDrain(fd);
	AssertEqual(delivered, (size_t)2);
	AssertEqual(netA.ipv4.rx_unicast, (uint32_t)2);

	//Without forwarding, another host's packet goes nowhere
	AssertEqual(driverB.Sent, (size_t)0);

	//With forwarding, another host's packet goes out the interface that reaches it (as an ARP request, its MAC being unknown)
	netA.ipv4.ip_forwarding = true;
	Ipv4TestUdp(netA, Net::build_ip(172, 16, 0, 5));
	AssertEqual(driverB.Sent, (size_t)1);
	delivered += Ipv4TestDrain(fd);
	AssertEqual(delivered, (size_t)2);

	//Broadcasts are delivered locally but never forwarded: not the limited broadcast, ours, or the target subnet's
	Ipv4TestUdp(netA, Net::broadcast);
	Ipv4TestUdp(netA, Net::build_ip(192, 168, 0, 255));
	delivered += Ipv4TestDrain(fd);
	AssertEqual(delivered, (size_t)4);
	Ipv4TestUdp(netA, Net::build_ip(172, 16, 0, 255));
	delivered += Ipv4TestDrain(fd);
	AssertEqual(delivered, (size_t)4);
	AssertEqual(driverB.Sent, (size_t)1);
	AssertEqual(driverA.Sent, (size_t)0);
	netA.ipv4.ip_forwarding = false;

	//An echo request is the stack's, not a socket's: a broadcast one gets no reply, one to our address does (as an ARP request for the sender)
	Ipv4TestPing(netA, Net::build_ip(192, 168, 0, 255));
	Ipv4TestPing(netA, Net::broadcast);
	AssertEqual(driverA.Sent, (size_t)0);
	Ipv4TestPing(netA, netA.ipv4.addr);
	AssertEqual(driverA.Sent, (size_t)1);
	delivered += Ipv4TestDrain(fd);
	AssertEqual(delivered, (size_t)4);

	Assert(Net::Socket::Close(fd));

	//The forwarded packet and the echo reply wait on ARP for peers that never answer, until the requests expire
	driverA.Ticks = Net::arp_retry_time;
	driverB.Ticks = Net::arp_retry_time;
	netA.PreDispatch();
	netB.PreDispatch();
	Assert(!netA.ethernet.pending.backing);
	Assert(!netB.ethernet.pending.backing);
	AssertEqual(driverA.Sent, (size_t)1);
	AssertEqual(driverB.Sent, (size_t)1);

	//Every buffer the stack sent from or dropped went back: both pools are first fit, so each allocates whole
	const buffer_t rx = Net::alloc_rx(Net::pool_size);
	Assert(rx);
	Net::free_rx(rx);
	const buffer_t tx = Net::alloc_tx(Net::pool_size);
	Assert(tx);
	Net::free_tx(tx);
}
