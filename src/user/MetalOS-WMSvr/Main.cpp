#include "user/MetalOS.h"
#include "user/Protocol_wm.h"
#include "user/Ipc.h"
#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"
#include "Lib/LinkedList.h"
#include <cstring>

//MetalOS usermode window manager: compositing, window/focus/drag/z-order, input routing. The
//kernel only maps the framebuffer + feeds the input ring ("input"); apps connect over "wm".

using namespace Graphics;

namespace
{
	constexpr size_t MaxWindows = 64;
	constexpr size_t MaxConns = 16;
	constexpr uint16_t HidMax = 32767; //HID absolute coordinate range (0..INT16_MAX)

	struct Connection
	{
		void* Region;              //mapped channel (request ring app->WM, message ring WM->app)
		HSharedMem ChannelHandle;  //our handle to the channel region; CloseHandle releases it
	};

	struct WmWindow
	{
		bool Used;                 //windows are freed in place (never moved) so z-order/Owner pointers stay valid
		Connection* Owner;
		uint32_t WindowId;         //unique within its connection
		void* Surface;             //mapped shared surface (packed Width*Height pixels)
		HSharedMem SurfaceHandle;  //our handle to the surface; CloseHandle releases it
		Rectangle Rect;            //position + size on screen
	};

	//Framebuffer / composition
	GraphicsDevice g_gfx;
	FrameBuffer g_back(nullptr, 0, 0);

	//Input
	void* g_inputRegion = nullptr;
	Point2D g_mouse;
	MouseButtonState g_prevButtons;

	//Connections + windows
	Connection g_conns[MaxConns];
	size_t g_connCount = 0;

	WmWindow g_windows[MaxWindows];
	size_t g_windowCount = 0;

	//Z-order: head is topmost. Holds pointers into g_windows (whose slots never move),
	//so the list can be reordered and removed from freely.
	LinkedList<WmWindow*> g_zorder;

	WmWindow* g_focus = nullptr;
	WmWindow* g_drag = nullptr;

	void Raise(WmWindow* const window)
	{
		//Move to the top (head) of the z-order.
		if (g_zorder.Remove(window))
			g_zorder.InsertHead(window);
	}

	WmWindow* HitTest(const Point2D& point)
	{
		//Head is topmost, so the first window containing the point wins.
		for (WmWindow* const window : g_zorder)
		{
			if (window->Rect.Contains(point))
				return window;
		}
		return nullptr;
	}

	//Send a message to a connection's WM->app ring
	void Post(Connection* const conn, const Message& message)
	{
		Wm::Channel::Replies(conn->Region).Enqueue(message);
	}

	void AcceptConnections()
	{
		uint64_t token = 0;
		while (PollEndpoint(Wm::ControlEndpoint, &token) == SyscallResult::Success)
		{
			//Redeem the app's grant to get our own handle, then map the channel.
			HSharedMem hChannel = nullptr;
			void* region = nullptr;
			if (!IpcClaim(token, hChannel, region))
				continue;

			//Reuse a freed connection slot if one exists, else grow.
			size_t slot = g_connCount;
			for (size_t i = 0; i < g_connCount; i++)
			{
				if (g_conns[i].Region == nullptr)
				{
					slot = i;
					break;
				}
			}
			if (slot == g_connCount)
			{
				if (g_connCount >= MaxConns)
				{
					CloseHandle((Handle)hChannel);
					break;
				}
				g_connCount++;
			}
			g_conns[slot].Region = region;
			g_conns[slot].ChannelHandle = hChannel;
		}
	}

	WmWindow* FindWindow(Connection* const conn, const uint32_t windowId)
	{
		for (size_t i = 0; i < g_windowCount; i++)
		{
			if (g_windows[i].Used && g_windows[i].Owner == conn && g_windows[i].WindowId == windowId)
				return &g_windows[i];
		}
		return nullptr;
	}

	void HandleRequest(Connection* const conn, const Wm::Request& request)
	{
		switch (request.Code)
		{
		case Wm::Op::AllocWindow:
		{
			//Redeem the app's grant to get our own handle, then map the surface.
			HSharedMem hSurface = nullptr;
			void* surface = nullptr;
			if (!IpcClaim(request.SurfaceGrant, hSurface, surface))
				return;

			//Reuse a freed window slot if available, else grow.
			size_t slot = g_windowCount;
			for (size_t i = 0; i < g_windowCount; i++)
			{
				if (!g_windows[i].Used)
				{
					slot = i;
					break;
				}
			}
			if (slot == g_windowCount)
			{
				if (g_windowCount >= MaxWindows)
				{
					CloseHandle((Handle)hSurface);
					return;
				}
				g_windowCount++;
			}

			WmWindow* const window = &g_windows[slot];
			window->Used = true;
			window->Owner = conn;
			window->WindowId = request.WindowId;
			window->Surface = surface;
			window->SurfaceHandle = hSurface;
			window->Rect = request.Rect;

			g_zorder.InsertHead(window); //new window on top
			g_focus = window;
			break;
		}

		case Wm::Op::MoveWindow:
		{
			WmWindow* const window = FindWindow(conn, request.WindowId);
			if (window)
			{
				window->Rect.X = request.Rect.X;
				window->Rect.Y = request.Rect.Y;
			}
			break;
		}
		}
	}

