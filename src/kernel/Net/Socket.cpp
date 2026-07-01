#include "Lib/Buffer.h"
#include "Net/BufferPool.h"
#include "Net/NetIf.h"
#include "Assert.h"
#include "Net/Packet.h"
#include "Lib/CircularBuffer.h"

namespace
{
	struct packet_entry_t
	{
		Net::NetIf* src_if;
		Net::Packet packet;
	};

	static constexpr size_t buffer_count = 8;
}

namespace Net::Socket
{
	//Receive() is implemented in Kernel_sockets.cpp, where the bound-socket registry
	//lives; it demuxes inbound packets into per-KSocket queues.

	//Note(tsharpe): Ideally this method would copy user data to one buffer and pass down multiple stacks if needed.
	//However, without scatter/gather for headers (which will be interface dependent), copying is necessary
	bool SendUdp(const endpoint_t dst, const uint16_t src_port, const void* const buffer, const size_t length)
	{
		NetIf& net_if = Router::Resolve(dst);

		//Require a configured source address, except for broadcast (e.g. DHCP DISCOVER
		//is sent from 0.0.0.0 before the interface has an address).
		if (net_if.ipv4.addr == Net::empty && dst.addr != Net::broadcast)
			return false;

		//Get packet from driver
		StaticBuffer<Net::buffer_size> allocated;
		if (length > allocated.Capacity)
			return false;

		const NetLayer& l2 = get_l2(net_if.l2);

		Packet packet(allocated);
		packet.mask(l2.HeaderSize);
		packet.mask(ipv4_hdr_size);
		packet.mask(udp_hdr_size);

		//Write data to packet
		memcpy(packet.buffer(), buffer, length);
		packet.trim(length);

		//Populate packet fields
		const uint16_t use_port = src_port != 0 ? src_port : rand_port();
		packet.dst = dst;
		packet.src = { net_if.ipv4.addr, use_port };

		return Udp::Send(net_if, packet);
	}

	bool SendTo(const endpoint_t dst, const uint16_t src_port, const void* const buffer, const size_t length, const proto_t proto)
	{
		NetIf& net_if = Router::Resolve(dst);

		//Get packet from driver
		StaticBuffer<Net::buffer_size> allocated;
		if (length > allocated.Capacity)
			return false;

		const NetLayer& l2 = get_l2(net_if.l2);
		Packet packet(allocated);

		switch (proto)
		{
			case proto_t::Udp:
				packet.mask(l2.HeaderSize);
				packet.mask(ipv4_hdr_size);
				packet.mask(udp_hdr_size);
				break;

			case proto_t::Icmp:
				packet.mask(l2.HeaderSize);
				packet.mask(ipv4_hdr_size);
				packet.mask(icmp_hdr_size);
				break;

			default:
				Assert(false);
				return false;
		}

		//Write data to packet
		memcpy(packet.buffer(), buffer, length);
		packet.trim(length);

		switch (proto)
		{
			case proto_t::Udp:
			{
				//Populate packet fields
				const uint16_t use_port = src_port != 0 ? src_port : rand_port();
				packet.dst = dst;
				packet.src = { net_if.ipv4.addr, use_port };

				return Udp::Send(net_if, packet);
			}
			break;

			case proto_t::Icmp:
			{
				packet.dst = { dst.addr, 0 };
				packet.src = { net_if.ipv4.addr, 0 };
				return Icmp::Send(net_if, packet);
			}
			break;

			default:
				Assert(false);
				return false;
		}
	}

	//Sends a caller-supplied, fully-formed ICMP message (raw socket): only L2/IPv4 headers
	//are added. The ICMP header + checksum are the caller's responsibility.
	bool SendIcmpRaw(const endpoint_t dst, const void* const buffer, const size_t length)
	{
		NetIf& net_if = Router::Resolve(dst);
		if (net_if.ipv4.addr == Net::empty && dst.addr != Net::broadcast)
			return false;

		StaticBuffer<Net::buffer_size> allocated;
		if (length > allocated.Capacity)
			return false;

		const NetLayer& l2 = get_l2(net_if.l2);
		Packet packet(allocated);
		packet.mask(l2.HeaderSize);
		packet.mask(ipv4_hdr_size);

		memcpy(packet.buffer(), buffer, length);
		packet.trim(length);

		packet.dst = { dst.addr, 0 };
		packet.src = { net_if.ipv4.addr, 0 };
		packet.proto = proto_t::Icmp;

		return IPv4::Send(net_if, packet);
	}
}
