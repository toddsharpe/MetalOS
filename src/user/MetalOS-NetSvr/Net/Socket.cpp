#include "Net/Net.h"
#include "Net/NetIf.h"
#include "Net/Packet.h"
#include "Lib/CircularBuffer.h"
#include "Lib/Serializer.h"
#include "Assert.h"

//TODO(tsharpe): Implement rand for this method
uint16_t Net::rand_port()
{
	static uint16_t port = 1024;
	port++;
	return (port % UINT16_MAX);
}

//Datagram sockets, ported from Nighthawk's socket_t: a fixed table, datagrams queued as views into the rx pool, and sends queued until a driver takes them
namespace Net::Socket
{
	namespace
	{
		static constexpr size_t SocketCount = 64;
		static constexpr size_t RxEntries = 8;
		static constexpr size_t TxEntries = 4;
		static constexpr int FirstFd = 100;

		//The echo id's offset in an ICMP echo message: after the type, code and checksum
		static constexpr size_t EchoIdOffset = icmp_hdr_size;

		struct tx_entry_t
		{
			buffer_t buffer;
			size_t header_size;
			size_t length;
			endpoint_t dst;
			NetIf* dst_if;
		};

		struct rx_entry_t
		{
			buffer_t payload;
			endpoint_t src;
			//What free_rx gets back: the whole allocation the payload is a view into
			buffer_t allocation;
		};

		//A free slot is all zero, its fd closed_fd
		struct socket_t
		{
			int fd;
			proto_t proto;
			endpoint_t endpoint;
			NetIf* bound_if;
			CircularBuffer<rx_entry_t, RxEntries> rx;
			CircularBuffer<tx_entry_t, TxEntries> tx;
			uint32_t rx_bytes;
			uint32_t tx_bytes;
		};

		static socket_t m_sockets[SocketCount] = {};
		static int m_nextFd = FirstFd;

		socket_t* Get(const int fd)
		{
			if (fd == closed_fd)
				return nullptr;

			for (socket_t& socket : m_sockets)
			{
				if (socket.fd == fd)
					return &socket;
			}
			return nullptr;
		}

		//An unbound socket matches nothing; a bound one, its interface, address and port where it names them
		bool Match(const NetIf& src_if, const proto_t proto, const endpoint_t dst, const socket_t& socket)
		{
			if (socket.proto != proto)
				return false;

			if (socket.bound_if != nullptr && socket.bound_if != &src_if)
				return false;

			if (socket.endpoint.addr != Net::empty && socket.endpoint.addr != dst.addr)
				return false;

			if (socket.endpoint.port != 0 && socket.endpoint.port != dst.port)
				return false;

			return socket.bound_if != nullptr || socket.endpoint.addr != Net::empty || socket.endpoint.port != 0;
		}

		bool Egress(const socket_t& socket, const tx_entry_t& entry)
		{
			Packet packet(entry.buffer);
			packet.mask(entry.header_size);
			packet.src = { entry.dst_if->ipv4.addr, socket.endpoint.port };
			packet.dst = entry.dst;

			if (socket.proto == proto_t::Udp)
			{
				packet.proto = proto_t::Udp;
				return Udp::Send(*entry.dst_if, packet);
			}

			//The app's echo message under a header Icmp::Send rewrites, checksum included
			packet.mask(icmp_hdr_size);
			packet.proto = proto_t::EchoRequest;
			return Icmp::Send(*entry.dst_if, packet);
		}

		void Flush(socket_t& socket)
		{
			while (!socket.tx.IsEmpty())
			{
				const tx_entry_t& entry = socket.tx.Peek();

				//A down link would refuse it every loop and jam the rest behind it, so it drops here
				if (!entry.dst_if->driver.IsLinkUp())
				{
					free_tx(entry.buffer);
					socket.tx.Dequeue();
					continue;
				}

				//An up link that refuses is busy, so the datagram waits for the next loop
				if (!Egress(socket, entry))
					break;

				socket.tx_bytes += static_cast<uint32_t>(entry.length);
				socket.tx.Dequeue();
			}
		}
	}

	int Create(const proto_t proto)
	{
		Assert(proto == proto_t::Udp || proto == proto_t::Icmp);

		for (socket_t& socket : m_sockets)
		{
			if (socket.fd != closed_fd)
				continue;

			socket.fd = m_nextFd++;
			socket.proto = proto;
			return socket.fd;
		}

		return closed_fd;
	}

