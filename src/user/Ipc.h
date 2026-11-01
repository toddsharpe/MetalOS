#pragma once

#include "user/MetalOS.h"
#include "user/Protocol_channel.h"

//Usermode IPC handshake helpers shared by the client DLLs and servers. Duplex channels live
//in shared memory; capability grants move handles across processes via the endpoint registry.

//Client: create a channel, stamp it with our pid, and grant it to the server over the control
//endpoint. Returns the mapped region (kept for the channel's life) or nullptr on failure.
template <typename Channel>
inline void* IpcConnect(const char* const controlEndpoint)
{
	HSharedMem handle = nullptr;
	void* region = nullptr;
	if (CreateSharedMemory(Channel::Size(), &handle, &region) != SyscallResult::Success)
		return nullptr;

	ProcessInfo info = {};
	GetProcessInfo(&info);
	Channel::Init(region, info.Id);

	//Mint a single-use token for the channel and advertise it on the control endpoint.
	uint64_t token = 0;
	if (ShareHandle((Handle)handle, &token) != SyscallResult::Success)
		return nullptr;
	if (PostEndpoint(controlEndpoint, token) != SyscallResult::Success)
		return nullptr;

	return region;
}

//Server: redeem a grant token -> map the region. outHandle is ours (CloseHandle to release);
//returns false and cleans up on failure. Retain outHandle for per-connection regions.
inline bool IpcClaim(const uint64_t token, HSharedMem& outHandle, void*& outRegion)
{
	outHandle = nullptr;
	outRegion = nullptr;

	Handle h = nullptr;
	if (ClaimHandle(token, &h) != SyscallResult::Success)
		return false;

	void* region = nullptr;
	if (MapSharedMemory((HSharedMem)h, &region) != SyscallResult::Success)
	{
		CloseHandle(h);
		return false;
	}

	outHandle = (HSharedMem)h;
	outRegion = region;
	return true;
}

//Server: look up a well-known region by endpoint, then claim + map it (handle dropped -- for
//singletons claimed once for the process lifetime, e.g. RX/input ring). Returns region or null.
inline void* IpcLookupRegion(const char* const endpoint)
{
	uint64_t token = 0;
	if (LookupEndpoint(endpoint, &token) != SyscallResult::Success)
		return nullptr;

	HSharedMem handle = nullptr;
	void* region = nullptr;
	return IpcClaim(token, handle, region) ? region : nullptr;
}
