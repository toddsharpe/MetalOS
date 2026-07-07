#include "user/MetalOS.h"
#include <stdio.h>
#include <string.h>

//Usermode ping over a raw ICMP socket (SOCK_RAW/IPPROTO_ICMP). The app builds the full
//ICMP echo request (incl. checksum); the kernel only adds the IP header. Replies arrive
//as complete ICMP messages, matched by id/seq.

static uint16_t IcmpChecksum(const uint8_t* data, size_t len)
{
	uint32_t sum = 0;
	size_t i = 0;
	for (; i + 1 < len; i += 2)
		sum += ((uint32_t)data[i] << 8) | data[i + 1];
	if (i < len)
		sum += (uint32_t)data[i] << 8;
	while (sum >> 16)
		sum = (sum & 0xFFFF) + (sum >> 16);
	return (uint16_t)~sum;
}

static bool ParseIp(const char* s, in_addr* out)
{
	uint32_t oct[4] = {};
	int idx = 0;
	uint32_t cur = 0;
	bool any = false;
	for (const char* p = s; ; p++)
	{
		if (*p >= '0' && *p <= '9') { cur = cur * 10 + (uint32_t)(*p - '0'); any = true; }
		else if (*p == '.' || *p == '\0')
		{
			if (!any || cur > 255 || idx > 3) return false;
			oct[idx++] = cur; cur = 0; any = false;
			if (*p == '\0') break;
		}
		else return false;
	}
	if (idx != 4) return false;
	//Network byte order: memory [o0,o1,o2,o3].
	out->s_addr = oct[0] | (oct[1] << 8) | (oct[2] << 16) | (oct[3] << 24);
	return true;
}

int main(int argc, char** argv)
{
	//Target = argv[1] if a valid dotted IP, else the gateway derived from interface 0.
	in_addr target = {};
	if (!(argc > 1 && ParseIp(argv[1], &target)))
	{
		//Enumeration is a kernel syscall; the IPv4 config is owned by netstack (GetInterfaceIp).
		InterfaceInfo ifaces[4] = {};
		size_t ifcount = 0;
		in_addr addr = {}, subnet = {}, gateway = {};
		if (GetInterfaces(ifaces, 4, &ifcount) != SyscallResult::Success || ifcount == 0 ||
			GetInterfaceIp(ifaces[0].index, &addr, &subnet) != SyscallResult::Success || addr.s_addr == 0)
		{
			printf("ping: no configured interface (run dhcp.exe first)\n");
			return 1;
		}
		//Prefer the DHCP-provided gateway; fall back to a network|.1 guess.
		GetGateway(ifaces[0].index, &gateway);
		if (gateway.s_addr != 0)
			target = gateway;
		else
			target.s_addr = (addr.s_addr & subnet.s_addr) | (1u << 24);
	}

	const uint8_t* t = (const uint8_t*)&target.s_addr;
	printf("ping %u.%u.%u.%u\n", t[0], t[1], t[2], t[3]);

	HSocket sock = SocketCreate(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sock == INVALID_SOCKET)
	{
		printf("  socket create failed\n");
		return 1;
	}

	sockaddr_in dst = {};
	dst.sin_family = AF_INET;
	dst.sin_port = 0;
	dst.sin_addr = target;

	const uint16_t id = 0x4d53; //'MS'
	const char* payload = "MetalOS ping";
	const size_t payloadLen = 12;

	//Warm up ARP: the first unicast send resolves asynchronously.
	{
		uint8_t warm[8 + 12] = { 8, 0 };
		SocketSendTo(sock, warm, sizeof(warm), &dst);
		Sleep(300);
	}

	int received = 0;
	for (uint16_t seq = 1; seq <= 4; seq++)
	{
		uint8_t msg[8 + 12] = {};
		msg[0] = 8; //echo request
		msg[1] = 0; //code
		msg[4] = (uint8_t)(id >> 8);  msg[5] = (uint8_t)id;
		msg[6] = (uint8_t)(seq >> 8); msg[7] = (uint8_t)seq;
		memcpy(msg + 8, payload, payloadLen);
		const size_t len = 8 + payloadLen;
		const uint16_t ck = IcmpChecksum(msg, len);
		msg[2] = (uint8_t)(ck >> 8); msg[3] = (uint8_t)ck;

		const milli_t t0 = GetTickCount();
		if (SocketSendTo(sock, msg, len, &dst) != SyscallResult::Success)
		{
			printf("  seq=%u send failed (arp?)\n", seq);
			Sleep(500);
			continue;
		}

		uint8_t reply[256] = {};
		size_t got = 0;
		sockaddr_in from = {};
		const SyscallResult r = SocketRecvFrom(sock, reply, sizeof(reply), &got, &from, 1000);
		if (r == SyscallResult::Success && got >= 8 && reply[0] == 0 /*echo reply*/ &&
			reply[4] == (uint8_t)(id >> 8) && reply[5] == (uint8_t)id &&
			reply[6] == (uint8_t)(seq >> 8) && reply[7] == (uint8_t)seq)
		{
			const milli_t rtt = GetTickCount() - t0;
			const uint8_t* f = (const uint8_t*)&from.sin_addr.s_addr;
			printf("  reply from %u.%u.%u.%u: seq=%u time=%ums\n",
				f[0], f[1], f[2], f[3], seq, (unsigned)rtt);
			received++;
		}
		else
		{
			printf("  seq=%u timeout\n", seq);
		}
		Sleep(500);
	}

	printf("ping: %d/4 replies\n", received);
	SocketClose(sock);
	return 0;
}
