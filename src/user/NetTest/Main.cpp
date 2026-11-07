#include "user/MetalOS.h"
#include "user/NetUtil.h"
#include <stdio.h>

int main(int argc, char** argv)
{
	UNUSED(argc);
	UNUSED(argv);
	printf("NetTest: UDP datagram socket demo\n");

	//Interface 0 must be configured (run dhcp.exe first). Show its address and derive the
	//gateway (network | .1) as a real, on-subnet unicast target that answers ARP.
	InterfaceInfo ifaces[4] = {};
	size_t ifcount = 0;
	if (GetInterfaces(ifaces, 4, &ifcount) != SyscallResult::Success || ifcount == 0)
	{
		printf("  no interfaces\n");
		return 1;
	}
	const InterfaceInfo& nic = ifaces[0];

	//The IPv4 config is owned by netstack (GetInterfaceIp/GetGateway), not the kernel enumeration.
	in_addr addr = {}, subnet = {};
	GetInterfaceIp(nic.index, &addr, &subnet);
	const uint8_t* a = (const uint8_t*)&addr.s_addr;
	const uint8_t* s = (const uint8_t*)&subnet.s_addr;
	printf("  if%u %02x:%02x:%02x:%02x:%02x:%02x  ip %u.%u.%u.%u / %u.%u.%u.%u\n",
		nic.index, nic.mac[0], nic.mac[1], nic.mac[2], nic.mac[3], nic.mac[4], nic.mac[5],
		a[0], a[1], a[2], a[3], s[0], s[1], s[2], s[3]);

	if (addr.s_addr == 0)
	{
		printf("  interface has no IP - run dhcp.exe first\n");
		return 1;
	}

	const in_addr gw = GatewayOrGuess(nic.index, addr, subnet);
	PrintIp("  gateway ", gw);

	HSocket sock = SocketCreate(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (sock == INVALID_SOCKET)
	{
		printf("  SocketCreate failed\n");
		return 1;
	}

	sockaddr_in local = {};
	local.sin_family = AF_INET;
	local.sin_port = Htons(12345);
	local.sin_addr.s_addr = 0; //INADDR_ANY
	printf("  bind     = %d\n", (int)SocketBind(sock, &local));

	//Send to the gateway. The first unicast send triggers async ARP and may report
	//failure until the ARP reply arrives, so retry a few times.
	sockaddr_in dst = {};
	dst.sin_family = AF_INET;
	dst.sin_port = Htons(9); //discard
	dst.sin_addr = gw;
	const char* msg = "hello from MetalOS";

	SyscallResult r = SyscallResult::Failed;
	for (int attempt = 0; attempt < 5; attempt++)
	{
		r = SocketSendTo(sock, msg, 18, &dst);
		printf("  sendto[%d] = %d\n", attempt, (int)r);
		if (r == SyscallResult::Success)
			break;
		Sleep(300); //let ARP resolve
	}

	SocketClose(sock);
	return 0;
}
