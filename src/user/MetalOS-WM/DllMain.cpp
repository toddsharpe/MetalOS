#include "user/MetalOS.h"
#include "user/Protocol_wm.h"
#include "user/Ipc.h"
#include "Graphics/Types.h"
#include "Lib/Span.h"
#include "Lib/StaticVector.h"
#include <cstring>

using namespace Graphics;

namespace
{
	constexpr size_t MaxWindows = 8;

	struct ClientWindow
	{
		uint16_t Id;
		void* Surface;
		HSharedMem SurfaceHandle;
	};

	void* g_channel = nullptr;
	uint16_t g_nextWindowId = 0; //ids are never reused and at most MaxWindows are made, so they stay small and nonzero
	StaticVector<ClientWindow, MaxWindows> g_windows;

	ClientWindow* Find(const uint16_t id)
	{
		for (ClientWindow& window : g_windows)
		{
			if (window.Id == id)
				return &window;
		}
		return nullptr;
	}

	//A shared surface for a client area, and a single-use grant the WM redeems to map it too
	bool CreateSurface(const uint32_t width, const uint32_t height, HSharedMem& handle, void*& address, uint64_t& token)
	{
		const size_t surfaceSize = (size_t)width * height * sizeof(Color);
		if (surfaceSize == 0 || CreateSharedMemory(surfaceSize, &handle, &address) != SyscallResult::Success)
			return false;

		if (ShareHandle((Handle)handle, &token) != SyscallResult::Success)
		{
			CloseHandle((Handle)handle);
			return false;
		}
		return true;
	}

	//Packs pixel art (up to 8x8) into the request's two bits per cell
	Wm::IconBits PackIcon(const PixelGlyph* const glyph)
	{
		Wm::IconBits icon = {};
		if (!glyph)
			return icon;

		for (size_t i = 0; i < 3; i++)
			icon.Palette[i] = glyph->Palette[i];
		for (uint32_t y = 0; y < glyph->Height && y < 8; y++)
		{
			const char* const row = glyph->Rows[y];
			for (uint32_t x = 0; x < glyph->Width && x < 8 && row[x] != '\0'; x++)
			{
				if (row[x] >= 'a' && row[x] <= 'c')
					icon.Rows[y] |= (uint16_t)((row[x] - 'a' + 1) << (2 * x));
			}
		}
		return icon;
	}
}

extern "C" SyscallResult AllocWindow(HWindow* handle, const Graphics::Rectangle* frame, const char* title, WindowStyle style, const PixelGlyph* icon)
{
	if (!handle || !frame)
		return SyscallResult::InvalidPointer;

	if (g_windows.Count() >= MaxWindows)
		return SyscallResult::Failed;

	HSharedMem hSurface = nullptr;
	void* surfaceAddr = nullptr;
	uint64_t token = 0;
	if (!CreateSurface(frame->Width, frame->Height, hSurface, surfaceAddr, token))
		return SyscallResult::Failed;

	const uint16_t id = ++g_nextWindowId;
	const ClientWindow window =
	{
		.Id = id,
		.Surface = surfaceAddr,
		.SurfaceHandle = hSurface,
	};
	g_windows.Add(window); //capacity checked above, so this succeeds

	Wm::Request request =
	{
		.Code = Wm::Op::AllocWindow,
		.WindowId = id,
		.Rect = *frame,
		.SurfaceGrant = token,
	};
	//Copy the (bounded, null-terminated) title into the fixed request field.
	size_t n = 0;
	if (title)
		for (; n < Wm::MaxTitle - 1 && title[n]; n++)
			request.Title[n] = title[n];
	request.Title[n] = '\0';
	request.Style = style;
	request.Icon = PackIcon(icon);
	Wm::Channel::Requests(g_channel).Enqueue(request);

	*handle = (HWindow)(uintptr_t)id;
	return SyscallResult::Success;
}

extern "C" SyscallResult ResizeWindow(HWindow handle, const uint32_t width, const uint32_t height)
{
	ClientWindow* const window = Find(WindowId(handle));
	if (!window)
		return SyscallResult::InvalidHandle;

	HSharedMem hSurface = nullptr;
	void* surfaceAddr = nullptr;
	uint64_t token = 0;
	if (!CreateSurface(width, height, hSurface, surfaceAddr, token))
		return SyscallResult::Failed;

	Wm::Request request =
	{
		.Code = Wm::Op::ResizeWindow,
		.WindowId = window->Id,
		.Rect = { 0, 0, width, height },
		.SurfaceGrant = token,
	};
	Wm::Channel::Requests(g_channel).Enqueue(request);

	//The WM keeps its own mapping of the old surface until it takes the new one, so ours can go now
	CloseHandle((Handle)window->SurfaceHandle);
	window->Surface = surfaceAddr;
	window->SurfaceHandle = hSurface;
	return SyscallResult::Success;
}

extern "C" void* GetWindowSurface(HWindow handle)
{
	ClientWindow* const window = Find(WindowId(handle));
	return window ? window->Surface : nullptr;
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
