#pragma once

#include <cstdint>

//Shared ABI between the kernel net device (Kernel_net.cpp) and the usermode
//network stack (MetalOS-NetSvr.exe). The kernel owns the NIC; it hands raw ethernet
//frames up over the RX ring and accepts frames to transmit via the KeNetSend
//syscall. The RX ring shm and the NIC MAC are published in the endpoint registry.
namespace NetDevice
{
	static constexpr uint32_t Mtu = 1514;       //max ethernet frame (incl. header, excl. FCS)
	static constexpr uint32_t RxCapacity = 256; //kernel->netstack RX ring depth

	//A raw ethernet frame carried over the RX ring (fixed-size record).
	struct Frame
	{
		uint32_t IfIdx;
		uint16_t Length;
		uint8_t Data[Mtu];
	};

	static constexpr const char* RxEndpoint = "net.rx";   //grant token for the RX ring shm
	static constexpr const char* MacEndpoint = "net.mac"; //NIC MAC packed into the low 48 bits
}
