//The stack's receive path without sockets; ArpCache_test.cpp already brings in Net/ArpCache.cpp
#include "Net/Checksum.cpp"
#include "Net/Net.cpp"
#include "Net/Arp.cpp"
#include "Net/Ethernet.cpp"
#include "Net/IPv4.cpp"
#include "Net/Icmp.cpp"
#include "Net/Udp.cpp"
#include "Net/Router.cpp"

namespace
{
	size_t ipv4TestDelivered;

	//Stands in for the NIC: counts the frames the stack sends through it
	class Ipv4TestDriver : public Net::NetDriver
	{
	public:
		Ipv4TestDriver(const uint8_t id) :
			Sent(),
			m_mac{ .bytes = {0x00, 0x15, 0x5D, 0x00, 0x01, id} }
		{

		}

		void Receive(Net::NetIf&) override
		{

		}

		bool Send(Net::NetIf&, Net::Packet&) override
		{
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
			return 0;
		}

		size_t Sent;

	private:
		Net::eth_mac_t m_mac;
	};

	//Hands the stack an IPv4 datagram as Ethernet::Receive would, with the ethernet header already masked
	template <size_t N>
	void Ipv4TestReceive(Net::NetIf& net_if, const uint8_t proto, const Net::ipv4_addr_t dst, const uint8_t (&payload)[N])
	{
		StaticBuffer<Net::buffer_size> buffer;
		Net::Packet packet(buffer);
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

namespace Net::Socket
{
	//Stands in for Sockets.cpp's bound-socket registry: counts what the stack delivers
	bool Receive(NetIf& net_if, Packet& packet)
	{
		ipv4TestDelivered++;
		return true;
	}
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

	//An interface doesn't forward until it is configured to
	Assert(!netA.ipv4.ip_forwarding);
	Assert(!netB.ipv4.ip_forwarding);

	//The interface's directed broadcast is accepted and counted as a broadcast, like the limited broadcast
	Ipv4TestUdp(netA, Net::build_ip(192, 168, 0, 255));
	AssertEqual(ipv4TestDelivered, (size_t)1);
	AssertEqual(netA.ipv4.rx_broadcast, (uint32_t)1);
	Ipv4TestUdp(netA, Net::broadcast);
	AssertEqual(ipv4TestDelivered, (size_t)2);
	AssertEqual(netA.ipv4.rx_broadcast, (uint32_t)2);
	AssertEqual(netA.ipv4.rx_accepted, (uint32_t)2);

	//Another subnet's broadcast and another host's address aren't ours
	Ipv4TestUdp(netA, Net::build_ip(192, 168, 1, 255));
	Ipv4TestUdp(netA, Net::build_ip(172, 16, 0, 5));
	AssertEqual(ipv4TestDelivered, (size_t)2);
	AssertEqual(netA.ipv4.rx_unicast, (uint32_t)2);

	//Without forwarding, another host's packet goes nowhere
	AssertEqual(driverB.Sent, (size_t)0);

	//With forwarding, another host's packet goes out the interface that reaches it (as an ARP request, its MAC being unknown)
	netA.ipv4.ip_forwarding = true;
	Ipv4TestUdp(netA, Net::build_ip(172, 16, 0, 5));
	AssertEqual(driverB.Sent, (size_t)1);
	AssertEqual(ipv4TestDelivered, (size_t)2);

	//Broadcasts are delivered locally but never forwarded: not the limited broadcast, ours, or the target subnet's
	Ipv4TestUdp(netA, Net::broadcast);
	Ipv4TestUdp(netA, Net::build_ip(192, 168, 0, 255));
	AssertEqual(ipv4TestDelivered, (size_t)4);
	Ipv4TestUdp(netA, Net::build_ip(172, 16, 0, 255));
	AssertEqual(ipv4TestDelivered, (size_t)4);
	AssertEqual(driverB.Sent, (size_t)1);
	AssertEqual(driverA.Sent, (size_t)0);
	netA.ipv4.ip_forwarding = false;

	//A broadcast ping reaches raw sockets but gets no reply; a ping to our address does (as an ARP request for the sender)
	Ipv4TestPing(netA, Net::build_ip(192, 168, 0, 255));
	Ipv4TestPing(netA, Net::broadcast);
	AssertEqual(ipv4TestDelivered, (size_t)6);
	AssertEqual(driverA.Sent, (size_t)0);
	Ipv4TestPing(netA, netA.ipv4.addr);
	AssertEqual(ipv4TestDelivered, (size_t)7);
	AssertEqual(driverA.Sent, (size_t)1);
}
