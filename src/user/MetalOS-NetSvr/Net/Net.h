#pragma once

#include "Assert.h"
#include <cstdint>
#include <cstring>
#include "Lib/ByteSwap.h"
#include "Lib/Span.h"
#include "Lib/Time.h"

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
		EchoRequest
	};

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
	static constexpr uint16_t ipv4_fragmented = 0x3FFF; //more fragments, or a fragment offset: this stack doesn't reassemble
	static constexpr size_t ipv4_hdr_size = 20;

	constexpr ipv4_addr_t build_ip(const uint8_t a0, const uint8_t a1, const uint8_t a2, const uint8_t a3)
	{
		return { .bytes = {a3, a2, a1, a0} };
	}

	static constexpr ipv4_addr_t empty = build_ip(0, 0, 0, 0);
	static constexpr ipv4_addr_t broadcast = build_ip(0xFF, 0xFF, 0xFF, 0xFF);
	static constexpr ipv4_addr_t subnet_32 = build_ip(0xFF, 0xFF, 0xFF, 0xFF);

	constexpr bool is_network(const ipv4_addr_t ip_1, const ipv4_addr_t ip_2, const ipv4_addr_t subnet)
	{
		return (ip_1.addr & subnet.addr) == (ip_2.addr & subnet.addr);
	}

	constexpr bool is_multicast(const ipv4_addr_t address)
	{
		const uint8_t high = address.addr >> 24;
		return (high & 0xF0) == 0xE0;
	}

	//The directed broadcast of the interface's network (all host bits set); a /31 or /32 has none
	constexpr bool is_subnet_broadcast(const ipv4_addr_t address, const ipv4_addr_t if_addr, const ipv4_addr_t subnet)
	{
		const uint32_t host = ~subnet.addr;
		return host > 1 && is_network(address, if_addr, subnet) && (address.addr & host) == host;
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
	static constexpr milli_t arp_retry_time = 1000; //An unanswered request (e.g. an unreachable host) may be sent again after this

	//The interface's one outstanding request; it expires, so a peer that never answers doesn't stop ARP for good
	struct arp_request_t
	{
		bool pending;
		milli_t sent;
		ipv4_addr_t target;

		bool expired(const milli_t now) const
		{
			return now - sent >= arp_retry_time;
		}

		//False while a request sent less than arp_retry_time ago is unanswered
		bool try_begin(const milli_t now)
		{
			if (pending && !expired(now))
				return false;

			pending = true;
			sent = now;
			return true;
		}

		void end()
		{
			pending = false;
		}
	};

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
	static constexpr uint8_t icmp_type_echo_request = 0x08;

	struct icmp_hdr_t
	{
		uint8_t type;
		uint8_t code;
		uint16_t checksum;
	};
	static constexpr size_t icmp_hdr_checksum_offset = 2;
	static constexpr size_t icmp_hdr_size = 4;

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

	//The internet checksum (RFC 1071); checksum_add sums in parts, as UDP's pseudo-header and datagram are
	uint16_t checksum(const void* const buffer, const size_t length);
	uint32_t checksum_add(const uint32_t current, const void* const buffer, const size_t length);
	uint16_t checksum_finalize(const uint32_t current);
	bool checksum_valid(const void* const buffer, const size_t length);

	uint16_t rand_port();

	/*
	 * Packet pools.
	 */
	static constexpr size_t pool_size = 64 * 1024;

	//A driver receives each frame here and frees it unless a socket took it
	buffer_t alloc_rx(const size_t bytes);
	void free_rx(const buffer_t buffer);

	//A sender builds here; the driver frees what it sends, and a sender whose Send fails frees it
	buffer_t alloc_tx(const size_t bytes);
	void free_tx(const buffer_t buffer);

	//What the stack loses that belongs to no one interface
	struct stack_stats_t
	{
		uint32_t pool_rx_empty;
		uint32_t pool_tx_empty;
		uint32_t socket_rx_full;
		uint32_t socket_tx_full;
	};
	extern stack_stats_t stack_stats;

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

		//Each loop: the frame waiting on ARP goes once its next hop resolves, or is dropped once the request expires
		void Dispatch(NetIf& net_if);
	}

	namespace TunEth
	{
		bool Receive(NetIf& net_if, Packet& packet);
		bool Send(NetIf& net_if, Packet& packet);
	}

	namespace IPv4
	{
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
		static constexpr size_t CacheSize = 8;

		void Update(const ipv4_addr_t& ip_addr, const eth_mac_t& eth_addr);
		bool Lookup(const ipv4_addr_t& ipAddr, eth_mac_t& eth_addr);
	}

	namespace Udp
	{
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

	//Datagram sockets: UDP, and ICMP echo, where the stack writes the socket's port as the echo id and delivers replies by it
	namespace Socket
	{
		static constexpr int closed_fd = 0;

		int Create(const proto_t proto);
		bool Bind(const int fd, const endpoint_t endpoint, NetIf* const net_if = nullptr);
		bool SendTo(const int fd, const void* const buffer, const size_t length, const endpoint_t dst);
		bool Close(const int fd);

		//A datagram larger than the buffer is dropped, not truncated
		bool Recv(const int fd, void* const buffer, const size_t length, size_t& bytes_read, endpoint_t* const src);

		//Each loop: send what a busy driver refused before
		void Dispatch();

		//From Udp and Icmp: queue the datagram on the socket it matches, which keeps its rx-pool backing
		bool Receive(NetIf& net_if, Packet& packet);
	}

	bool AddNetIf(NetIf& net_if);
	Span<NetIf* const> GetNetIfs();
	bool HasNetIf(NetIf& net_if);

	typedef bool (*handler_tx)(NetIf& interface, Packet& packet);

	struct NetLayer
	{
		constexpr NetLayer() : Receive(), Send(), HeaderSize()
		{

		}
		
		constexpr NetLayer(const handler_tx _rx, const handler_tx _tx, const size_t _header_size = 0) : Receive(_rx), Send(_tx), HeaderSize(_header_size)
		{

		}

		const handler_tx Receive;
		const handler_tx Send;
		const size_t HeaderSize;
	};
}
