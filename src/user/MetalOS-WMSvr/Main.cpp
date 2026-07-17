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
	size_t g_connCount = 0;

	WmWindow g_windows[MaxWindows];
	size_t g_windowCount = 0;

	//Z-order: head is topmost. Holds pointers into g_windows (whose slots never move),
	//so the list can be reordered and removed from freely.
	LinkedList<WmWindow*> g_zorder;

	WmWindow* g_focus = nullptr;
	WmWindow* g_drag = nullptr;
	bool g_startMenuOpen = false;

	//Panel/taskbar/start-menu (defined below, used by ProcessInput).
	int32_t PanelTop();
	void HandlePanelClick(const Point2D& point);
	bool InStartMenu(const Point2D& point);
	void HandleStartMenuClick(const Point2D& point);
	//Window controls (defined below, used by ProcessInput).
	void CloseWindow(WmWindow* const window);
	void Minimize(WmWindow* const window);
	void RestoreAndFocus(WmWindow* const window);

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
					if (InStartMenu(mousePos))
					{
						//Launch the picked app and close the menu.
						HandleStartMenuClick(mousePos);
						g_drag = nullptr;
					}
					else if (mousePos.Y >= PanelTop())
					{
						//Clicks on the panel drive the launcher/taskbar, not window focus/drag.
						HandlePanelClick(mousePos);
						g_drag = nullptr;
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

	//Ask the owning app to close (it exits its message loop), then drop the window.
	void CloseWindow(WmWindow* const window)
	{
		Message message = {};
		message.Header.MessageType = MessageType::CloseEvent;
		Post(window->Owner, message);
		DestroyWindow(window);
	}

	//Hide a window from compositing (still listed in the taskbar). Refocus the next visible window.
	void Minimize(WmWindow* const window)
	{
		window->Minimized = true;
		if (g_drag == window)
			g_drag = nullptr;
		if (g_focus == window)
		{
			g_focus = nullptr;
			for (WmWindow* const w : g_zorder)
			{
				if (!w->Minimized)
				{
					g_focus = w;
					break;
				}
			}
		}
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
	int32_t StartMenuTop() { return PanelTop() - (int32_t)g_appCount * MenuItemHeight; }

	//Write a zero-padded 2-digit value into out[0..1].
	void Fmt2(char* const out, const uint32_t v)
	{
		out[0] = (char)('0' + (v / 10) % 10);
		out[1] = (char)('0' + v % 10);
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
		int32_t x = StartWidth + TaskGap;
		for (size_t i = 0; i < g_windowCount; i++)
		{
			if (!g_windows[i].Used || x + TaskWidth > w - ClockWidth)
				continue;

			const bool focused = (&g_windows[i] == g_focus);
			const bool hover = g_mouse.X >= x && g_mouse.X < x + TaskWidth && g_mouse.Y >= y0;
			const Color bg = focused ? Colors::Blue : (hover ? Color{ 0x68, 0x68, 0x68 } : taskBg);
			Draw2D::DrawRectangle(g_back, bg, { x, y0 + 4, TaskWidth, PanelHeight - 8 });

			//Title, ellipsized to ~14 chars so it fits the button.
			const char* const t = g_windows[i].Title;
			size_t tlen = 0;
			while (t[tlen])
				tlen++;
			char label[16];
			size_t n;
			if (tlen <= 14)
			{
				for (n = 0; n < tlen; n++)
					label[n] = t[n];
			}
			else
			{
				for (n = 0; n < 11; n++)
					label[n] = t[n];
				label[11] = label[12] = label[13] = '.';
				n = 14;
			}
			Draw2D::DrawText(g_back, { x + 6, y0 + 10 }, CString(label, n), Colors::White);
			x += TaskWidth + TaskGap;
		}

		//Clock (HH:MM:SS) on the right
		SystemTime t = {};
		if (GetSystemTime(&t) == SyscallResult::Success)
		{
			char clock[9];
			Fmt2(&clock[0], t.Hour);   clock[2] = ':';
			Fmt2(&clock[3], t.Minute); clock[5] = ':';
			Fmt2(&clock[6], t.Second); clock[8] = '\0';
			Draw2D::DrawText(g_back, { w - ClockWidth + 10, y0 + 10 }, CString(clock, 8), Colors::White);
		}
	}

	void HandlePanelClick(const Point2D& point)
	{
		const int32_t w = (int32_t)g_gfx.Width;

		//Start button -> toggle the app menu
		if (point.X >= 4 && point.X < StartWidth)
		{
			g_startMenuOpen = !g_startMenuOpen;
			return;
		}

		//Any other panel click dismisses the menu.
		g_startMenuOpen = false;

		//Taskbar button -> focus + raise its window (same layout as DrawPanel)
		int32_t x = StartWidth + TaskGap;
		for (size_t i = 0; i < g_windowCount; i++)
		{
			if (!g_windows[i].Used || x + TaskWidth > w - ClockWidth)
				continue;
			if (point.X >= x && point.X < x + TaskWidth)
			{
				WmWindow* const w = &g_windows[i];
				if (w->Minimized)
					RestoreAndFocus(w);   //restore a minimized window
				else if (w == g_focus)
					Minimize(w);          //click the focused window's button to minimize it
				else
				{
					g_focus = w;          //otherwise bring it to the front
					Raise(w);
				}
				return;
			}
			x += TaskWidth + TaskGap;
		}
	}

	void DrawStartMenu()
	{
		if (!g_startMenuOpen)
			return;

		const int32_t x = 4;
		const int32_t y0 = StartMenuTop();
		const int32_t h = (int32_t)g_appCount * MenuItemHeight;
		const Color menuBg = { 0x30, 0x30, 0x30 };

		Draw2D::DrawRectangle(g_back, menuBg, { x, y0, MenuWidth, (uint32_t)h });
		Draw2D::DrawRectangle(g_back, Colors::Blue, { x, y0, MenuWidth, 2 });

		for (size_t i = 0; i < g_appCount; i++)
		{
			const int32_t iy = y0 + (int32_t)i * MenuItemHeight;
			//Highlight the item under the cursor.
			if (g_mouse.X >= x && g_mouse.X < x + MenuWidth && g_mouse.Y >= iy && g_mouse.Y < iy + MenuItemHeight)
				Draw2D::DrawRectangle(g_back, Colors::Blue, { x, iy, MenuWidth, MenuItemHeight });
			Draw2D::DrawText(g_back, { x + 8, iy + 8 }, CString(g_apps[i].Label), Colors::White);
		}
	}

	bool InStartMenu(const Point2D& point)
	{
		if (!g_startMenuOpen)
			return false;
		const int32_t y0 = StartMenuTop();
		return point.X >= 4 && point.X < 4 + MenuWidth && point.Y >= y0 && point.Y < PanelTop();
	}

	void HandleStartMenuClick(const Point2D& point)
	{
		const size_t idx = (size_t)((point.Y - StartMenuTop()) / MenuItemHeight);
		if (idx < g_appCount)
			CreateProcess(g_apps[idx].Exe, nullptr, nullptr);
		g_startMenuOpen = false;
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
