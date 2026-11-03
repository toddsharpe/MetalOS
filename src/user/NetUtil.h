#pragma once

#include "user/MetalOS.h"
#include <stdio.h>

//Helpers shared by the network apps; sockaddr_in and in_addr fields are network byte order

inline uint16_t Htons(const uint16_t value)
{
	return (uint16_t)((value >> 8) | (value << 8));
}

//Prints label then the address in dotted form (network byte order keeps the first octet first in memory)
inline void PrintIp(const char* const label, const in_addr address)
{
	const uint8_t* const b = (const uint8_t*)&address.s_addr;
	printf("%s%u.%u.%u.%u\n", label, b[0], b[1], b[2], b[3]);
}

//The interface's DHCP-provided gateway, or a network|.1 guess when it has none
inline in_addr GatewayOrGuess(const uint32_t interfaceIndex, const in_addr address, const in_addr subnet)
{
	in_addr gateway = {};
	GetGateway(interfaceIndex, &gateway);
	if (gateway.s_addr == 0)
		gateway.s_addr = (address.s_addr & subnet.s_addr) | (1u << 24);
	return gateway;
}
