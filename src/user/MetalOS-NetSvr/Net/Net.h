#pragma once

#include "Assert.h"
#include <cstdint>
#include <cstring>
#include "Lib/ByteSwap.h"

namespace Net
{
	/*
	 * Forward declarataions.
	 */
	struct NetIf;
	struct Packet;
	
	/*
	 * Implementation.
	 */
	enum class proto_t
	{
		None,
		Eth,
		Arp,
		Ipv4,
		Udp,
		Icmp,
		EchoReply,
		EchoRequest,
		DestUnreachable
	};

	static constexpr char proto_None[] = "None";
	static constexpr char proto_Eth[] = "Eth";
	static constexpr char proto_Arp[] = "Arp";
	static constexpr char proto_Ipv4[] = "Ipv4";
	static constexpr char proto_Udp[] = "Udp";
	static constexpr char proto_Icmp[] = "Icmp";
	static constexpr char proto_EchoReply[] = "EchoReply";
	static constexpr char proto_EchoRequest[] = "EchoRequest";

	constexpr const char* to_string(const proto_t proto)
	{
		switch (proto)
		{
			case proto_t::Eth:
				return proto_Eth;
			case proto_t::Arp:
				return proto_Arp;
			case proto_t::Ipv4:
				return proto_Ipv4;
			case proto_t::Udp:
				return proto_Udp;
			case proto_t::Icmp:
				return proto_Icmp;
			case proto_t::EchoReply:
				return proto_EchoReply;
			case proto_t::EchoRequest:
				return proto_EchoRequest;
			default:
				Assert(false);
				return proto_None;
		}
	}

	enum class l2_t
	{
		Ethernet,
		Tun,
		Serial,
		NoEth
	};

	static constexpr size_t mtu = 1500;
	static constexpr size_t l2_res = 14;
	static constexpr size_t buffer_size = mtu + l2_res;

	/*
	 * Ethernet.
	 */
	static constexpr uint16_t eth_type_ipv4 = 0x0800;
	static constexpr uint16_t eth_type_arp = 0x0806;

	struct eth_mac_t
	{
		uint8_t bytes[6];

		bool operator==(const eth_mac_t& other) const
		{
			return memcmp(bytes, other.bytes, sizeof(eth_mac_t::bytes)) == 0;
		}

