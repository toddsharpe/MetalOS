#pragma once

#include "UserThread.h"
#include "EfiDisplay.h"
#include <Graphics/Types.h>
#include <Graphics/DynamicFrameBuffer.h>
#include <string>
#include <memory>

//Simple windowing system. Windows are drawn in order, top window is back of m_windows.
//NOTE(tsharpe): Handles are just kernel pointers. This should be fixed.
//NOTE(tsharpe): Enforcing one thread per window would make handle tracking easier
class WindowingSystem
{
private:
	static size_t ThreadLoop(void* arg);

public:
	WindowingSystem(EfiDisplay& display);
	void Initialize();

	//Systemcalls
	HWindow AllocWindow(UserThread& thread, const Graphics::Rectangle& bounds);
	bool PaintWindow(const HWindow handle, const ReadOnlyBuffer& buffer);
	bool MoveWindow(const HWindow handle, const Graphics::Rectangle& bounds);
	bool GetWindowRect(const HWindow handle, Graphics::Rectangle& bounds);

	void PostMessage(Message& message);

	bool ThreadHasWindow(const UserThread& thread) const;
	void FreeWindow(const UserThread& thread);
	void FreeWindows(const UserProcess& proc);

private:
	struct Window
	{
		Window(UserThread& thread) :
			Thread(thread),
			Bounds(),
			FrameBuffer()
		{};

		UserThread& Thread;
		Graphics::Rectangle Bounds;
		Buffer FrameBuffer;
	};

	size_t ThreadLoop();

	std::shared_ptr<Window> GetWindow(const Graphics::Point2D& point) const;
	bool HandleValid(const HWindow handle) const;

	EfiDisplay& m_display;
	DynamicFrameBuffer m_frameBuffer;
	std::unique_ptr<std::list<std::shared_ptr<Window>>> m_windows;

	//Mouse
	Graphics::Point2D m_mousePos;
	MouseButtonState m_prevMouseButtons;

	//Drag
	std::shared_ptr<Window> m_dragWindow;
	std::shared_ptr<Window> m_focusWindow;
};

