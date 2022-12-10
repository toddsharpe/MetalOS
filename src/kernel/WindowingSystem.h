#pragma once

#include "UserThread.h"
#include "EfiDisplay.h"
#include <Graphics/Types.h>
#include <Graphics/DynamicFrameBuffer.h>
#include <string>
#include <memory>

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

	void PostMessage(Message* message);

	bool ThreadHasWindow(const UserThread& thread) const;
	void FreeWindow(const UserThread& thread);

private:
	struct Window
	{
		Window(UserThread& thread) :
			Thread(thread),
			Bounds(),
			FrameBuffer()
		{};
		
		bool operator==(const Window& rhs) const
		{
			return this == &rhs;
		}

		UserThread& Thread;
		Graphics::Rectangle Bounds;
		//size_t z_index;
		Buffer FrameBuffer;
	};

	size_t ThreadLoop();

	Window* GetWindow(const Graphics::Point2D& point) const;
	bool HandleValid(const HWindow handle) const;

	EfiDisplay& m_display;
	DynamicFrameBuffer m_frameBuffer;
	std::unique_ptr<std::list<Window>> m_windows;

	//Mouse
	Graphics::Point2D m_mousePos;
	MouseButtonState m_prevMouseButtons;

	//Drag
	Window* m_dragWindow;
	Window* m_focusWindow;
};