	void ProcessRequests()
	{
		for (size_t i = 0; i < g_connCount; i++)
		{
			if (g_conns[i].Region == nullptr)
				continue;
			SharedRing<Wm::Request> ring = Wm::Channel::Requests(g_conns[i].Region);
			Wm::Request request;
			while (ring.Dequeue(request))
				HandleRequest(&g_conns[i], request);
		}
	}

	void ProcessInput()
	{
		if (!g_inputRegion)
			return;

		SharedRing<Message> ring(g_inputRegion);
		Message message;
		while (ring.Dequeue(message))
		{
			if (message.Header.MessageType == MessageType::MouseEvent)
			{
				const uint16_t absX = (uint16_t)((uint32_t)g_gfx.Width * message.MouseEvent.XPosition / HidMax);
				const uint16_t absY = (uint16_t)((uint32_t)g_gfx.Height * message.MouseEvent.YPosition / HidMax);
				const Point2D mousePos = { absX, absY };

				const bool clicked = message.MouseEvent.Buttons.LeftPressed && !g_prevButtons.LeftPressed;
				if (clicked)
				{
					g_focus = HitTest(mousePos);
					g_drag = g_focus;
					if (g_focus)
						Raise(g_focus);
				}
				else if (message.MouseEvent.Buttons.LeftPressed)
				{
					if (g_drag)
					{
						g_drag->Rect.X += (int32_t)(mousePos.X - g_mouse.X);
						g_drag->Rect.Y += (int32_t)(mousePos.Y - g_mouse.Y);
					}
				}
				else
				{
					g_drag = nullptr;
				}

				g_mouse = mousePos;
				g_prevButtons = message.MouseEvent.Buttons;
			}

			//Route input to the focused window's owner
			if (g_focus)
				Post(g_focus->Owner, message);
		}
	}

	//Free a window in place (keeps other z-order/Owner pointers valid).
	void DestroyWindow(WmWindow* const window)
	{
		//Drop it from the z-order.
		g_zorder.Remove(window);

		if (g_drag == window)
			g_drag = nullptr;
		if (g_focus == window)
			g_focus = g_zorder.First(); //new topmost (null if the list is now empty)

		//Release our surface handle (unmaps it), then free the slot.
		if (window->Surface)
			CloseHandle((Handle)window->SurfaceHandle);
		window->Surface = nullptr;
		window->Used = false;
		window->Owner = nullptr;
	}

	//Reclaim windows/connections whose owning process has exited (clean or crash).
	void ReapDeadConnections()
	{
		for (size_t i = 0; i < g_connCount; i++)
		{
			Connection* const conn = &g_conns[i];
			if (conn->Region == nullptr)
				continue;

			const uint32_t pid = reinterpret_cast<Wm::Channel::Header*>(conn->Region)->ProcessId;
			if (pid == 0 || IsProcessAlive(pid))
				continue;

			for (size_t w = 0; w < g_windowCount; w++)
			{
				if (g_windows[w].Used && g_windows[w].Owner == conn)
					DestroyWindow(&g_windows[w]);
			}
			//Release our channel handle (unmaps it) and free the connection slot.
			CloseHandle((Handle)conn->ChannelHandle);
			conn->Region = nullptr;
		}
	}

	void Composite()
	{
		Draw2D::FillScreen(g_back, Colors::Black);

		//Draw bottom to top: Reversed() walks tail -> head, i.e. bottommost first.
		for (WmWindow* const window : g_zorder.Reversed())
		{
			const Rectangle bounds = { window->Rect.X, window->Rect.Y, window->Rect.Width, window->Rect.Height };
			if (window == g_focus)
				Draw2D::WriteFrame(g_back, bounds, window->Surface);
			else
				Draw2D::WriteFrameGrayscale(g_back, bounds, window->Surface);
		}

		//Blit backbuffer to the hardware framebuffer, honoring stride
		Color* const fb = (Color*)g_gfx.FrameBase;
		for (uint32_t y = 0; y < g_gfx.Height; y++)
			memcpy(fb + (size_t)y * g_gfx.PixelsPerScanLine, g_back.Buffer + (size_t)y * g_gfx.Width, (size_t)g_gfx.Width * sizeof(Color));

		//Request repaints
		for (size_t i = 0; i < g_windowCount; i++)
		{
			if (!g_windows[i].Used)
				continue;
			Message message = {};
			message.Header.MessageType = MessageType::PaintEvent;
			message.PaintEvent.Region = g_windows[i].Rect;
			Post(g_windows[i].Owner, message);
		}
	}
}

int main(int argc, char** argv)
{
	//Map the framebuffer into this process
	if (MapFramebuffer(&g_gfx) != SyscallResult::Success)
		return 1;

	//Backbuffer (packed Width*Height)
	void* const back = VirtualAlloc(nullptr, (size_t)g_gfx.Width * g_gfx.Height * sizeof(Color));
	g_back = FrameBuffer(static_cast<Color*>(back), g_gfx.Height, g_gfx.Width);

	//Publish the screen size for clients (GetScreenRect) and open the control endpoint
	RegisterEndpoint(Wm::ScreenEndpoint, ((uint64_t)g_gfx.Width << 32) | g_gfx.Height);
	RegisterEndpoint(Wm::ControlEndpoint, 0);

	//Claim the kernel->WM input ring (published as a grant token under "input").
	g_inputRegion = IpcLookupRegion(Wm::InputEndpoint);

	const uint32_t targetFps = 30;
	while (true)
	{
		AcceptConnections();
		ProcessRequests();
		ProcessInput();
		ReapDeadConnections();
		Composite();
		Sleep(1000 / targetFps);
	}

	return 0;
}
