#include "user/MetalOS.h"
#include "user/Protocol_wm.h"
#include "user/Ipc.h"
#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"
#include "Lib/LinkedList.h"
#include <cstring>
#include <stdio.h>

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
		void* Region;              //mapped channel (request ring app->WM, message ring WM->app); null = free slot
		HSharedMem ChannelHandle;  //our handle to the channel region; CloseHandle releases it
	};

	struct WmWindow
	{
		Connection* Owner;
		uint32_t WindowId;         //unique within its connection
		void* Surface;             //mapped shared surface (packed Width*Height pixels); null = free slot
		HSharedMem SurfaceHandle;  //our handle to the surface; CloseHandle releases it
		Rectangle Rect;            //position + size on screen
		char Title[Wm::MaxTitle];  //window title (for the taskbar)
		bool Minimized;            //hidden from compositing but kept in the taskbar
		bool ShowTitle;            //draw WM chrome (minimize/close buttons); false = chromeless
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
	WmWindow g_windows[MaxWindows];

	//Z-order: head is topmost. Holds pointers into g_windows (whose slots never move),
	//so the list can be reordered and removed from freely.
	LinkedList<WmWindow*> g_zorder;

	WmWindow* g_focus = nullptr;
	WmWindow* g_drag = nullptr;
	bool g_startMenuOpen = false;

	//Title-bar control buttons, at the window's top-right (drawn by the WM over the window).
	constexpr int32_t BtnSize = 16;
	Rectangle CloseBtn(const WmWindow* const w) { return { w->Rect.X + (int32_t)w->Rect.Width - BtnSize - 2, w->Rect.Y + 2, BtnSize, BtnSize }; }
	Rectangle MinBtn(const WmWindow* const w)   { return { w->Rect.X + (int32_t)w->Rect.Width - 2 * BtnSize - 4, w->Rect.Y + 2, BtnSize, BtnSize }; }

	void Raise(WmWindow* const window)
	{
		//Move to the top (head) of the z-order.
		if (g_zorder.Remove(window))
			g_zorder.InsertHead(window);
	}

	WmWindow* HitTest(const Point2D& point)
	{
		//Head is topmost, so the first (non-minimized) window containing the point wins.
		for (WmWindow* const window : g_zorder)
		{
			if (!window->Minimized && window->Rect.Contains(point))
				return window;
		}
		return nullptr;
	}

	//Topmost window that isn't minimized (null if none)
	WmWindow* TopmostVisible()
	{
		for (WmWindow* const window : g_zorder)
		{
			if (!window->Minimized)
				return window;
		}
		return nullptr;
	}

	//Send a message to a connection's WM->app ring
	void Post(Connection* const conn, const Message& message)
	{
		Wm::Channel::Replies(conn->Region).Enqueue(message);
	}

	//Send a message that carries only its type (e.g. PaintEvent, CloseEvent)
	void PostEvent(Connection* const conn, const MessageType type)
	{
		Message message = {};
		message.Header.MessageType = type;
		Post(conn, message);
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

			//Take the first free connection slot.
			Connection* slot = nullptr;
			for (Connection& conn : g_conns)
			{
				if (!conn.Region)
				{
					slot = &conn;
					break;
				}
			}
			if (!slot)
			{
				CloseHandle((Handle)hChannel);
				break;
			}
			slot->Region = region;
			slot->ChannelHandle = hChannel;
		}
	}

	WmWindow* FindWindow(Connection* const conn, const uint32_t windowId)
	{
		for (WmWindow& window : g_windows)
		{
			if (window.Surface && window.Owner == conn && window.WindowId == windowId)
				return &window;
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

			//Take the first free window slot.
			WmWindow* window = nullptr;
			for (WmWindow& slot : g_windows)
			{
				if (!slot.Surface)
				{
					window = &slot;
					break;
				}
			}
			if (!window)
			{
				CloseHandle((Handle)hSurface);
				return;
			}

			window->Owner = conn;
			window->WindowId = request.WindowId;
			window->Surface = surface;
			window->SurfaceHandle = hSurface;
			window->Rect = request.Rect;
			memcpy(window->Title, request.Title, Wm::MaxTitle);
			window->Title[Wm::MaxTitle - 1] = '\0';
			window->Minimized = false;
			window->ShowTitle = request.ShowTitle;

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
		for (Connection& conn : g_conns)
		{
			if (conn.Region == nullptr)
				continue;
			SharedRing<Wm::Request> ring = Wm::Channel::Requests(conn.Region);
			Wm::Request request;
			while (ring.Dequeue(request))
				HandleRequest(&conn, request);
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
			g_focus = TopmostVisible();

		//Release our surface handle (unmaps it), then free the slot.
		CloseHandle((Handle)window->SurfaceHandle);
		window->Surface = nullptr;
		window->Owner = nullptr;
	}

	//Ask the owning app to close (it exits its message loop), then drop the window.
	void CloseWindow(WmWindow* const window)
	{
		PostEvent(window->Owner, MessageType::CloseEvent);
		DestroyWindow(window);
	}

	//Hide a window from compositing (still listed in the taskbar). Refocus the next visible window.
	void Minimize(WmWindow* const window)
	{
		window->Minimized = true;
		if (g_focus == window)
			g_focus = TopmostVisible();
	}

	void RestoreAndFocus(WmWindow* const window)
	{
		window->Minimized = false;
		g_focus = window;
		Raise(window);
	}

	//Reclaim windows/connections whose owning process has exited (clean or crash).
	void ReapDeadConnections()
	{
		for (Connection& conn : g_conns)
		{
			if (conn.Region == nullptr)
				continue;

			const uint32_t pid = reinterpret_cast<Wm::Channel::Header*>(conn.Region)->ProcessId;
			if (pid == 0 || IsProcessAlive(pid))
				continue;

			for (WmWindow& window : g_windows)
			{
				if (window.Surface && window.Owner == &conn)
					DestroyWindow(&window);
			}
			//Release our channel handle (unmaps it) and free the connection slot.
			CloseHandle((Handle)conn.ChannelHandle);
			conn.Region = nullptr;
		}
	}

	//----- Panel / taskbar: Start launcher + running-window list + clock, always on top -----
	constexpr int32_t PanelHeight = 28;
	constexpr int32_t StartWidth  = 68;
	constexpr int32_t TaskWidth   = 160;
	constexpr int32_t TaskGap     = 4;
	constexpr int32_t ClockWidth  = 92;

	//Start menu
	struct AppEntry { const char* Label; const char* Exe; };
	constexpr AppEntry g_apps[] =
	{
		{ "Terminal",  "term.exe" },
		{ "Doom",      "doom.exe" },
		{ "Fire",      "fire.exe" },
		{ "Calc",      "calc.exe" },
		{ "Processes", "ps.exe"   },
	};
	constexpr size_t  g_appCount     = sizeof(g_apps) / sizeof(g_apps[0]);
	constexpr int32_t MenuWidth      = 160;
	constexpr int32_t MenuItemHeight = 24;

	int32_t PanelTop() { return (int32_t)g_gfx.Height - PanelHeight; }

	//Start menu sits directly above the panel, aligned with the Start button
	Rectangle StartMenuRect()
	{
		const uint32_t h = (uint32_t)g_appCount * MenuItemHeight;
		return { 4, PanelTop() - (int32_t)h, MenuWidth, h };
	}

	//Taskbar layout: one button per window in slot order until the clock; fn(window, x) returns true to stop
	template <typename TFn>
	void ForEachTaskButton(TFn&& fn)
	{
		int32_t x = StartWidth + TaskGap;
		for (WmWindow& window : g_windows)
		{
			if (!window.Surface)
				continue;
			if (x + TaskWidth > (int32_t)g_gfx.Width - ClockWidth || fn(window, x))
				return;
			x += TaskWidth + TaskGap;
		}
	}

	void DrawPanel()
	{
		const int32_t y0 = PanelTop();
		const int32_t w = (int32_t)g_gfx.Width;
		const Color panelBg = { 0x20, 0x20, 0x20 };
		const Color taskBg  = { 0x50, 0x50, 0x50 };

		//Background + blue top edge
		Draw2D::DrawRectangle(g_back, panelBg, { 0, y0, (uint32_t)w, PanelHeight });
		Draw2D::DrawRectangle(g_back, Colors::Blue, { 0, y0, (uint32_t)w, 2 });

		//Start button
		Draw2D::DrawRectangle(g_back, Colors::Blue, { 4, y0 + 4, StartWidth - 8, PanelHeight - 8 });
		Draw2D::DrawText(g_back, { 12, y0 + 10 }, CString("Start"), Colors::White);

		//Taskbar: one button per window (slot order); focused highlighted, hovered lit, ellipsized.
		ForEachTaskButton([&](const WmWindow& window, const int32_t x)
		{
			const bool focused = (&window == g_focus);
			const bool hover = g_mouse.X >= x && g_mouse.X < x + TaskWidth && g_mouse.Y >= y0;
			const Color bg = focused ? Colors::Blue : (hover ? Color{ 0x68, 0x68, 0x68 } : taskBg);
			Draw2D::DrawRectangle(g_back, bg, { x, y0 + 4, TaskWidth, PanelHeight - 8 });

			//Title, ellipsized to ~14 chars so it fits the button.
			const size_t len = strlen(window.Title);
			const Point2D end = Draw2D::DrawText(g_back, { x + 6, y0 + 10 }, CString(window.Title, len <= 14 ? len : 11), Colors::White);
			if (len > 14)
				Draw2D::DrawText(g_back, end, CString("..."), Colors::White);
			return false;
		});

		//Clock (HH:MM:SS) on the right
		SystemTime t = {};
		if (GetSystemTime(&t) == SyscallResult::Success)
		{
			char clock[9];
			snprintf(clock, sizeof(clock), "%02d:%02d:%02d", t.Hour, t.Minute, t.Second);
			Draw2D::DrawText(g_back, { w - ClockWidth + 10, y0 + 10 }, CString(clock), Colors::White);
		}
	}

	void HandlePanelClick(const Point2D& point)
	{
		//Start button -> toggle the app menu
		if (point.X >= 4 && point.X < StartWidth)
		{
			g_startMenuOpen = !g_startMenuOpen;
			return;
		}

		//Any other panel click dismisses the menu.
		g_startMenuOpen = false;

		//Taskbar button -> minimize the focused window, otherwise restore/focus/raise it
		ForEachTaskButton([&](WmWindow& window, const int32_t x)
		{
			if (point.X < x || point.X >= x + TaskWidth)
				return false;

			if (&window == g_focus && !window.Minimized)
				Minimize(&window);
			else
				RestoreAndFocus(&window);
			return true;
		});
	}

	void DrawStartMenu()
	{
		if (!g_startMenuOpen)
			return;

		const Rectangle menu = StartMenuRect();
		const Color menuBg = { 0x30, 0x30, 0x30 };

		Draw2D::DrawRectangle(g_back, menuBg, menu);
		Draw2D::DrawRectangle(g_back, Colors::Blue, { menu.X, menu.Y, menu.Width, 2 });

		for (size_t i = 0; i < g_appCount; i++)
		{
			const Rectangle item = { menu.X, menu.Y + (int32_t)i * MenuItemHeight, menu.Width, MenuItemHeight };
			//Highlight the item under the cursor.
			if (item.Contains(g_mouse))
				Draw2D::DrawRectangle(g_back, Colors::Blue, item);
			Draw2D::DrawText(g_back, { item.X + 8, item.Y + 8 }, CString(g_apps[i].Label), Colors::White);
		}
	}

	bool InStartMenu(const Point2D& point)
	{
		return g_startMenuOpen && StartMenuRect().Contains(point);
	}

	void HandleStartMenuClick(const Point2D& point)
	{
		const size_t idx = (size_t)((point.Y - StartMenuRect().Y) / MenuItemHeight);
		if (idx < g_appCount)
			CreateProcess(g_apps[idx].Exe, nullptr, nullptr);
		g_startMenuOpen = false;
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
					if (InStartMenu(mousePos))
					{
						//Launch the picked app and close the menu.
						HandleStartMenuClick(mousePos);
					}
					else if (mousePos.Y >= PanelTop())
					{
						//Clicks on the panel drive the launcher/taskbar, not window focus/drag.
						HandlePanelClick(mousePos);
					}
					else
					{
						//A click on the desktop/window dismisses the start menu.
						g_startMenuOpen = false;
						WmWindow* const hit = HitTest(mousePos);
						if (hit && hit->ShowTitle && CloseBtn(hit).Contains(mousePos))
						{
							CloseWindow(hit);
						}
						else if (hit && hit->ShowTitle && MinBtn(hit).Contains(mousePos))
						{
							Minimize(hit);
						}
						else
						{
							g_focus = hit;
							g_drag = hit;
							if (hit)
								Raise(hit);
						}
					}
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

	void Composite()
	{
		Draw2D::FillScreen(g_back, Colors::Black);

		//Draw bottom to top: Reversed() walks tail -> head, i.e. bottommost first.
		for (WmWindow* const window : g_zorder.Reversed())
		{
			if (window->Minimized)
				continue;

			const Rectangle bounds = { window->Rect.X, window->Rect.Y, window->Rect.Width, window->Rect.Height };
			if (window == g_focus)
				Draw2D::WriteFrame(g_back, bounds, window->Surface);
			else
				Draw2D::WriteFrameGrayscale(g_back, bounds, window->Surface);

			//WM-drawn title-bar controls (over the window's top-right): minimize then close.
			//Chromeless windows (ShowTitle == false, e.g. Doom) get no controls.
			if (window->ShowTitle)
			{
				const Rectangle mb = MinBtn(window);
				Draw2D::DrawRectangle(g_back, Color{ 0x50, 0x50, 0x50 }, mb);
				Draw2D::DrawText(g_back, { mb.X + 4, mb.Y + 4 }, CString("-"), Colors::White);
				const Rectangle cb = CloseBtn(window);
				Draw2D::DrawRectangle(g_back, Colors::Red, cb);
				Draw2D::DrawText(g_back, { cb.X + 4, cb.Y + 4 }, CString("x"), Colors::White);
			}
		}

		//Panel/taskbar draws last so it's always on top of windows; the start menu tops even that.
		DrawPanel();
		DrawStartMenu();

		//Blit backbuffer to the hardware framebuffer, honoring stride
		Color* const fb = (Color*)g_gfx.FrameBase;
		for (uint32_t y = 0; y < g_gfx.Height; y++)
			memcpy(fb + (size_t)y * g_gfx.PixelsPerScanLine, g_back.Buffer + (size_t)y * g_gfx.Width, (size_t)g_gfx.Width * sizeof(Color));

		//Present: tell the display to re-read the framebuffer (synthvid needs a DIRT to refresh).
		FlushFramebuffer();

		//Request repaints
		for (WmWindow& window : g_windows)
		{
			if (window.Surface)
				PostEvent(window.Owner, MessageType::PaintEvent);
		}
	}
}

int main(int argc, char** argv)
{
	//Map the framebuffer into this process
	if (MapFramebuffer(&g_gfx) != SyscallResult::Success)
		return 1;
	DebugPrintf("WM: framebuffer %dx%d, stride %d, base %p\n", g_gfx.Width, g_gfx.Height, g_gfx.PixelsPerScanLine, g_gfx.FrameBase);

	//Backbuffer (packed Width*Height)
	void* const back = VirtualAlloc(nullptr, (size_t)g_gfx.Width * g_gfx.Height * sizeof(Color));
	g_back = FrameBuffer(static_cast<Color*>(back), g_gfx.Height, g_gfx.Width);

	//Publish the work area (screen minus the panel) for clients (GetScreenRect) so apps place and
	//size windows above the taskbar, then open the control endpoint.
	RegisterEndpoint(Wm::ScreenEndpoint, ((uint64_t)g_gfx.Width << 32) | (g_gfx.Height - PanelHeight));
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
