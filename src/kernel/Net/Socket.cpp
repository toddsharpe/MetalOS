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
	bool Receive(NetIf& net_if, Packet& packet)
	{
		return true;
	}

	read_t ReadUdp(const uint16_t dst_port, endpoint_t& src, void* const buffer, const size_t length, size_t& bytes_read)
	{
		return read_t::Empty;
	}

	read_t ReadUdp(const endpoint_t dst, endpoint_t& src, void* const buffer, const size_t length, size_t& bytes_read)
	{
		return read_t::Empty;
	}
	
	//Note(tsharpe): Ideally this method would copy user data to one buffer and pass down multiple stacks if needed.
	//However, without scatter/gather for headers (which will be interface dependent), copying is necessary
	bool SendUdp(const endpoint_t dst, const uint16_t src_port, const void* const buffer, const size_t length)
	{
		NetIf& net_if = Router::Resolve(dst);

		//Wait for valid ip address on this interface to send udp
		if (net_if.ipv4.addr == Net::empty)
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

	bool SendUdpIf(NetIf& net_if, const endpoint_t dst, const uint16_t src_port, const void* const buffer, const size_t length)
	{
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
}
