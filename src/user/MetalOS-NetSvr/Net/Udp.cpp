#include "Net/Net.h"
#include "Net/NetIf.h"
#include "Lib/Deserializer.h"
#include "Lib/Serializer.h"

namespace Net::Udp
{
	//The checksum's pseudo-header (RFC 768): the addresses IPv4 carries, the protocol and the datagram's length, summed ahead of the datagram
	static uint32_t PseudoChecksum(const endpoint_t src, const endpoint_t dst, const uint16_t length)
	{
		uint8_t pseudo[12] = {};
		Serializer ser(pseudo);
		ser.Write(src.addr.addr);
		ser.Write(dst.addr.addr);
		ser.Write<uint8_t>(0);
		ser.Write(ip_proto_udp);
		ser.Write(length);
		return checksum_add(0, pseudo, sizeof(pseudo));
	}

	bool Receive(NetIf& net_if, Packet& packet)
	{
		if (packet.length() < udp_hdr_size)
		{
			net_if.ipv4.rx_dropped++;
			return false;
		}

		Deserializer deser(packet.buffer(), packet.length());
		const udp_hdr_t udp_hdr =
		{
			.src_port = deser.ReadU16(),
			.dst_port = deser.ReadU16(),
			.length = deser.ReadU16(),
			.checksum = deser.ReadU16(),
		};
		//Its length against the bytes that arrived, and its checksum when the sender computed one (zero means it didn't, which IPv4 allows)
		if (udp_hdr.length < udp_hdr_size || udp_hdr.length > packet.length())
		{
			net_if.ipv4.rx_dropped++;
			return false;
		}
		packet.trim(udp_hdr.length);
		if (udp_hdr.checksum != 0 && checksum_finalize(checksum_add(PseudoChecksum(packet.src, packet.dst, udp_hdr.length), packet.buffer(), packet.length())) != 0)
		{
			net_if.ipv4.rx_dropped++;
			return false;
		}
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

		//Over the pseudo-header and the whole datagram; one that sums to zero goes as 0xFFFF, since zero means none
		uint16_t check = checksum_finalize(checksum_add(PseudoChecksum(packet.src, packet.dst, hdr.length), packet.buffer(), packet.length()));
		if (check == 0)
			check = 0xFFFF;
		ser.Reset();
		ser.Skip(udp_checksum_offset);
		ser.Write(check);

		packet.proto = proto_t::Udp;
		return IPv4::Send(net_if, packet);
	}
}
