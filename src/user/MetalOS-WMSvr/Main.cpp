#include "user/MetalOS.h"
#include "user/Protocol_wm.h"
#include "user/Protocol_net.h"
#include "user/Ipc.h"
#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"
#include "Graphics/Theme.h"
#include "Graphics/Icons.h"
#include "Lib/LinkedList.h"
#include <cstring>
#include <stdio.h>

//MetalOS usermode window manager: compositing, window frames, focus, drag, z-order, taskbar, start menu, window switching and input routing. The kernel only maps the framebuffer and feeds the input ring ("input"); apps connect over "wm".

using namespace Graphics;

namespace
{
	static constexpr size_t MaxWindows = 64;
	static constexpr size_t MaxConns = 16;
	static constexpr uint16_t HidMax = 32767; //HID absolute coordinate range (0..INT16_MAX)
	static constexpr uint32_t DroppableBacklog = 32; //mouse moves and paints are skipped while an app has this many messages waiting
	static constexpr uint32_t DimKeep = 184; //unfocused windows keep this much of their brightness (of 256)
	static constexpr milli_t DoubleClickTime = 400;
	static constexpr milli_t NetworkPollTime = 1000;
	static constexpr int32_t TitleGrip = 96; //this much of a title bar always stays on screen, so a window can be grabbed again

	struct Connection
	{
		void* Region;              //mapped channel (request ring app->WM, message ring WM->app); null = free slot
		HSharedMem ChannelHandle;  //our handle to the channel region; CloseHandle releases it
	};

	struct WmWindow
	{
		Connection* Owner;
		uint16_t WindowId;         //unique within its connection; messages for the window carry it
		void* Surface;             //mapped shared surface (packed client-sized pixels); null = free slot
		HSharedMem SurfaceHandle;  //our handle to the surface; CloseHandle releases it
		Rectangle Client;          //where the app's surface goes on screen; the frame is drawn around it
		Rectangle Restore;         //client area to go back to when un-maximized
		char Title[Wm::MaxTitle];
		WindowStyle Style;
		const PixelGlyph* Icon;    //IconGlyph when the app sent one, else the generic icon
		PixelGlyph IconGlyph;      //the app's icon, its rows pointing into IconRows
		char IconRows[8][9];
		bool Minimized;            //hidden but kept in the taskbar
		bool Maximized;
	};

	enum class Button
	{
		None,
		Minimize,
		Maximize,
		Close,
	};

	//Framebuffer, composition and the desktop drawn once at startup
	GraphicsDevice g_gfx;
	FrameBuffer g_back(nullptr, 0, 0);
	FrameBuffer g_desktop(nullptr, 0, 0);

	//Input
	void* g_inputRegion = nullptr;
	Point2D g_mouse;
	MouseButtonState g_prevButtons;
	bool g_wmPress = false; //the left button went down on something the WM handles, so the WM keeps the mouse until it comes up
	bool g_ctrl = false;
	bool g_alt = false;
	bool g_swallow[256]; //keys whose press the WM took, so their release is taken too

	//Connections + windows
	Connection g_conns[MaxConns];
	WmWindow g_windows[MaxWindows];

	//Z-order: head is topmost. Holds pointers into g_windows (whose slots never move), so the list can be reordered and removed from freely.
	LinkedList<WmWindow*> g_zorder;

	WmWindow* g_focus = nullptr;
	WmWindow* g_drag = nullptr;
	WmWindow* g_pressedWindow = nullptr; //window whose title-bar button is held down
	Button g_pressedButton = Button::None;
	WmWindow* g_lastTitleClick = nullptr;
	milli_t g_lastTitleTime = 0;

	//Start menu and window switcher
	bool g_startMenuOpen = false;
	bool g_menuPress = false;
	size_t g_menuSelection = 0;
	bool g_switching = false;
	size_t g_switchSelection = 0;

	//Taskbar tray
	char g_address[16] = "OFFLINE";
	bool g_online = false;
	milli_t g_lastNetworkPoll = 0;

	int32_t ScreenWidth()
	{
		return (int32_t)g_gfx.Width;
	}

	int32_t TaskbarTop()
	{
		return (int32_t)g_gfx.Height - Theme::TaskbarHeight;
	}

	bool IsFullscreen(const WmWindow* const window)
	{
		return HasStyle(window->Style, WindowStyle::Fullscreen);
	}

	bool IsFramed(const WmWindow* const window)
	{
		return !HasStyle(window->Style, WindowStyle::Chromeless) && !IsFullscreen(window);
	}

	bool IsResizable(const WmWindow* const window)
	{
		return IsFramed(window) && HasStyle(window->Style, WindowStyle::Resizable);
	}

	bool FullscreenActive()
	{
		return g_focus && IsFullscreen(g_focus) && !g_focus->Minimized;
	}

	//The window and its frame
	Rectangle Outer(const WmWindow* const window)
	{
		const Rectangle& c = window->Client;
		if (!IsFramed(window))
			return c;

		return { c.X - Theme::FrameEdge, c.Y - Theme::FrameTop, c.Width + 2 * Theme::FrameEdge, c.Height + Theme::FrameTop + Theme::FrameEdge };
	}

	Rectangle TitleBar(const WmWindow* const window)
	{
		const Rectangle outer = Outer(window);
		return { outer.X + Theme::FrameEdge, outer.Y + Theme::FrameEdge, outer.Width - 2 * Theme::FrameEdge, Theme::TitleHeight };
	}

	//Title-bar buttons from the right: close, then maximize (resizable windows only), then minimize
	Rectangle ButtonRect(const WmWindow* const window, const Button button)
	{
		int32_t slot = 0;
		switch (button)
		{
			case Button::Close:
				slot = 0;
				break;

			case Button::Maximize:
				slot = 1;
				break;

			case Button::Minimize:
				slot = IsResizable(window) ? 2 : 1;
				break;

			default:
				return {};
		}

		const Rectangle bar = TitleBar(window);
		const int32_t x = bar.X + (int32_t)bar.Width - Theme::ButtonMargin - Theme::ButtonWidth - slot * (Theme::ButtonWidth + Theme::ButtonGap);
		return { x, bar.Y + (Theme::TitleHeight - Theme::ButtonHeight) / 2, Theme::ButtonWidth, Theme::ButtonHeight };
	}

	Button ButtonAt(const WmWindow* const window, const Point2D& point)
	{
		if (!IsFramed(window))
			return Button::None;
		if (ButtonRect(window, Button::Close).Contains(point))
			return Button::Close;
		if (IsResizable(window) && ButtonRect(window, Button::Maximize).Contains(point))
			return Button::Maximize;
		if (ButtonRect(window, Button::Minimize).Contains(point))
			return Button::Minimize;
		return Button::None;
	}

