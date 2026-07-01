#include "user/MetalOS.h"

//SocketRecvFrom has more than 4 arguments, which the syscall ABI (Arg0..Arg3) cannot
//carry. The public BSD-style entry points are thin wrappers that pack the trailing
//arguments into SocketRecvParams and invoke the 2-argument raw stub (syscalls.asm).
extern "C" SyscallResult SocketRecvFromSys(HSocket sock, SocketRecvParams* params);

extern "C" SyscallResult SocketRecvFrom(HSocket sock, void* buf, size_t maxLen, size_t* bytesRecv, sockaddr_in* from, milli_t timeoutMs)
{
	SocketRecvParams params = { buf, maxLen, bytesRecv, from, timeoutMs };
	return SocketRecvFromSys(sock, &params);
}

extern "C" SyscallResult SocketRecv(HSocket sock, void* buf, size_t maxLen, size_t* bytesRecv, milli_t timeoutMs)
{
	return SocketRecvFrom(sock, buf, maxLen, bytesRecv, nullptr, timeoutMs);
}
