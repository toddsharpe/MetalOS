#include "Lib/Buffer.h"
#include "Net/NetIf.h"
#include "Assert.h"
#include "Net/Packet.h"

namespace Net::Socket
{
	//Receive() is implemented in Kernel_sockets.cpp, where the bound-socket registry
	//lives; it demuxes inbound packets into per-KSocket queues.

	namespace
	{
		//Note(tsharpe): Ideally this method would copy user data to one buffer and pass down multiple stacks if needed.
		//However, without scatter/gather for headers (which will be interface dependent), copying is necessary
		bool SendPayload(const endpoint_t dst, const uint16_t src_port, const void* const buffer, const size_t length, const size_t header_size, const proto_t proto, const handler_tx send)
		{
			NetIf& net_if = Router::Resolve(dst);

			//Require a configured source address, except for broadcast (e.g. DHCP DISCOVER
			//is sent from 0.0.0.0 before the interface has an address).
			if (net_if.ipv4.addr == Net::empty && dst.addr != Net::broadcast)
				return false;

			StaticBuffer<Net::buffer_size> allocated;
			Packet packet(allocated);
			packet.mask(get_l2(net_if.l2).HeaderSize);
			packet.mask(ipv4_hdr_size);
			packet.mask(header_size);

			//The payload has to fit in the space left after the headers
			if (length > packet.length())
				return false;

			memcpy(packet.buffer(), buffer, length);
			packet.trim(length);
			packet.dst = dst;
			packet.src = { net_if.ipv4.addr, src_port };
			packet.proto = proto;
			return send(net_if, packet);
		}
	}

	bool SendUdp(const endpoint_t dst, const uint16_t src_port, const void* const buffer, const size_t length)
	{
		const uint16_t use_port = src_port != 0 ? src_port : rand_port();
		return SendPayload(dst, use_port, buffer, length, udp_hdr_size, proto_t::Udp, &Udp::Send);
	}

	//Sends a caller-supplied, fully-formed ICMP message (raw socket): only L2/IPv4 headers
	//are added. The ICMP header + checksum are the caller's responsibility.
	bool SendIcmpRaw(const endpoint_t dst, const void* const buffer, const size_t length)
	{
		return SendPayload({ dst.addr, 0 }, 0, buffer, length, 0, proto_t::Icmp, &IPv4::Send);
	}
}