	//A fullscreen window's surface, as large as fits the screen without changing its shape, centered
	Rectangle FullscreenRect(const WmWindow* const window)
	{
		const uint32_t width = window->Client.Width;
		const uint32_t height = window->Client.Height;
		uint32_t w = g_gfx.Width;
		uint32_t h = (uint32_t)((uint64_t)height * g_gfx.Width / width);
		if (h > g_gfx.Height)
		{
			h = g_gfx.Height;
			w = (uint32_t)((uint64_t)width * g_gfx.Height / height);
		}
		return { (int32_t)(g_gfx.Width - w) / 2, (int32_t)(g_gfx.Height - h) / 2, w, h };
	}

	//Unpacks the icon the app sent into the window's own pixel art; a window whose app sent none gets the generic icon
	void SetIcon(WmWindow* const window, const Wm::IconBits& icon)
	{
		bool sent = false;
		for (uint32_t y = 0; y < 8; y++)
		{
			for (uint32_t x = 0; x < 8; x++)
			{
				const uint32_t cell = (icon.Rows[y] >> (2 * x)) & 3;
				window->IconRows[y][x] = cell ? (char)('a' + cell - 1) : '.';
			}
			window->IconRows[y][8] = '\0';
			window->IconGlyph.Rows[y] = window->IconRows[y];
			sent |= icon.Rows[y] != 0;
		}

		window->IconGlyph.Width = 8;
		window->IconGlyph.Height = 8;
		for (size_t i = 0; i < 3; i++)
			window->IconGlyph.Palette[i] = icon.Palette[i];
		window->Icon = sent ? &window->IconGlyph : &Icons::Window;
	}

	//Draws text cut to maxChars, ending in ".." when it doesn't fit
	void DrawFitted(const Point2D position, const char* const text, const size_t maxChars, const Color color, const uint32_t scale)
	{
		const size_t length = strlen(text);
		if (length <= maxChars)
		{
			Draw2D::DrawPixelText(g_back, position, cstr_t(text, length), color, scale);
			return;
		}

		const size_t kept = maxChars > 2 ? maxChars - 2 : 0;
		const Point2D end = Draw2D::DrawPixelText(g_back, position, cstr_t(text, kept), color, scale);
		Draw2D::DrawPixelText(g_back, end, Str(".."), color, scale);
	}

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
			if (!window->Minimized && Outer(window).Contains(point))
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

	WmWindow* Find(const Connection* const conn, const uint16_t windowId)
	{
		for (WmWindow& window : g_windows)
		{
			if (window.Surface && window.Owner == conn && window.WindowId == windowId)
				return &window;
		}
		return nullptr;
	}

	//Send a message for a window to its owner's WM->app ring, stamped with the window's id; a droppable one (a mouse move or paint, which the next one replaces) only while the app keeps up
	void Post(const WmWindow* const window, Message message, const bool droppable = false)
	{
		SharedRing<Message> ring = Wm::Channel::Replies(window->Owner->Region);
		if (droppable && ring.Count() >= DroppableBacklog)
			return;
		message.Header.Window = window->WindowId;
		ring.Enqueue(message);
	}

	//Send a message that carries only its type (e.g. PaintEvent, CloseEvent)
	void PostEvent(const WmWindow* const window, const MessageType type, const bool droppable = false)
	{
		Message message = {};
		message.Header.MessageType = type;
		Post(window, message, droppable);
	}

	//Keeps enough of a framed window's title bar on screen (and above the taskbar) to grab it again
	void KeepReachable(WmWindow* const window)
	{
		if (!IsFramed(window))
			return;

		const Rectangle outer = Outer(window);
		const int32_t right = outer.X + (int32_t)outer.Width;
		if (right < TitleGrip)
			window->Client.X += TitleGrip - right;
		if (outer.X > ScreenWidth() - TitleGrip)
			window->Client.X -= outer.X - (ScreenWidth() - TitleGrip);
		if (outer.Y < 0)
			window->Client.Y -= outer.Y;
		if (outer.Y > TaskbarTop() - Theme::FrameTop)
			window->Client.Y -= outer.Y - (TaskbarTop() - Theme::FrameTop);
	}

	//Gives a window focus, showing and raising it; a fullscreen window only shows while it has focus, so one losing it is minimized
	void Focus(WmWindow* const window)
	{
		if (g_focus && g_focus != window && IsFullscreen(g_focus))
			g_focus->Minimized = true;

		g_focus = window;
		if (window)
		{
			window->Minimized = false;
			Raise(window);
		}
	}

