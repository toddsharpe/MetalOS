#pragma once

#include "Net/Net.h"
#include "Net/Dhcp.h"
#include "Net/NetIf.h"
#include "Assert.h"
#include "Core/Deserializer.h"
#include "Core/Serializer.h"
#include "Core/Time.h"
#include <array>

namespace Net
{
	class DhcpClient
	{
	public:
		DhcpClient(Net::NetIf& netIf) :
			your_ip(),
			server_ip(),
			subnet(),
			m_netIf(netIf),
			m_state(),
			m_stateEntryTime(),
			m_retries(),
			transaction_id(),
			m_ack()
		{

		}

		void Dispatch(const nano_t time)
		{
			/*
			 * Run state machine.
			 */
			const state_t next_state = get_next_state(time);
			if (next_state != m_state)
			{
				printf("dhcp %d->%d\n", static_cast<int>(m_state), static_cast<int>(next_state));
				m_stateEntryTime = time;
				m_state = next_state;
			}

			/*
			 * Dispatch state.
			 */
			dispatch_state();
		}

		ipv4_addr_t your_ip;
		ipv4_addr_t server_ip;
		ipv4_addr_t subnet;

	private:
		static constexpr nano_t timeout = 3 * Second;
		static constexpr uint32_t max_retries = 5;

		enum state_t
		{
			idle,
			discover,
			offer_wait,
			request,
			ack_wait,
			finished,
			failed
		};

		state_t get_next_state(const nano_t time) const
		{
			const bool timeout = (time - m_stateEntryTime) >= DhcpClient::timeout;
			const bool maxRetries = m_retries > DhcpClient::max_retries;

			switch (m_state)
			{
				case state_t::idle:
					return state_t::discover;

				case state_t::discover:
					return state_t::offer_wait;

				case state_t::offer_wait:
					if (your_ip != Net::empty)
						return state_t::request;
					else if (timeout)
						return maxRetries ? state_t::failed : state_t::discover;
					else
						return state_t::offer_wait;

				case state_t::request:
					return state_t::ack_wait;

				case state_t::ack_wait:
					if (m_ack)
						return state_t::finished;
					else if (timeout)
						return maxRetries ? state_t::failed : state_t::request;
					else
						return state_t::ack_wait;

				case state_t::finished:
					return state_t::finished;

				case state_t::failed:
					return state_t::failed;

				default:
					Assert(false);
					return m_state;
			}
		}

		void dispatch_state()
		{
			switch (m_state)
			{
				case state_t::idle:
					break;

				case state_t::discover:
					dispatch_discover();
					break;

				case state_t::offer_wait:
					dispatch_offer_wait();
					break;

				case state_t::request:
					dispatch_request();
					break;

				case state_t::ack_wait:
					dispatch_ack_wait();
					break;

				case state_t::finished:
				case state_t::failed:
					break;

				default:
					printf("Unknown state: %d\n", static_cast<int>(m_state));
					Assert(false);
					break;
			}
		}

		void dispatch_discover()
		{
			transaction_id = rand();
			
			const uint8_t options[] =
			{
				Dhcp::options_message_type, 0x01, Dhcp::dhcp_discover,
				Dhcp::options_end
			};
			const Dhcp::dhcp_hdr_t hdr =
			{
				.opcode = Dhcp::opcode_request,
				.hardware_type = Dhcp::hardware_type_ethernet,
				.haddr_length = Dhcp::haddr_eth_len,
				.hops = 0,
				.transaction_id = transaction_id,
				.seconds = 0,
				.flags = Dhcp::flags_broadcast,
				.client_addr = {},
				.your_addr = {},
				.server_addr = {},
				.gateway_addr = {},
				.client_haddr = { .mac = m_netIf.ethernet.addr, .padding = {} },
				.server_name = {},
				.file = {},
				.magic_cookie = Dhcp::magic_cookie,
			};

			uint8_t packet[Dhcp::hdr_size + Dhcp::options_resv] = {};
			Serializer ser(packet, sizeof(packet));
			ser.Write(hdr.opcode);
			ser.Write(hdr.hardware_type);
			ser.Write(hdr.haddr_length);
			ser.Write(hdr.hops);
			ser.Write(hdr.transaction_id);
			ser.Write(hdr.seconds);
			ser.Write(hdr.flags);
			ser.Write(hdr.client_addr.addr);
			ser.Write(hdr.your_addr.addr);
			ser.Write(hdr.server_addr.addr);
			ser.Write(hdr.gateway_addr.addr);
			ser.Write(hdr.client_haddr.mac.bytes);
			ser.WriteBytes(hdr.client_haddr.padding);
			ser.WriteBytes(hdr.server_name, sizeof(hdr.server_name));
			ser.WriteBytes(hdr.file, sizeof(hdr.file));
			ser.Write(hdr.magic_cookie);
			ser.WriteBytes(options, sizeof(options));
			ser.PadTo(Dhcp::min_packet_size);

			Socket::SendUdpIf(m_netIf, { Net::broadcast, Dhcp::server_port }, Dhcp::client_port, packet, ser.Bytes());
			m_retries++;
		}

