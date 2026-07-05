#include "user/MetalOS.h"
#include "user/WmProtocol.h"
#include "Graphics/Types.h"
#include "Lib/Buffer.h"
#include <cstring>

using namespace Graphics;

namespace
{
	constexpr size_t MaxWindows = 64;

	struct ClientWindow
	{
		bool Used;
		uint32_t Id;
		void* Surface;
		size_t SurfaceSize;
		Rectangle Rect;
	};

	bool g_connected = false;
	void* g_channel = nullptr;
	uint32_t g_nextWindowId = 0;
	ClientWindow g_windows[MaxWindows];

	bool EnsureConnected()
	{
		if (g_connected)
			return true;

		HSharedMem hChannel = nullptr;
		void* region = nullptr;
		if (CreateSharedMemory(Wm::ChannelSize(), &hChannel, &region) != SyscallResult::Success)
			return false;

		//Stamp our process id so the WM can detect when we exit and reclaim our windows.
		ProcessInfo info = {};
		GetProcessInfo(&info);
		Wm::InitChannel(region, info.Id);
		g_channel = region;

		//Grant the channel to the WM: mint a capability token and advertise it.
		uint64_t token = 0;
		if (ShareHandle((Handle)hChannel, &token) != SyscallResult::Success)
			return false;
		if (PostEndpoint(Wm::ControlEndpoint, token) != SyscallResult::Success)
			return false;

		g_connected = true;
		return true;
	}

	ClientWindow* Find(const uint32_t id)
	{
		for (size_t i = 0; i < MaxWindows; i++)
		{
			if (g_windows[i].Used && g_windows[i].Id == id)
				return &g_windows[i];
		}
		return nullptr;
	}

	ClientWindow* Alloc()
	{
		for (size_t i = 0; i < MaxWindows; i++)
		{
			if (!g_windows[i].Used)
				return &g_windows[i];
		}
		return nullptr;
	}

	uint32_t ToId(const HWindow handle) { return (uint32_t)(uintptr_t)handle; }
}

extern "C" SyscallResult AllocWindow(HWindow* handle, const Graphics::Rectangle* frame)
{
	if (!handle || !frame)
		return SyscallResult::InvalidPointer;
	if (!EnsureConnected())
		return SyscallResult::Failed;

	ClientWindow* const window = Alloc();
	if (!window)
		return SyscallResult::Failed;

	const size_t surfaceSize = (size_t)frame->Width * frame->Height * sizeof(Color);
	HSharedMem hSurface = nullptr;
	void* surfaceAddr = nullptr;
	if (CreateSharedMemory(surfaceSize, &hSurface, &surfaceAddr) != SyscallResult::Success)
		return SyscallResult::Failed;

	//Grant the surface to the WM (single-use token carried in the AllocWindow request).
	uint64_t token = 0;
	if (ShareHandle((Handle)hSurface, &token) != SyscallResult::Success)
		return SyscallResult::Failed;

	const uint32_t id = ++g_nextWindowId;
	window->Used = true;
	window->Id = id;
	window->Surface = surfaceAddr;
	window->SurfaceSize = surfaceSize;
	window->Rect = *frame;

	Wm::Request request = {};
	request.Code = Wm::Op::AllocWindow;
	request.WindowId = id;
	request.Rect = *frame;
	request.SurfaceGrant = token;
	request.Width = frame->Width;
	request.Height = frame->Height;
	Wm::RequestRing(g_channel).Enqueue(request);

	*handle = (HWindow)(uintptr_t)id;
	return SyscallResult::Success;
}

extern "C" void* GetWindowSurface(HWindow handle)
{
	ClientWindow* const window = Find(ToId(handle));
	return window ? window->Surface : nullptr;
}

extern "C" SyscallResult MoveWindow(HWindow handle, const Graphics::Rectangle* frame)
{
	if (!frame)
		return SyscallResult::InvalidPointer;

	ClientWindow* const window = Find(ToId(handle));
	if (!window)
		return SyscallResult::InvalidHandle;

	window->Rect.X = frame->X;
	window->Rect.Y = frame->Y;

	Wm::Request request = {};
	request.Code = Wm::Op::MoveWindow;
	request.WindowId = window->Id;
	request.Rect = *frame;
	Wm::RequestRing(g_channel).Enqueue(request);
	return SyscallResult::Success;
}

extern "C" SyscallResult GetWindowRect(HWindow handle, Graphics::Rectangle* frame)
{
	if (!frame)
		return SyscallResult::InvalidPointer;

	ClientWindow* const window = Find(ToId(handle));
	if (!window)
		return SyscallResult::InvalidHandle;

	*frame = window->Rect;
	return SyscallResult::Success;
}

extern "C" SyscallResult GetMessage(Message* message)
{
	if (!message)
		return SyscallResult::InvalidPointer;
	if (!g_connected)
		return SyscallResult::Failed;

	SharedRing<Message> ring = Wm::MessageRing(g_channel);
	while (!ring.Dequeue(*message))
		Sleep(5);
	return SyscallResult::Success;
}

extern "C" SyscallResult PeekMessage(Message* message)
{
	if (!message)
		return SyscallResult::InvalidPointer;
	if (!g_connected)
		return SyscallResult::Failed;

	SharedRing<Message> ring = Wm::MessageRing(g_channel);
	return ring.Dequeue(*message) ? SyscallResult::Success : SyscallResult::Failed;
}

extern "C" SyscallResult GetScreenRect(Graphics::Rectangle* rect)
{
	if (!rect)
		return SyscallResult::InvalidPointer;

	uint64_t packed = 0;
	if (LookupEndpoint(Wm::ScreenEndpoint, &packed) != SyscallResult::Success)
		return SyscallResult::Failed;

	rect->X = 0;
	rect->Y = 0;
	rect->Width = (uint32_t)(packed >> 32);
	rect->Height = (uint32_t)(packed & 0xFFFFFFFF);
	return SyscallResult::Success;
}

size_t DllMain(HModule handle)
{
	return true;
}
