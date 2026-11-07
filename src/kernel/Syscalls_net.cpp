#include "kernel/Syscalls.h"
#include "kernel/Scheduler.h"
#include "kernel/Drivers/Driver.h"
#include "kernel/Kernel_net.h"

//Transmit a raw ethernet frame through the NIC (the TCP/IP stack is the usermode netstack)
SyscallResult NetSend(const uint32_t ifIdx, const void* frame, size_t length)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (!length)
		return SyscallResult::InvalidArg;
	if (!proc.Space.IsValidRange(frame, length))
		return SyscallResult::InvalidPointer;

	return KeNetSend(ifIdx, frame, length) ? SyscallResult::Success : SyscallResult::Failed;
}

//Lists the NICs in the device tree by index and MAC; IPv4 config belongs to the usermode netstack
SyscallResult GetInterfaces(InterfaceInfo* buffer, const size_t maxCount, size_t* count)
{
	UProcess& proc = Scheduler::GetUProcess();
	if (maxCount > static_cast<size_t>(-1) / sizeof(InterfaceInfo))
		return SyscallResult::InvalidArg; //Byte size would overflow
	if (!proc.Space.IsValidRange(buffer, maxCount * sizeof(InterfaceInfo)) || !proc.Space.IsValid(count))
		return SyscallResult::InvalidPointer;

	const size_t total = KeNetInterfaceCount();

	size_t n = 0;
	for (size_t i = 0; i < total && n < maxCount; i++)
	{
		NicDriver* driver = KeNetGetInterface(i);
		if (!driver)
			continue;

		InterfaceInfo& info = buffer[n];
		info.index = (uint32_t)i;
		driver->GetMac(info.mac);

		n++;
	}

	*count = n;
	return SyscallResult::Success;
}