		void dispatch_offer_wait()
		{
			uint8_t buffer[Dhcp::rx_size] = {};
			Net::endpoint_t src = {};
			size_t bytes_read = 0;
			const Socket::read_t received = Socket::ReadUdp({ Net::broadcast, Dhcp::client_port }, src, buffer, sizeof(buffer), bytes_read);
			if (received != Socket::read_t::Success)
				return;

			Deserializer deser(buffer, bytes_read);
			Dhcp::dhcp_hdr_t hdr =
			{
				.opcode = deser.ReadU8(),
				.hardware_type = deser.ReadU8(),
				.haddr_length = deser.ReadU8(),
				.hops = deser.ReadU8(),
				.transaction_id = deser.ReadU32(),
				.seconds = deser.ReadU16(),
				.flags = deser.ReadU16(),
				.client_addr = {.addr = deser.ReadU32() },
				.your_addr = {.addr = deser.ReadU32() },
				.server_addr = {.addr = deser.ReadU32() },
				.gateway_addr = {.addr = deser.ReadU32() },
				.client_haddr = {},
				.server_name = {},
				.file = {},
				.magic_cookie = {}
			};
			deser.Read(hdr.client_haddr.mac.bytes);
			deser.Skip(sizeof(hdr.client_haddr.padding));
			deser.ReadBytes(hdr.server_name);
			deser.ReadBytes(hdr.file);
			hdr.magic_cookie = deser.ReadU32();
			AssertEqual(hdr.magic_cookie, Dhcp::magic_cookie);

			//Check if its the right transaction id
			if (hdr.transaction_id != transaction_id)
				return;

			const uint8_t msg_type = Dhcp::GetOption<uint8_t>(deser.Peek(), deser.Remaining(), Dhcp::options_message_type);
			if (msg_type != Dhcp::dhcp_offer)
				return;

			server_ip =
			{
				.addr = Dhcp::GetOption<uint32_t>(deser.Peek(), deser.Remaining(), Dhcp::options_dhcp_server)
			};

			subnet =
			{
				.addr = Dhcp::GetOption<uint32_t>(deser.Peek(), deser.Remaining(), Dhcp::options_subnet)
			};

			//Save
			your_ip = hdr.your_addr;
		}

