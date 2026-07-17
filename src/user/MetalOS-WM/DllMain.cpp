#include "user/MetalOS.h"
#include "user/Protocol_wm.h"
#include "user/Ipc.h"
#include "Graphics/Types.h"
#include "Lib/Buffer.h"
#include "Lib/StaticVector.h"
#include <cstring>

using namespace Graphics;

namespace
{
	constexpr size_t MaxWindows = 8;

	struct ClientWindow
	{
		uint32_t Id;
		void* Surface;
		HSharedMem SurfaceHandle; //our handle to the surface shm; DeleteSharedMemory on teardown
		size_t SurfaceSize;
		Rectangle Rect;
	};

	void* g_channel = nullptr;
	uint32_t g_nextWindowId = 0;
	StaticVector<ClientWindow, MaxWindows> g_windows;

	ClientWindow* Find(const uint32_t id)
	{
		for (ClientWindow& window : g_windows)
		{
			if (window.Id == id)
				return &window;
		}
		return nullptr;
	}

	uint32_t ToId(const HWindow handle) { return (uint32_t)(uintptr_t)handle; }
}

extern "C" SyscallResult AllocWindow(HWindow* handle, const Graphics::Rectangle* frame, const char* title, bool showTitle)
{
	if (!handle || !frame)
		return SyscallResult::InvalidPointer;

	if (g_windows.Count() >= MaxWindows)
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
	const ClientWindow window =
	{
		.Id = id,
		.Surface = surfaceAddr,
		.SurfaceHandle = hSurface,
		.SurfaceSize = surfaceSize,
		.Rect = *frame,
	};
	g_windows.Add(window); //capacity checked above, so this succeeds

	Wm::Request request =
	{
		.Code = Wm::Op::AllocWindow,
		.WindowId = id,
		.Rect = *frame,
		.SurfaceGrant = token,
		.Width = frame->Width,
		.Height = frame->Height,
	};
	//Copy the (bounded, null-terminated) title into the fixed request field.
	size_t n = 0;
	if (title)
		for (; n < Wm::MaxTitle - 1 && title[n]; n++)
			request.Title[n] = title[n];
	request.Title[n] = '\0';
	request.ShowTitle = showTitle;
	Wm::Channel::Requests(g_channel).Enqueue(request);

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

	const Wm::Request request =
	{
		.Code = Wm::Op::MoveWindow,
		.WindowId = window->Id,
		.Rect = *frame,
	};
	Wm::Channel::Requests(g_channel).Enqueue(request);
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

	SharedRing<Message> ring = Wm::Channel::Replies(g_channel);
	while (!ring.Dequeue(*message))
		Sleep(5);
	return SyscallResult::Success;
}

extern "C" SyscallResult PeekMessage(Message* message)
{
	if (!message)
		return SyscallResult::InvalidPointer;

	SharedRing<Message> ring = Wm::Channel::Replies(g_channel);
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

bool DllMain(HModule handle, DllReason reason)
{
	switch (reason)
	{
		case DllReason::ProcessAttach:
		{
			//Open a channel to the WM (create + Init + grant over "wm")
			void* const region = IpcConnect<Wm::Channel>(Wm::ControlEndpoint);
			if (!region)
				return false;

			g_channel = region;
		}
		break;
	}

	return true;
}