		bool operator!=(const eth_mac_t& other) const
		{
			return !(*this == other);
		}
	};
	static constexpr eth_mac_t eth_broadcast = { .bytes = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF} };
	static constexpr eth_mac_t eth_empty = { .bytes = {} };

	constexpr eth_mac_t build_mac(const uint8_t a0, const uint8_t a1, const uint8_t a2, const uint8_t a3, const uint8_t a4, const uint8_t a5)
	{
		return { .bytes = {a5, a4, a3, a2, a1, a0} };
	}

	struct eth_hdr_t
	{
		eth_mac_t dst;
		eth_mac_t src;
		uint16_t type;
	};
	static_assert(sizeof(eth_hdr_t) <= l2_res);
	static constexpr size_t eth_hdr_size = 14;

	constexpr uint16_t get_eth_type(const proto_t proto)
	{
		switch (proto)
		{
			case proto_t::Arp:
				return eth_type_arp;

			case proto_t::Ipv4:
				return eth_type_ipv4;

			default:
				Assert(false);
				return 0;
		}
	}

	/*
	 * Linux TunEth.
	 */
	struct tun_hdr_t
	{
		uint16_t flags;
		uint16_t ether_type;
	};
	static constexpr size_t tun_hdr_size = 4;

	/*
	 * IPv4.
	 */
	static constexpr uint8_t ip_proto_icmp = 0x01;
	static constexpr uint8_t ip_proto_igmp = 0x02;
	static constexpr uint8_t ip_proto_tcp = 0x06;
	static constexpr uint8_t ip_proto_udp = 0x11;

	constexpr uint8_t get_ipv4_proto(const proto_t proto)
	{
		switch (proto)
		{
			case proto_t::None:
				return 0;

			case proto_t::Udp:
				return ip_proto_udp;

			case proto_t::Icmp:
				return ip_proto_icmp;

			default:
				return 0;
		}
	}

	struct ipv4_addr_t
	{
		union
		{
			uint32_t addr;
			uint8_t bytes[4];
		};

		bool operator==(const ipv4_addr_t& other) const
		{
			return addr == other.addr;
		}

		bool operator!=(const ipv4_addr_t& other) const
		{
			return !(*this == other);
		}
	};

	struct ipv4_hdr_t
	{
		uint8_t ihl_version;
		uint8_t dscp;
		uint16_t length;
		uint16_t id;
		uint16_t fragment_flags;
		uint8_t ttl;
		uint8_t proto;
		uint16_t checksum;
		ipv4_addr_t src;
		ipv4_addr_t dst;
	};
	static constexpr size_t ipv4_hdr_checksum_offset = 10;
	static constexpr size_t ipv4_hdr_size = 20;

	constexpr ipv4_addr_t build_ip(const uint8_t a0, const uint8_t a1, const uint8_t a2, const uint8_t a3)
	{
		return { .bytes = {a3, a2, a1, a0} };
	}

	constexpr ipv4_addr_t empty = build_ip(0, 0, 0, 0);
	constexpr ipv4_addr_t localhost = build_ip(127, 0, 0, 1);
	constexpr ipv4_addr_t server_ip = build_ip(192, 168, 0, 0);
	constexpr ipv4_addr_t telem_ip = build_ip(239, 9, 9, 9);
	constexpr ipv4_addr_t broadcast = build_ip(0xFF, 0xFF, 0xFF, 0xFF);

	constexpr ipv4_addr_t subnet_16 = build_ip(0xFF, 0xFF, 0, 0);
	constexpr ipv4_addr_t subnet_24 = build_ip(0xFF, 0xFF, 0xFF, 0);
	constexpr ipv4_addr_t subnet_32 = build_ip(0xFF, 0xFF, 0xFF, 0xFF);

	constexpr bool is_network(const ipv4_addr_t ip_1, const ipv4_addr_t ip_2, const ipv4_addr_t subnet)
	{
		return (ip_1.addr & subnet.addr) == (ip_2.addr & subnet.addr);
	}

	constexpr bool is_multicast(const ipv4_addr_t address)
	{
		const uint8_t high = address.addr >> 24;
		return (high & 0xF0) == 0xE0;
	}

	/*
	 * ARP.
	 */
	static constexpr uint16_t arp_op_request = 1;
	static constexpr uint16_t arp_op_reply = 2;

	struct arp_t
	{
		uint16_t hw_type;
		uint16_t proto;
		uint8_t hw_len;
		uint8_t proto_len;
		uint16_t op;
		eth_mac_t sender_hw;
		ipv4_addr_t sender_proto;
		eth_mac_t target_hw;
		ipv4_addr_t target_proto;
	};

	static constexpr size_t arp_hdr_size = 28;

	/*
	 * UDP.
	 */
	struct udp_hdr_t
	{
		uint16_t src_port;
		uint16_t dst_port;
		uint16_t length;
		uint16_t checksum;
	};
	static constexpr size_t udp_checksum_offset = 6;
	static constexpr size_t udp_hdr_size = 8;

	/*
	 * ICMP.
	 */
	static constexpr uint8_t icmp_type_echo_reply = 0x00;
	static constexpr uint8_t icmp_type_dest_unreachable = 0x03;
	static constexpr uint8_t icmp_type_echo_request = 0x08;

	struct icmp_hdr_t
	{
		uint8_t type;
		uint8_t code;
		uint16_t checksum;
	};
	static constexpr size_t icmp_hdr_checksum_offset = 2;
	static constexpr size_t icmp_hdr_size = 4;

	struct dest_unreachable_t
	{
		uint32_t unused;
	};
	static constexpr size_t dst_unreachable_size = 4;

	struct echo_hdr_t
	{
		uint16_t id;
		uint16_t seq_num;
	};
	static constexpr size_t echo_hdr_size = 4;

	constexpr uint8_t get_icmp_type(const proto_t proto)
	{
		switch (proto)
		{
			case proto_t::EchoReply:
				return icmp_type_echo_reply;

			case proto_t::EchoRequest:
				return icmp_type_echo_request;

			default:
				Assert(false);
				return 0;
		}
	}

	/*
	 * Stack.
	 */

	struct endpoint_t
	{
		ipv4_addr_t addr;
		uint16_t port;

		bool operator==(const endpoint_t& other) const
		{
			return addr == other.addr && port == other.port;
		}

		bool operator!=(const endpoint_t& other) const
		{
			return !(*this == other);
		}
	};

	uint16_t checksum(const void* const buffer, const size_t length);
	uint32_t checksum_add(const uint32_t current, const void* const buffer, const size_t length);
	uint16_t checksum_finalize(const uint32_t current);
	bool checksum_valid(const void* const buffer, const size_t length);

	uint16_t rand_port();

	/*
	 * Layers.
	 */
	namespace NoEth
	{
		bool Receive(NetIf& net_if, Packet& packet);
		bool Send(NetIf& net_if, Packet& packet);
	}
	namespace Serial
	{
		bool Receive(NetIf& net_if, Packet& packet);
		bool Send(NetIf& net_if, Packet& packet);
	}
	namespace Ethernet
	{
		bool Receive(NetIf& net_if, Packet& packet);
		bool Send(NetIf& net_if, Packet& packet);
		void TxFlush(NetIf& net_if);
	}

	namespace TunEth
	{
		bool Receive(NetIf& net_if, Packet& packet);
		bool Send(NetIf& net_if, Packet& packet);
	}

	namespace IPv4
	{
		bool Meta(Packet &packet);
		bool Receive(NetIf &net_if, Packet &packet);
		bool Send(NetIf &net_if, Packet &packet);
	}

	namespace Arp
	{
		bool Receive(NetIf& net_if, Packet& packet);
		bool Send(NetIf& net_if, Packet& packet);
	}

	namespace ArpCache
	{
		bool Contains(const ipv4_addr_t& ip_addr);
		bool Update(const ipv4_addr_t& ip_addr, const eth_mac_t& eth_addr);
		bool Lookup(const ipv4_addr_t& ipAddr, eth_mac_t& eth_addr);

		void Display();
	}

	namespace Udp
	{
		bool Meta(Packet& packet);
		bool Receive(NetIf& net_if, Packet& packet);
		bool Send(NetIf& net_if, Packet& packet);
	}

	namespace Icmp
	{
		bool Receive(NetIf& net_if, Packet& packet);
		bool Send(NetIf& net_if, Packet& packet);
	}

	namespace Router
	{
		NetIf& Resolve(const Net::endpoint_t& dst);
		bool AddRoute(NetIf& net_if, const ipv4_addr_t dst, const ipv4_addr_t subnet_mask);
	}

	namespace Socket
	{
		enum class read_t
		{
			Empty,
			Success,
			Unreachable,
			Failure
		};
		
		bool Receive(NetIf& net_if, Packet& packet);

		bool SendUdp(const endpoint_t dst, const uint16_t src_port, const void* const buffer, const size_t length);
		bool SendIcmpRaw(const endpoint_t dst, const void* const buffer, const size_t length);
	}

	bool AddNetIf(NetIf& net_if);

	typedef bool (*handler_tx)(NetIf& interface, Packet& packet);

	struct NetLayer
	{
		NetLayer() : Receive(), Send(), HeaderSize()
		{

		}
		
		NetLayer(const handler_tx _rx, const handler_tx _tx, const size_t _header_size = 0) : Receive(_rx), Send(_tx), HeaderSize(_header_size)
		{

		}

		const handler_tx Receive;
		const handler_tx Send;
		const size_t HeaderSize;
	};
}

inline uint16_t htons(const uint16_t host)
{
	return ByteSwap<uint16_t>(host);
}