	bool Bind(const int fd, const endpoint_t endpoint, NetIf* const net_if)
	{
		socket_t* const socket = Get(fd);
		if (!socket)
			return false;

		socket->endpoint = endpoint;
		if (socket->endpoint.port == 0)
			socket->endpoint.port = rand_port();
		socket->bound_if = net_if;
		return true;
	}

	bool SendTo(const int fd, const void* const buffer, const size_t length, const endpoint_t dst)
	{
		socket_t* const socket = Get(fd);
		if (!socket)
			return false;

		//UDP adds its header; an ICMP socket's app sends a whole echo request
		const bool udp = socket->proto == proto_t::Udp;
		const size_t transport_size = udp ? udp_hdr_size : 0;
		if (length > mtu - ipv4_hdr_size - transport_size)
			return false;
		if (!udp && (length < icmp_hdr_size + echo_hdr_size || static_cast<const uint8_t*>(buffer)[0] != icmp_type_echo_request))
			return false;

		if (socket->tx.IsFull())
		{
			stack_stats.socket_tx_full++;
			return false;
		}

		NetIf& dst_if = socket->bound_if ? *socket->bound_if : Router::Resolve(dst);

		//A source address is needed, except for broadcast (DHCP DISCOVER goes from 0.0.0.0 before there is one)
		if (dst_if.ipv4.addr == Net::empty && dst.addr != Net::broadcast)
			return false;

		//An unbound socket takes an ephemeral port, so replies come back to it; for ICMP it is the echo id
		if (socket->endpoint.port == 0)
			socket->endpoint.port = rand_port();

		const size_t header_size = get_l2(dst_if.l2).HeaderSize + ipv4_hdr_size + transport_size;
		const buffer_t allocated = alloc_tx(header_size + length);
		if (!allocated)
			return false;

		memcpy(allocated.Data + header_size, buffer, length);
		if (!udp)
		{
			Serializer ser(allocated.Data + header_size + EchoIdOffset, length - EchoIdOffset);
			ser.Write(socket->endpoint.port);
		}

		const tx_entry_t entry =
		{
			.buffer = allocated,
			.header_size = header_size,
			.length = length,
			.dst = dst,
			.dst_if = &dst_if
		};
		if (!socket->tx.Enqueue(entry))
		{
			free_tx(allocated);
			return false;
		}

		//Attempt to send now; a busy driver leaves it for Dispatch
		Flush(*socket);
		return true;
	}

	bool Recv(const int fd, void* const buffer, const size_t length, size_t& bytes_read, endpoint_t* const src)
	{
		socket_t* const socket = Get(fd);
		if (!socket)
			return false;

		while (!socket->rx.IsEmpty() && socket->rx.Peek().payload.Size > length)
		{
			const rx_entry_t oversize = socket->rx.Dequeue();
			socket->rx_bytes -= static_cast<uint32_t>(oversize.payload.Size);
			free_rx(oversize.allocation);
		}

		if (socket->rx.IsEmpty())
			return false;

		const rx_entry_t entry = socket->rx.Dequeue();
		memcpy(buffer, entry.payload.Data, entry.payload.Size);
		bytes_read = entry.payload.Size;
		if (src)
			*src = entry.src;

		free_rx(entry.allocation);
		socket->rx_bytes -= static_cast<uint32_t>(bytes_read);
		return true;
	}

	bool Close(const int fd)
	{
		socket_t* const socket = Get(fd);
		if (!socket)
			return false;

		while (!socket->rx.IsEmpty())
			free_rx(socket->rx.Dequeue().allocation);

		while (!socket->tx.IsEmpty())
			free_tx(socket->tx.Dequeue().buffer);

		*socket = {};
		return true;
	}

	void Dispatch()
	{
		for (socket_t& socket : m_sockets)
		{
			if (socket.fd != closed_fd)
				Flush(socket);
		}
	}

	bool Receive(NetIf& net_if, Packet& packet)
	{
		for (socket_t& socket : m_sockets)
		{
			if (socket.fd == closed_fd || !Match(net_if, packet.proto, packet.dst, socket))
				continue;

			//An empty datagram has nothing to hand up
			if (packet.length() == 0)
				return false;

			if (socket.rx.IsFull())
			{
				stack_stats.socket_rx_full++;
				return false;
			}

			//The driver's rx-pool allocation is the socket's now, the payload a view into it
			const rx_entry_t entry =
			{
				.payload = { packet.buffer(), packet.length() },
				.src = packet.src,
				.allocation = packet.backing
			};
			socket.rx.Enqueue(entry);
			packet.taken = true;
			socket.rx_bytes += static_cast<uint32_t>(packet.length());
			return true;
		}

		return false;
	}
}