		void dispatch_request()
		{
			const uint8_t options[] =
			{
				Dhcp::options_message_type, 0x01, Dhcp::dhcp_request,
				Dhcp::options_requested_ip, 0x04, your_ip.bytes[3], your_ip.bytes[2], your_ip.bytes[1], your_ip.bytes[0],
				Dhcp::options_dhcp_server, 0x04, server_ip.bytes[3], server_ip.bytes[2], server_ip.bytes[1], server_ip.bytes[0],
				Dhcp::options_end
			};

			Dhcp::dhcp_hdr_t hdr =
			{
				.opcode = Dhcp::opcode_request,
				.hardware_type = Dhcp::hardware_type_ethernet,
				.haddr_length = Dhcp::haddr_eth_len,
				.hops = 0,
				.transaction_id = transaction_id,
				.seconds = 0,
				.flags = Dhcp::flags_broadcast,
				.client_addr = {},
				.your_addr = your_ip,
				.server_addr = server_ip,
				.gateway_addr = {},
				.client_haddr = { .mac = m_netIf.ethernet.addr, .padding = {} },
				.server_name = { },
				.file = {},
				.magic_cookie = Dhcp::magic_cookie,
			};

			uint8_t packet[Dhcp::hdr_size + Dhcp::options_resv] = {};
			Serializer ser(packet, sizeof(packet));
			ser.Write(hdr.opcode);
			ser.Write(hdr.hardware_type);
			ser.Write(hdr.haddr_length);
			ser.Write(hdr.hops);
			ser.Write(hdr.transaction_id);
			ser.Write(hdr.seconds);
			ser.Write(hdr.flags);
			ser.Write(hdr.client_addr.addr);
			ser.Write(hdr.your_addr.addr);
			ser.Write(hdr.server_addr.addr);
			ser.Write(hdr.gateway_addr.addr);
			ser.Write(hdr.client_haddr.mac.bytes);
			ser.WriteBytes(hdr.client_haddr.padding);
			ser.WriteBytes(hdr.server_name, sizeof(hdr.server_name));
			ser.WriteBytes(hdr.file, sizeof(hdr.file));
			ser.Write(hdr.magic_cookie);
			ser.WriteBytes(options, sizeof(options));
			ser.PadTo(Dhcp::min_packet_size);

			Socket::SendUdpIf(m_netIf, { Net::broadcast, Dhcp::server_port }, Dhcp::client_port, packet, ser.Bytes());
			m_retries++;
		}

		void dispatch_ack_wait()
		{
			uint8_t buffer[Dhcp::rx_size] = {};
			Net::endpoint_t src = {};
			size_t bytes_read = 0;
			const Socket::read_t received = Socket::ReadUdp({ Net::broadcast, Dhcp::client_port }, src, buffer, sizeof(buffer), bytes_read);
			if (received != Socket::read_t::Success)
				return;

			Deserializer deser(buffer, sizeof(buffer));
			Dhcp::dhcp_hdr_t hdr =
			{
				.opcode = deser.ReadU8(),
				.hardware_type = deser.ReadU8(),
				.haddr_length = deser.ReadU8(),
				.hops = deser.ReadU8(),
				.transaction_id = deser.ReadU32(),
				.seconds = deser.ReadU16(),
				.flags = deser.ReadU16(),
				.client_addr = {.addr = deser.ReadU32() },
				.your_addr = {.addr = deser.ReadU32() },
				.server_addr = {.addr = deser.ReadU32() },
				.gateway_addr = {.addr = deser.ReadU32() },
			};
			deser.Read(hdr.client_haddr.mac.bytes);
			deser.Skip(sizeof(hdr.client_haddr.padding));
			deser.Read(hdr.server_name);
			deser.Read(hdr.file);
			hdr.magic_cookie = deser.ReadU32();
			Assert(hdr.magic_cookie == Dhcp::magic_cookie);

			//Check if its the right transaction id
			if (hdr.transaction_id != transaction_id)
				return;

			const uint8_t msg_type = Dhcp::GetOption<uint8_t>(deser.Peek(), deser.Remaining(), Dhcp::options_message_type);
			if (msg_type != Dhcp::dhcp_ack)
				return;

			const Net::ipv4_addr_t router =
			{
				.addr = Dhcp::GetOption<uint32_t>(deser.Peek(), deser.Remaining(), Dhcp::options_router)
			};
			UNUSED(router);

			/*
			 * Accept.
			 */
			printf("DHCP Ack: %d.%d.%d.%d, %d.%d.%d.%d from %d.%d.%d.%d\n",
				your_ip.bytes[3], your_ip.bytes[2], your_ip.bytes[1], your_ip.bytes[0],
				subnet.bytes[3], subnet.bytes[2], subnet.bytes[1], subnet.bytes[0],
				server_ip.bytes[3], server_ip.bytes[2], server_ip.bytes[1], server_ip.bytes[0]
			);
			m_netIf.ipv4.addr = your_ip;
			m_netIf.ipv4.subnet_mask = subnet;

			m_ack = true;
		}

		Net::NetIf& m_netIf;

		/*
		 * State machine.
		 */
		state_t m_state;
		nano_t m_stateEntryTime;
		uint32_t m_retries;

		/*
		 * DHCP info.
		 */
		uint32_t transaction_id;
		bool m_ack;
	};
}