	//Maximize fills the space above the taskbar; the app draws at the new size once it answers the ResizeEvent with ResizeWindow
	void ToggleMaximize(WmWindow* const window)
	{
		if (!IsResizable(window))
			return;

		Rectangle target = window->Restore;
		if (!window->Maximized)
		{
			window->Restore = window->Client;
			target = { Theme::FrameEdge, Theme::FrameTop, (uint32_t)(ScreenWidth() - 2 * Theme::FrameEdge), (uint32_t)(TaskbarTop() - Theme::FrameTop - Theme::FrameEdge) };
		}
		window->Maximized = !window->Maximized;
		window->Client.X = target.X;
		window->Client.Y = target.Y;

		Message message = {};
		message.Header.MessageType = MessageType::ResizeEvent;
		message.ResizeEvent.Width = (uint16_t)target.Width;
		message.ResizeEvent.Height = (uint16_t)target.Height;
		Post(window, message);
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

	void HandleRequest(Connection* const conn, const Wm::Request& request)
	{
		//Redeem the app's grant to get our own handle, then map the surface.
		HSharedMem hSurface = nullptr;
		void* surface = nullptr;
		if (!IpcClaim(request.SurfaceGrant, hSurface, surface))
			return;

		switch (request.Code)
		{
		case Wm::Op::AllocWindow:
		{
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
			if (!window || request.Rect.Width == 0 || request.Rect.Height == 0)
			{
				CloseHandle((Handle)hSurface);
				return;
			}

			window->Owner = conn;
			window->WindowId = request.WindowId;
			window->Surface = surface;
			window->SurfaceHandle = hSurface;
			window->Client = request.Rect;
			window->Restore = request.Rect;
			memcpy(window->Title, request.Title, Wm::MaxTitle);
			window->Title[Wm::MaxTitle - 1] = '\0';
			window->Style = request.Style;
			SetIcon(window, request.Icon);
			window->Minimized = false;
			window->Maximized = false;
			KeepReachable(window);

			g_zorder.InsertHead(window); //new window on top
			Focus(window);
			break;
		}

		case Wm::Op::ResizeWindow:
		{
			WmWindow* const window = Find(conn, request.WindowId);
			if (!window || request.Rect.Width == 0 || request.Rect.Height == 0)
			{
				CloseHandle((Handle)hSurface);
				return;
			}

			//Swap in the new surface; the app has already let go of the old one
			CloseHandle((Handle)window->SurfaceHandle);
			window->Surface = surface;
			window->SurfaceHandle = hSurface;
			window->Client.Width = request.Rect.Width;
			window->Client.Height = request.Rect.Height;
			KeepReachable(window);
			break;
		}

		default:
			CloseHandle((Handle)hSurface);
			break;
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
		if (g_pressedWindow == window)
		{
			g_pressedWindow = nullptr;
			g_pressedButton = Button::None;
		}
		if (g_lastTitleClick == window)
			g_lastTitleClick = nullptr;
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
		PostEvent(window, MessageType::CloseEvent);
		DestroyWindow(window);
	}

	//Hide a window from compositing (still listed in the taskbar). Refocus the next visible window.
	void Minimize(WmWindow* const window)
	{
		window->Minimized = true;
		if (g_focus == window)
			g_focus = TopmostVisible();
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

	//----- Desktop: a near-black field with a faint ember glow rising from the bottom edge, drawn once -----

	uint32_t IntSqrt(const uint64_t value)
	{
		uint64_t result = 0;
		uint64_t bit = 1ull << 62;
		while (bit > value)
			bit >>= 2;

		uint64_t rest = value;
		while (bit != 0)
		{
			if (rest >= result + bit)
			{
				rest -= result + bit;
				result = (result >> 1) + bit;
			}
			else
			{
				result >>= 1;
			}
			bit >>= 2;
		}
		return (uint32_t)result;
	}

	void BuildDesktop()
	{
		const int32_t width = (int32_t)g_gfx.Width;
		const int32_t height = (int32_t)g_gfx.Height;
		Draw2D::FillScreen(g_desktop, Theme::Void);

		//An ellipse centered below the bottom edge; t is the distance from its center in thousandths of its radius
		const int64_t centerX = width / 2;
		const int64_t centerY = (int64_t)height * 112 / 100;
		const int64_t radiusX = (int64_t)width * 1500 / 1920;
		const int64_t radiusY = (int64_t)height * 560 / 1080;
		static constexpr Color inner = Theme::Rgb(0xFF6018);
		static constexpr Color outer = Theme::Rgb(0xA0280C);
		for (int32_t y = 0; y < height; y++)
		{
			const int64_t dy = (y - centerY) * 1000 / radiusY;
			if (dy < -720 || dy > 720)
				continue;

			Color* const row = g_desktop.Buffer + (size_t)y * g_desktop.Width;
			for (int32_t x = 0; x < width; x++)
			{
				const int64_t dx = (x - centerX) * 1000 / radiusX;
				const uint32_t t = IntSqrt((uint64_t)(dx * dx + dy * dy));
				if (t >= 720)
					continue;

				//Glow strength in thousandths: 170 at the center, 70 by t = 420, gone by t = 720
				Color glow = outer;
				uint32_t alpha = 0;
				if (t <= 420)
				{
					glow = Draw2D::Blend(inner, outer, t, 420);
					alpha = 170 - 100 * t / 420;
				}
				else
				{
					alpha = 70 * (720 - t) / 300;
				}
				row[x] = Draw2D::Blend(Theme::Void, glow, alpha, 1000);
			}
		}

		//A wordmark low on the right, pressed into the background: a dark edge above, then letters a shade lighter than what's under them
		static constexpr uint32_t scale = 6;
		const cstr_t mark = Str("METALOS");
		const int32_t markX = width - 72 - (int32_t)Draw2D::PixelTextWidth(mark.Size, scale);
		const int32_t markY = height - 96 - (int32_t)(8 * scale);
		for (int32_t pass = 0; pass < 2; pass++)
		{
			for (size_t i = 0; i < mark.Size; i++)
			{
				const uint8_t* const map = Draw2D::m_font.GetCharacterMap(mark[i]);
				for (uint32_t row = 0; row < 8; row++)
				{
					for (uint32_t col = 0; col < 8; col++)
					{
						if ((map[row] & (1u << col)) == 0)
							continue;

						const Rectangle cell = { markX + (int32_t)((i * 8 + col) * scale), markY + (int32_t)(row * scale) - (pass == 0 ? 2 : 0), scale, scale };
						if (pass == 0)
						{
							Draw2D::Darken(g_desktop, cell, 150);
							continue;
						}
						for (uint32_t y = 0; y < scale; y++)
						{
							Color* const pixel = g_desktop.Buffer + (size_t)(cell.Y + (int32_t)y) * g_desktop.Width + cell.X;
							for (uint32_t x = 0; x < scale; x++)
								pixel[x] = Draw2D::Blend(pixel[x], Theme::Steel, 7, 100);
						}
					}
				}
			}
		}
	}

	//0 is Sunday (Sakamoto's method), worked out from the date since the clock chip's own weekday isn't kept reliably
	size_t WeekDay(uint32_t year, const uint32_t month, const uint32_t day)
	{
		static constexpr uint32_t Offsets[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
		if (month < 3)
			year--;
		return (size_t)((year + year / 4 - year / 100 + year / 400 + Offsets[month - 1] + day) % 7);
	}

	//----- Window frames -----

	//A soft shadow under and beside a window: 1px bands that fade out with distance
	void DrawShadow(const Rectangle& outer)
	{
		const int32_t right = outer.X + (int32_t)outer.Width;
		const int32_t bottom = outer.Y + (int32_t)outer.Height;
		for (int32_t i = 0; i < 6; i++)
		{
			const uint32_t keep = 130 + (uint32_t)i * 20;
			const uint32_t side = outer.Height > 10 ? outer.Height - 10 + (uint32_t)i : 0;
			Draw2D::Darken(g_back, { outer.X - i, bottom + i, outer.Width + 2 * (uint32_t)i, 1 }, keep);
			Draw2D::Darken(g_back, { right + i, outer.Y + 10, 1, side }, keep);
			Draw2D::Darken(g_back, { outer.X - 1 - i, outer.Y + 10, 1, side }, keep);
		}
	}

	void DrawTitleButton(const WmWindow* const window, const Button button, const bool focused)
	{
		const Rectangle rect = ButtonRect(window, button);
		const bool hover = !g_switching && rect.Contains(g_mouse) && HitTest(g_mouse) == window;
		const bool pressed = hover && g_pressedWindow == window && g_pressedButton == button;

		const Theme::Face* face = &Theme::Raised;
		if (button == Button::Close)
			face = pressed ? &Theme::DangerSunken : (hover ? &Theme::DangerHover : &Theme::Raised);
		else
			face = pressed ? &Theme::Sunken : (hover ? &Theme::RaisedHover : &Theme::Raised);
		Draw2D::DrawFace(g_back, rect, *face);

		const PixelGlyph* glyph = &Icons::Minimize;
		if (button == Button::Maximize)
			glyph = window->Maximized ? &Icons::Restore : &Icons::Maximize;
		else if (button == Button::Close)
			glyph = &Icons::Close;
		const uint32_t keep = (focused || hover) ? 256 : 150;
		Draw2D::DrawGlyph(g_back, { rect.X + (Theme::ButtonWidth - 10) / 2, rect.Y + (Theme::ButtonHeight - 10) / 2 }, *glyph, 2, keep);
	}

	void DrawWindow(const WmWindow* const window)
	{
		const bool focused = (window == g_focus);
		if (!IsFramed(window))
		{
			if (focused)
				Draw2D::WriteFrame(g_back, window->Client, window->Surface);
			else
				Draw2D::WriteFrameDimmed(g_back, window->Client, window->Surface, DimKeep);
			return;
		}

		const Rectangle outer = Outer(window);
		DrawShadow(outer);
		if (focused)
			Draw2D::DrawOutline(g_back, { outer.X - 1, outer.Y - 1, outer.Width + 2, outer.Height + 2 }, Theme::Rgb(0x2A170F));
		Draw2D::DrawOutline(g_back, outer, Theme::Outline);
		Draw2D::DrawBevel(g_back, { outer.X + 1, outer.Y + 1, outer.Width - 2, outer.Height - 2 }, focused ? Theme::BevelLight : Theme::Rgb(0x34373D), Theme::BevelDark);

		//Title bar: gradient, then the accent line that marks focus
		const Rectangle bar = TitleBar(window);
		Draw2D::FillGradient(g_back, bar, focused ? Theme::GunmetalTop : Theme::GunmetalDimTop, focused ? Theme::GunmetalBottom : Theme::GunmetalDimBottom);
		Draw2D::DrawRectangle(g_back, focused ? Theme::Ember : Theme::AccentOff, { bar.X, bar.Y + Theme::TitleHeight, bar.Width, Theme::AccentHeight });

		const int32_t textY = bar.Y + (Theme::TitleHeight - Theme::Glyph) / 2;
		Draw2D::DrawGlyph(g_back, { bar.X + 8, textY }, *window->Icon, Theme::TextScale, focused ? 256 : 150);

		const int32_t textX = bar.X + 8 + Theme::Glyph + 10;
		const int32_t buttonsLeft = ButtonRect(window, Button::Minimize).X;
		const size_t maxChars = buttonsLeft - 8 > textX ? (size_t)(buttonsLeft - 8 - textX) / Theme::Glyph : 0;
		if (focused)
			DrawFitted({ textX + 2, textY + 2 }, window->Title, maxChars, Theme::Rgb(0x000000), Theme::TextScale);
		DrawFitted({ textX, textY }, window->Title, maxChars, focused ? Theme::Bone : Theme::TextDim, Theme::TextScale);

		DrawTitleButton(window, Button::Minimize, focused);
		if (IsResizable(window))
			DrawTitleButton(window, Button::Maximize, focused);
		DrawTitleButton(window, Button::Close, focused);

		if (focused)
			Draw2D::WriteFrame(g_back, window->Client, window->Surface);
		else
			Draw2D::WriteFrameDimmed(g_back, window->Client, window->Surface, DimKeep);
	}

	//----- Taskbar: start button, a button per window, network status and the clock -----

	static constexpr int32_t BarButtonTop = 5;
	static constexpr int32_t BarButtonHeight = 30;
	static constexpr int32_t StartX = 6;
	static constexpr int32_t StartWidth = 10 + Theme::Glyph + 10 + 7 * Theme::Glyph + 14;
	static constexpr int32_t TaskX = StartX + StartWidth + 12;
	static constexpr int32_t TaskWidth = 232;
	static constexpr int32_t TaskGap = 6;
	static constexpr size_t TaskChars = (TaskWidth - 10 - Theme::Glyph - 10 - 10) / Theme::Glyph;
	static constexpr int32_t ClockWidth = 5 * Theme::Glyph;

	static constexpr Theme::Face StartFace = { Theme::Rgb(0x2C2E33), Theme::Rgb(0x202125), Theme::Rgb(0x4A4E56), Theme::Rgb(0x121316), Theme::Bone };
	static constexpr Theme::Face StartHover = { Theme::Rgb(0x35373D), Theme::Rgb(0x27292D), Theme::Rgb(0x50545C), Theme::Rgb(0x121316), Theme::Bone };
	static constexpr Theme::Face StartOpen = { Theme::Rgb(0x18191C), Theme::Rgb(0x1F2024), Theme::Rgb(0x070708), Theme::Rgb(0x2F3238), Theme::Molten };
	static constexpr Theme::Face TaskActive = { Theme::Rgb(0x35373D), Theme::Rgb(0x27292D), Theme::Rgb(0x4A4E56), Theme::Rgb(0x27292D), Theme::Bone };
	static constexpr Theme::Face TaskHover = { Theme::Rgb(0x212226), Theme::Rgb(0x212226), Theme::Rgb(0x2F3238), Theme::Rgb(0x212226), Theme::Bone };
	static constexpr Theme::Face TaskBehind = { Theme::Rgb(0x16171A), Theme::Rgb(0x16171A), Theme::Rgb(0x070708), Theme::Rgb(0x26282C), Theme::Rgb(0x9B978F) };
	static constexpr Theme::Face TaskMinimized = { Theme::Rgb(0x111214), Theme::Rgb(0x111214), Theme::Rgb(0x070708), Theme::Rgb(0x1D1E21), Theme::Rgb(0x5F5B56) };

	Rectangle StartButtonRect()
	{
		return { StartX, TaskbarTop() + BarButtonTop, StartWidth, BarButtonHeight };
	}

	//Left edge of the tray (network status and clock); task buttons stop before it
	int32_t TrayLeft()
	{
		const int32_t clockX = ScreenWidth() - 14 - ClockWidth;
		const int32_t separator = clockX - 18;
		return separator - 10 - (int32_t)Draw2D::PixelTextWidth(strlen(g_address), Theme::TextScale) - 10 - Theme::Glyph;
	}

	//Taskbar layout: one button per window in slot order until the tray; fn(window, rect) returns true to stop
	template <typename TFn>
	void ForEachTaskButton(TFn&& fn)
	{
		int32_t x = TaskX;
		const int32_t limit = TrayLeft() - 12;
		for (WmWindow& window : g_windows)
		{
			if (!window.Surface)
				continue;
			if (x + TaskWidth > limit)
				return;
			if (fn(window, Rectangle{ x, TaskbarTop() + BarButtonTop, TaskWidth, BarButtonHeight }))
				return;
			x += TaskWidth + TaskGap;
		}
	}

	//The address NetSvr publishes once dhcp configures it, checked about once a second
	void PollNetwork()
	{
		const milli_t now = GetTickCount();
		if (now - g_lastNetworkPoll < NetworkPollTime && g_lastNetworkPoll != 0)
			return;
		g_lastNetworkPoll = now;

		uint64_t value = 0;
		g_online = LookupEndpoint(NetIpc::AddressEndpoint, &value) == SyscallResult::Success && (uint32_t)value != 0;
		if (!g_online)
		{
			strcpy(g_address, "OFFLINE");
			return;
		}

		//Network byte order keeps the first octet in the low byte
		const uint32_t address = (uint32_t)value;
		snprintf(g_address, sizeof(g_address), "%u.%u.%u.%u", address & 0xFF, (address >> 8) & 0xFF, (address >> 16) & 0xFF, address >> 24);
	}

	void DrawTaskButton(const WmWindow& window, const Rectangle& rect)
	{
		const bool active = (&window == g_focus) && !window.Minimized;
		const bool hover = !active && rect.Contains(g_mouse);
		const Theme::Face& face = active ? TaskActive : (hover ? TaskHover : (window.Minimized ? TaskMinimized : TaskBehind));
		Draw2D::DrawFace(g_back, rect, face);
		if (active)
			Draw2D::DrawRectangle(g_back, Theme::Ember, { rect.X + 1, rect.Y + (int32_t)rect.Height - 2, rect.Width - 2, 2 });

		//A minimized window's button has a broken edge, like it's only half there
		if (window.Minimized && !hover)
		{
			for (int32_t x = rect.X + 2; x < rect.X + (int32_t)rect.Width - 2; x += 8)
			{
				Draw2D::DrawRectangle(g_back, Theme::Divider, { x, rect.Y, 4, 1 });
				Draw2D::DrawRectangle(g_back, Theme::Divider, { x, rect.Y + (int32_t)rect.Height - 1, 4, 1 });
			}
		}

		const uint32_t keep = active || hover ? 256 : (window.Minimized ? 110 : 180);
		const int32_t y = rect.Y + (BarButtonHeight - Theme::Glyph) / 2;
		Draw2D::DrawGlyph(g_back, { rect.X + 10, y }, *window.Icon, Theme::TextScale, keep);
		DrawFitted({ rect.X + 10 + Theme::Glyph + 10, y }, window.Title, TaskChars, face.Label, Theme::TextScale);
	}

	void DrawTaskbar()
	{
		const int32_t top = TaskbarTop();
		const uint32_t width = g_gfx.Width;
		Draw2D::FillGradient(g_back, { 0, top, width, Theme::TaskbarHeight }, Theme::Rgb(0x1C1D21), Theme::Rgb(0x111214));
		Draw2D::DrawRectangle(g_back, Theme::Rgb(0x000000), { 0, top - 1, width, 1 });
		Draw2D::DrawRectangle(g_back, Theme::Rgb(0x3B3E45), { 0, top, width, 1 });

		//Start button
		const Rectangle start = StartButtonRect();
		const Theme::Face& startFace = g_startMenuOpen ? StartOpen : (start.Contains(g_mouse) ? StartHover : StartFace);
		Draw2D::DrawFace(g_back, start, startFace);
		const int32_t textY = start.Y + (BarButtonHeight - Theme::Glyph) / 2;
		Draw2D::DrawGlyph(g_back, { start.X + 10, textY }, Icons::Flame, Theme::TextScale);
		Draw2D::DrawPixelText(g_back, { start.X + 10 + Theme::Glyph + 10, textY }, Str("MetalOS"), startFace.Label, Theme::TextScale);
		Draw2D::DrawRectangle(g_back, Theme::Divider, { start.X + StartWidth + 5, top + 7, 1, 26 });

		ForEachTaskButton([&](const WmWindow& window, const Rectangle& rect)
		{
			DrawTaskButton(window, rect);
			return false;
		});

		//Tray: network status, a separator, then the time over the date
		const int32_t clockX = ScreenWidth() - 14 - ClockWidth;
		const int32_t separator = clockX - 18;
		const int32_t addressX = separator - 10 - (int32_t)Draw2D::PixelTextWidth(strlen(g_address), Theme::TextScale);
		Draw2D::DrawGlyph(g_back, { addressX - 10 - Theme::Glyph, textY }, Icons::Network, Theme::TextScale, g_online ? 256 : 90);
		Draw2D::DrawPixelText(g_back, { addressX, textY }, Str(g_address), g_online ? Theme::Rgb(0xC9C3B6) : Theme::Muted, Theme::TextScale);
		Draw2D::DrawRectangle(g_back, Theme::Divider, { separator, top + 7, 1, 26 });

		SystemTime time = {};
		if (GetSystemTime(&time) == SyscallResult::Success)
		{
			static constexpr const char* Days[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
			static constexpr const char* Months[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };
			char clock[8];
			snprintf(clock, sizeof(clock), "%02d:%02d", time.Hour, time.Minute);
			Draw2D::DrawPixelText(g_back, { clockX, top + 6 }, Str(clock), Theme::Bone, Theme::TextScale);

			const uint32_t month = time.Month >= 1 && time.Month <= 12 ? time.Month : 1;
			const size_t day = WeekDay(time.Year, month, time.Day);
			char date[16];
			const int length = snprintf(date, sizeof(date), "%s %02d %s", Days[day], time.Day, Months[month - 1]);
			const int32_t dateX = ScreenWidth() - 14 - (int32_t)Draw2D::PixelTextWidth((size_t)length, 1);
			Draw2D::DrawPixelText(g_back, { dateX, top + 26 }, cstr_t(date, (size_t)length), Theme::Steel, 1);
		}
	}

	void HandleTaskbarPress(const Point2D& point)
	{
		//Start button -> toggle the app menu
		if (StartButtonRect().Contains(point))
		{
			g_startMenuOpen = !g_startMenuOpen;
			g_menuSelection = 0;
			return;
		}

		//Any other taskbar click dismisses the menu.
		g_startMenuOpen = false;

		//Taskbar button -> minimize the focused window, otherwise restore/focus/raise it
		ForEachTaskButton([&](WmWindow& window, const Rectangle& rect)
		{
			if (!rect.Contains(point))
				return false;

			if (&window == g_focus && !window.Minimized)
				Minimize(&window);
			else
				Focus(&window);
			return true;
		});
	}

	//----- Start menu -----

	struct AppEntry
	{
		const char* Label;
		const char* Hint;
		const char* Exe;
		const PixelGlyph* Icon;
	};

	static constexpr AppEntry g_apps[] =
	{
		{ "Terminal", "Command shell", "term.exe", &Icons::Terminal },
		{ "Calc", "Four-function calculator", "calc.exe", &Icons::Calc },
		{ "Fire", "The Doom fire effect", "fire.exe", &Icons::Flame },
		{ "Doom", "Ultimate Doom, full screen", "doom.exe", &Icons::Skull },
	};
	static constexpr size_t g_appCount = sizeof(g_apps) / sizeof(g_apps[0]);

	static constexpr int32_t MenuWidth = 380;
	static constexpr int32_t MenuHeader = 72;
	static constexpr int32_t MenuPad = 6;
	static constexpr int32_t MenuItemHeight = 56;
	static constexpr int32_t MenuFooter = 36;

	//The start menu sits just above the taskbar, aligned with the Start button
	Rectangle StartMenuRect()
	{
		const uint32_t height = MenuHeader + 2 * MenuPad + (uint32_t)g_appCount * MenuItemHeight + MenuFooter;
		return { StartX, TaskbarTop() - 6 - (int32_t)height, MenuWidth, height };
	}

	Rectangle MenuItemRect(const size_t index)
	{
		const Rectangle menu = StartMenuRect();
		return { menu.X + 2, menu.Y + MenuHeader + MenuPad + (int32_t)index * MenuItemHeight, menu.Width - 4, MenuItemHeight };
	}

	bool InStartMenu(const Point2D& point)
	{
		return g_startMenuOpen && StartMenuRect().Contains(point);
	}

	void Launch(const size_t index)
	{
		g_startMenuOpen = false;
		if (index < g_appCount)
			CreateProcess(g_apps[index].Exe, nullptr, nullptr);
	}

	void DrawStartMenu()
	{
		if (!g_startMenuOpen)
			return;

		const Rectangle menu = StartMenuRect();
		DrawShadow(menu);
		Draw2D::DrawOutline(g_back, menu, Theme::Outline);
		const Rectangle inner = { menu.X + 1, menu.Y + 1, menu.Width - 2, menu.Height - 2 };
		Draw2D::FillGradient(g_back, inner, Theme::Rgb(0x1A1B1F), Theme::Rgb(0x141518));
		Draw2D::DrawBevel(g_back, inner, Theme::BevelLight, Theme::BevelDark);

		//Header: the wordmark over an ember line
		Draw2D::DrawRectangle(g_back, Theme::Rgb(0x111214), { menu.X + 2, menu.Y + 2, menu.Width - 4, MenuHeader - 4 });
		Draw2D::DrawRectangle(g_back, Theme::Ember, { menu.X + 2, menu.Y + MenuHeader - 2, menu.Width - 4, 2 });
		Draw2D::DrawGlyph(g_back, { menu.X + 16, menu.Y + 18 }, Icons::Flame, 4);
		Draw2D::DrawPixelText(g_back, { menu.X + 67, menu.Y + 27 }, Str("METALOS"), Theme::Rgb(0x000000), 3);
		Draw2D::DrawPixelText(g_back, { menu.X + 64, menu.Y + 24 }, Str("METALOS"), Theme::EmberText, 3);

		for (size_t i = 0; i < g_appCount; i++)
		{
			const Rectangle item = MenuItemRect(i);
			const bool selected = (i == g_menuSelection);
			if (selected)
			{
				Draw2D::DrawRectangle(g_back, Theme::Selection, item);
				Draw2D::DrawOutline(g_back, item, Theme::SelectionEdge);
			}
			Draw2D::DrawGlyph(g_back, { item.X + 14, item.Y + 12 }, *g_apps[i].Icon, 4);
			Draw2D::DrawPixelText(g_back, { item.X + 60, item.Y + 12 }, CStr(g_apps[i].Label), selected ? Theme::Molten : Theme::Bone, Theme::TextScale);
			Draw2D::DrawPixelText(g_back, { item.X + 60, item.Y + 36 }, CStr(g_apps[i].Hint), selected ? Theme::Rgb(0xB4A294) : Theme::Steel, 1);
		}

		//Footer: the keyboard shortcuts
		const int32_t footer = menu.Y + (int32_t)menu.Height - MenuFooter;
		Draw2D::DrawRectangle(g_back, Theme::AccentOff, { menu.X + 2, footer, menu.Width - 4, 1 });
		Draw2D::DrawPixelText(g_back, { menu.X + 16, footer + 14 }, Str("CTRL+ESC MENU"), Theme::Hint, 1);
		const cstr_t switchHint = Str("ALT+TAB SWITCH");
		Draw2D::DrawPixelText(g_back, { menu.X + (int32_t)menu.Width - 16 - (int32_t)Draw2D::PixelTextWidth(switchHint.Size, 1), footer + 14 }, switchHint, Theme::Hint, 1);
	}

	void OpenStartMenu()
	{
		//A fullscreen window would hide the menu, so it steps aside
		if (FullscreenActive())
			Minimize(g_focus);
		g_startMenuOpen = true;
		g_menuSelection = 0;
	}

	//----- Window switcher (Alt+Tab) -----

	static constexpr int32_t TileWidth = 240;
	static constexpr int32_t TileGap = 24;
	static constexpr int32_t ThumbHeight = 128;
	static constexpr int32_t TileHeight = 2 + 8 + ThumbHeight + 12 + Theme::Glyph + 8 + 2;
	static constexpr size_t TileChars = (TileWidth - 4 - 16 - Theme::Glyph - 10) / Theme::Glyph;

	//Windows in switch order: the z-order from the top, which is also most recently focused first
	size_t SwitchList(WmWindow** const list, const size_t max)
	{
		size_t count = 0;
		for (WmWindow* const window : g_zorder)
		{
			if (count == max)
				break;
			list[count++] = window;
		}
		return count;
	}

	void StartSwitch()
	{
		WmWindow* list[MaxWindows];
		const size_t count = SwitchList(list, MaxWindows);
		if (count == 0)
			return;

		g_startMenuOpen = false;
		g_switching = true;
		g_switchSelection = count > 1 ? 1 : 0;
	}

	void AdvanceSwitch()
	{
		WmWindow* list[MaxWindows];
		const size_t count = SwitchList(list, MaxWindows);
		g_switchSelection = count ? (g_switchSelection + 1) % count : 0;
	}

	void CommitSwitch()
	{
		g_switching = false;
		WmWindow* list[MaxWindows];
		const size_t count = SwitchList(list, MaxWindows);
		if (count != 0)
			Focus(list[g_switchSelection < count ? g_switchSelection : 0]);
	}

	//A window's client area scaled to fit inside a box, keeping its shape
	void DrawThumbnail(const WmWindow* const window, const Rectangle& box)
	{
		const uint32_t width = window->Client.Width;
		const uint32_t height = window->Client.Height;
		uint32_t w = box.Width;
		uint32_t h = (uint32_t)((uint64_t)height * box.Width / width);
		if (h > box.Height)
		{
			h = box.Height;
			w = (uint32_t)((uint64_t)width * box.Height / height);
		}
		if (w == 0 || h == 0)
			return;
		Draw2D::WriteFrameScaled(g_back, { box.X + (int32_t)(box.Width - w) / 2, box.Y + (int32_t)(box.Height - h) / 2, w, h }, window->Surface, width, height);
	}

	void DrawSwitcher()
	{
		if (!g_switching)
			return;

		WmWindow* list[MaxWindows];
		const size_t count = SwitchList(list, MaxWindows);
		if (count == 0)
		{
			g_switching = false;
			return;
		}
		if (g_switchSelection >= count)
			g_switchSelection = 0;

		//As many tiles as fit across the screen, scrolled so the selection shows
		size_t shown = (size_t)((ScreenWidth() - 120 + TileGap) / (TileWidth + TileGap));
		if (shown > count)
			shown = count;
		const size_t first = g_switchSelection >= shown ? g_switchSelection - shown + 1 : 0;

		Draw2D::Darken(g_back, { 0, 0, g_gfx.Width, g_gfx.Height }, 110);

		const uint32_t panelWidth = (uint32_t)(shown * TileWidth + (shown - 1) * TileGap + 44);
		static constexpr uint32_t panelHeight = 18 + 8 + 18 + TileHeight + 22;
		const Rectangle panel = { (ScreenWidth() - (int32_t)panelWidth) / 2, ((int32_t)g_gfx.Height - (int32_t)panelHeight) / 2, panelWidth, panelHeight };
		DrawShadow(panel);
		Draw2D::DrawRectangle(g_back, Theme::Rgb(0x141518), panel);
		Draw2D::DrawOutline(g_back, panel, Theme::Outline);
		Draw2D::DrawRectangle(g_back, Theme::Ember, { panel.X, panel.Y, panel.Width, 2 });

		Draw2D::DrawPixelText(g_back, { panel.X + 22, panel.Y + 18 }, Str("SWITCH WINDOW"), Theme::Hint, 1);
		const cstr_t hint = Str("HOLD ALT, TAB TO MOVE");
		Draw2D::DrawPixelText(g_back, { panel.X + (int32_t)panel.Width - 22 - (int32_t)Draw2D::PixelTextWidth(hint.Size, 1), panel.Y + 18 }, hint, Theme::Hint, 1);

		for (size_t i = 0; i < shown; i++)
		{
			const WmWindow* const window = list[first + i];
			const bool selected = (first + i == g_switchSelection);
			const Rectangle tile = { panel.X + 22 + (int32_t)i * (TileWidth + TileGap), panel.Y + 18 + 8 + 18, TileWidth, TileHeight };
			if (selected)
			{
				Draw2D::DrawRectangle(g_back, Theme::Rgb(0x1D1612), tile);
				Draw2D::DrawOutline(g_back, tile, Theme::Ember);
				Draw2D::DrawOutline(g_back, { tile.X + 1, tile.Y + 1, tile.Width - 2, tile.Height - 2 }, Theme::Ember);
			}

			//Thumbnail: a sliver of title bar over the window's contents
			const Rectangle thumb = { tile.X + 10, tile.Y + 10, TileWidth - 20, ThumbHeight };
			Draw2D::DrawRectangle(g_back, Theme::IronDeep, thumb);
			Draw2D::FillGradient(g_back, { thumb.X, thumb.Y, thumb.Width, 8 }, Theme::GunmetalTop, Theme::GunmetalBottom);
			if (selected)
				Draw2D::DrawRectangle(g_back, Theme::Ember, { thumb.X, thumb.Y + 8, thumb.Width, 1 });
			DrawThumbnail(window, { thumb.X + 4, thumb.Y + 12, thumb.Width - 8, thumb.Height - 16 });
			Draw2D::DrawOutline(g_back, thumb, Theme::Outline);

			const int32_t labelY = thumb.Y + ThumbHeight + 12;
			Draw2D::DrawGlyph(g_back, { tile.X + 10, labelY }, *window->Icon, Theme::TextScale, selected ? 256 : 180);
			DrawFitted({ tile.X + 10 + Theme::Glyph + 10, labelY }, window->Title, TileChars, selected ? Theme::Bone : Theme::Rgb(0x9B978F), Theme::TextScale);
		}
	}

	//----- Input -----

	//Keys the WM keeps for itself: Alt+Tab, Ctrl+Esc and the open start menu; true means the app doesn't see it
	bool HandleHotkey(const VirtualKey key)
	{
		if (g_switching)
		{
			if (key == VK_TAB)
				AdvanceSwitch();
			else if (key == VK_ESCAPE)
				g_switching = false;
			return true;
		}

		if (g_alt && key == VK_TAB)
		{
			StartSwitch();
			return true;
		}

		if (g_ctrl && key == VK_ESCAPE)
		{
			if (g_startMenuOpen)
				g_startMenuOpen = false;
			else
				OpenStartMenu();
			return true;
		}

		if (!g_startMenuOpen)
			return false;

		switch (key)
		{
			case VK_UP:
				g_menuSelection = g_menuSelection == 0 ? g_appCount - 1 : g_menuSelection - 1;
				break;

			case VK_DOWN:
				g_menuSelection = (g_menuSelection + 1) % g_appCount;
				break;

			case VK_RETURN:
				Launch(g_menuSelection);
				break;

			case VK_ESCAPE:
				g_startMenuOpen = false;
				break;
		}
		return true;
	}

	void OnKey(const Message& message)
	{
		const VirtualKey key = message.KeyEvent.Key;
		const bool pressed = message.KeyEvent.Flags.Pressed;
		const size_t slot = key & 0xFF;

		if (key == VK_CONTROL)
			g_ctrl = pressed;

		//Alt always reaches the app, and letting go of it ends a switch once the window losing focus has seen it
		if (key == VK_MENU)
		{
			g_alt = pressed;
			if (g_focus)
				Post(g_focus, message);
			if (!pressed && g_switching)
				CommitSwitch();
			return;
		}

		if (!pressed && g_swallow[slot])
		{
			g_swallow[slot] = false;
			return;
		}

		if (pressed && HandleHotkey(key))
		{
			g_swallow[slot] = true;
			return;
		}

		if (g_focus)
			Post(g_focus, message);
	}

	void OnPress(const Point2D& point)
	{
		//Mid-switch clicks do nothing, and a fullscreen window takes the whole mouse
		if (g_switching)
		{
			g_wmPress = true;
			return;
		}
		if (FullscreenActive())
			return;

		if (InStartMenu(point))
		{
			g_menuPress = true;
			g_wmPress = true;
			return;
		}

		if (point.Y >= TaskbarTop())
		{
			HandleTaskbarPress(point);
			g_wmPress = true;
			return;
		}

		//A click anywhere else dismisses the start menu
		g_startMenuOpen = false;
		WmWindow* const hit = HitTest(point);
		if (!hit)
			return;

		Focus(hit);
		const Button button = ButtonAt(hit, point);
		if (button != Button::None)
		{
			g_pressedWindow = hit;
			g_pressedButton = button;
			g_wmPress = true;
			return;
		}

		if (IsFramed(hit) && TitleBar(hit).Contains(point))
		{
			//Only the title bar drags a window, and the app sees none of it; a second click soon after maximizes
			g_wmPress = true;
			const milli_t now = GetTickCount();
			if (g_lastTitleClick == hit && now - g_lastTitleTime < DoubleClickTime)
			{
				g_lastTitleClick = nullptr;
				ToggleMaximize(hit);
			}
			else
			{
				g_lastTitleClick = hit;
				g_lastTitleTime = now;
				if (!hit->Maximized)
					g_drag = hit;
			}
			return;
		}

		//The frame's edges belong to the WM; a press in the client area goes to the app
		if (!hit->Client.Contains(point))
			g_wmPress = true;
	}

	void OnRelease(const Point2D& point)
	{
		//A title-bar button acts when it's released over the button it was pressed on
		if (g_pressedButton != Button::None)
		{
			WmWindow* const window = g_pressedWindow;
			const Button button = g_pressedButton;
			g_pressedWindow = nullptr;
			g_pressedButton = Button::None;
			if (window && window->Surface && ButtonAt(window, point) == button)
			{
				if (button == Button::Close)
					CloseWindow(window);
				else if (button == Button::Minimize)
					Minimize(window);
				else
					ToggleMaximize(window);
			}
		}

		if (g_menuPress)
		{
			g_menuPress = false;
			for (size_t i = 0; i < g_appCount; i++)
			{
				if (InStartMenu(point) && MenuItemRect(i).Contains(point))
					Launch(i);
			}
		}

		if (g_drag)
			KeepReachable(g_drag);
		g_drag = nullptr;
	}

	void OnMouse(Message& message)
	{
		const MouseButtonState buttons = message.MouseEvent.Buttons;
		const bool move = buttons == g_prevButtons;
		const Point2D point = { (int32_t)((uint32_t)g_gfx.Width * (uint16_t)message.MouseEvent.XPosition / HidMax), (int32_t)((uint32_t)g_gfx.Height * (uint16_t)message.MouseEvent.YPosition / HidMax) };
		const bool down = buttons.LeftPressed && !g_prevButtons.LeftPressed;
		const bool up = !buttons.LeftPressed && g_prevButtons.LeftPressed;

		if (down)
			OnPress(point);
		else if (buttons.LeftPressed && g_drag)
		{
			g_drag->Client.X += point.X - g_mouse.X;
			g_drag->Client.Y += point.Y - g_mouse.Y;
		}
		if (up)
			OnRelease(point);

		//The menu item under the mouse is the selected one
		if (g_startMenuOpen)
		{
			for (size_t i = 0; i < g_appCount; i++)
			{
				if (MenuItemRect(i).Contains(point))
					g_menuSelection = i;
			}
		}

		g_mouse = point;
		g_prevButtons = buttons;

		//The app doesn't see a press the WM handled, nor the moves and release that follow it
		if (g_wmPress)
		{
			if (up)
				g_wmPress = false;
			return;
		}
		if (!g_focus)
			return;

		//Apps hit-test their controls in client coordinates; a fullscreen window's are its surface's
		if (FullscreenActive())
		{
			const Rectangle fit = FullscreenRect(g_focus);
			message.MouseEvent.XPosition = (int16_t)((int64_t)(point.X - fit.X) * g_focus->Client.Width / fit.Width);
			message.MouseEvent.YPosition = (int16_t)((int64_t)(point.Y - fit.Y) * g_focus->Client.Height / fit.Height);
		}
		else
		{
			message.MouseEvent.XPosition = (int16_t)(point.X - g_focus->Client.X);
			message.MouseEvent.YPosition = (int16_t)(point.Y - g_focus->Client.Y);
		}
		Post(g_focus, message, move);
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
				OnMouse(message);
			else if (message.Header.MessageType == MessageType::KeyEvent)
				OnKey(message);
		}
	}

	void Composite()
	{
		PollNetwork();

		if (FullscreenActive())
		{
			//A fullscreen window covers everything, with black bars where its shape doesn't match the screen
			const Rectangle fit = FullscreenRect(g_focus);
			Draw2D::FillScreen(g_back, Theme::Rgb(0x000000));
			Draw2D::WriteFrameScaled(g_back, fit, g_focus->Surface, g_focus->Client.Width, g_focus->Client.Height);
		}
		else
		{
			memcpy(g_back.Buffer, g_desktop.Buffer, g_back.Size());

			//Draw bottom to top: Reversed() walks tail -> head, i.e. bottommost first.
			for (WmWindow* const window : g_zorder.Reversed())
			{
				if (!window->Minimized)
					DrawWindow(window);
			}

			//The taskbar draws over windows, and the start menu over the taskbar
			DrawTaskbar();
			DrawStartMenu();
		}
		DrawSwitcher();

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
				PostEvent(&window, MessageType::PaintEvent, true);
		}
	}
}

int main(int argc, char** argv)
{
	UNUSED(argc);
	UNUSED(argv);
	//Map the framebuffer into this process
	if (MapFramebuffer(&g_gfx) != SyscallResult::Success)
		return 1;
	DebugPrintf("WM: framebuffer %dx%d, stride %d, base %p\n", g_gfx.Width, g_gfx.Height, g_gfx.PixelsPerScanLine, g_gfx.FrameBase);

	//Backbuffer and desktop (packed Width*Height)
	void* const back = VirtualAlloc(nullptr, (size_t)g_gfx.Width * g_gfx.Height * sizeof(Color));
	void* const desktop = VirtualAlloc(nullptr, (size_t)g_gfx.Width * g_gfx.Height * sizeof(Color));
	if (!back || !desktop)
		return 1;
	g_back = FrameBuffer(static_cast<Color*>(back), g_gfx.Height, g_gfx.Width);
	g_desktop = FrameBuffer(static_cast<Color*>(desktop), g_gfx.Height, g_gfx.Width);
	BuildDesktop();

	//Publish the work area (screen minus the taskbar) for clients (GetScreenRect) so apps place and size windows above it, then open the control endpoint.
	RegisterEndpoint(Wm::ScreenEndpoint, ((uint64_t)g_gfx.Width << 32) | (g_gfx.Height - Theme::TaskbarHeight));
	RegisterEndpoint(Wm::ControlEndpoint, 0);

	//Claim the kernel->WM input ring (published as a grant token under "input").
	g_inputRegion = IpcLookupRegion(Wm::InputEndpoint);

	static constexpr uint32_t targetFps = 30;
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
