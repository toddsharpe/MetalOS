#include "Net/Net.h"
#include "Net/NetIf.h"
#include "Lib/Deserializer.h"
#include "Lib/Serializer.h"

namespace Net::Udp
{
	/*
	struct udp_pseudo_hdr_t
	{
		ipv4_addr_t src;
		ipv4_addr_t dst;
		uint8_t zeroes;
		uint8_t proto;
		uint16_t length;
	};
	
	static uint16_t calculate_checksum(const Packet& packet)
	{
		const udp_pseudo_hdr_t hdr =
		{
			.src = packet.src.addr,
			.dst = packet.dst.addr,
			.zeroes = 0,
			.proto = ip_proto_udp,
			.length = static_cast<uint16_t>(packet.length()),
		};

		uint8_t buffer[Net::buffer_size] = {};
		Serializer ser(buffer, sizeof(buffer));
		ser.Write(hdr.src.addr);
		ser.Write(hdr.dst.addr);
		ser.Write(hdr.zeroes);
		ser.Write(hdr.proto);
		ser.Write(hdr.length);

		uint32_t checksum = checksum_add(0, buffer, ser.Bytes());
		checksum = checksum_add(checksum, packet.buffer(), packet.length());
		return checksum_finalize(checksum);
	}
	*/

	bool Meta(Packet& packet)
	{
		if (packet.length() < udp_hdr_size)
			return false;

		Deserializer deser(packet.buffer(), packet.length());
		const udp_hdr_t udp_hdr =
		{
			.src_port = deser.ReadU16(),
			.dst_port = deser.ReadU16(),
			.length = deser.ReadU16(),
			.checksum = deser.ReadU16(),
		};
		packet.mask(udp_hdr_size);

		//Update endpoints
		packet.dst.port = udp_hdr.dst_port;
		packet.src.port = udp_hdr.src_port;
		packet.proto = proto_t::Udp;

		return true;
	}
	
	bool Receive(NetIf& net_if, Packet& packet)
	{
		if (packet.length() < udp_hdr_size)
			return false;

		Deserializer deser(packet.buffer(), packet.length());
		const udp_hdr_t udp_hdr =
		{
			.src_port = deser.ReadU16(),
			.dst_port = deser.ReadU16(),
			.length = deser.ReadU16(),
			.checksum = deser.ReadU16(),
		};
		packet.mask(udp_hdr_size);

		//Update endpoints
		packet.dst.port = udp_hdr.dst_port;
		packet.src.port = udp_hdr.src_port;
		packet.proto = proto_t::Udp;

		return Socket::Receive(net_if, packet);
	}

	bool Send(NetIf& net_if, Packet& packet)
	{
		if (packet.headroom() < udp_hdr_size)
			return false;
		packet.unmask(udp_hdr_size);

		//TODO(tsharpe): Finish
		const udp_hdr_t hdr =
		{
			.src_port = packet.src.port,
			.dst_port = packet.dst.port,
			.length = static_cast<uint16_t>(packet.length()),
			.checksum = 0x0000
		};

		//Serialize udp header on packet
		Serializer ser(packet.buffer(), packet.length());
		ser.Write(hdr.src_port);
		ser.Write(hdr.dst_port);
		ser.Write(hdr.length);
		ser.Write(hdr.checksum);

		//NOTE(tsharpe): Leaving checksum empty, linux should accept

		packet.proto = proto_t::Udp;
		return IPv4::Send(net_if, packet);
	}
}
