#include "WindowingSystem.h"
#include "Scheduler.h"
#include "Kernel.h"
#include "Assert.h"
#include <limits>

size_t WindowingSystem::ThreadLoop(void* arg)
{
	return static_cast<WindowingSystem*>(arg)->ThreadLoop();
};

WindowingSystem::WindowingSystem(Display* display) :
	m_windows(),
	m_display(display),
	m_mousePos(),
	m_prevMouseButtons(),
	m_dragWindow()
{

}

void WindowingSystem::Initialize()
{
	//Create thread
	KThread* thread = kernel.KeCreateThread(&WindowingSystem::ThreadLoop, this);
	thread->SetName("WindowingSystem::ThreadLoop");
}

size_t WindowingSystem::ThreadLoop() const
{
	while (true)
	{
		//Clear frame - TODO: double buffer, refactor graphics
		m_display->ColorScreen(Colors::Black);
		
		//Draw windows
		for (auto it = m_windows.begin(); it != m_windows.end(); it++)
		{
			const Window* window = *it;
			if (std::next(it) != m_windows.end())
			{
				m_display->WriteFrameGrayscale(window->Bounds, window->FrameBuffer);
			}
			else
			{
				//Top window gets color
				m_display->WriteFrame(window->Bounds, window->FrameBuffer);
			}
		}

		//Draw cursor
		m_display->ColorRectangle(Colors::Blue, { m_mousePos.X, m_mousePos.Y, 3, 3 });

		//Request repaints
		//TODO: only changed regions?
		for (const auto& window : m_windows)
		{
			Assert(window->Thread);
			Message message;
			message.Header.MessageType = MessageType::PaintEvent;
			message.PaintEvent.Region = window->Bounds;
			window->Thread->EnqueueMessage(&message);
		}

		kernel.KeSleepThread(SECOND / 30);
	}
}

HWindow WindowingSystem::AllocWindow(UserThread* thread, const Rectangle& bounds)
{
	//Allocate framebuffer on page boundaries
	//Map into process directly one day?
	const size_t bytes = bounds.Height * bounds.Width * sizeof(Color);
	const size_t count = SIZE_TO_PAGES(bytes);
	void* buffer = kernel.AllocateWindows(count);
	
	Window* window = new Window();
	window->Bounds = bounds;
	window->FrameBuffer = { buffer, bytes };
	window->Thread = thread;

	m_windows.push_back(window);
	return (HWindow)window;
}

bool WindowingSystem::PaintWindow(HWindow handle, const ReadOnlyBuffer& buffer)
{
	if (!HandleValid(handle))
		return false;
	
	Window* window = static_cast<Window*>(handle);
	if (window->FrameBuffer.Length != buffer.Length)
		return false;

	memcpy(window->FrameBuffer.Data, buffer.Data, buffer.Length);
	return true;
}

bool WindowingSystem::MoveWindow(HWindow handle, const Rectangle& bounds)
{
	if (!HandleValid(handle))
		return false;
	
	Window* window = static_cast<Window*>(handle);
	
	//We can't deallocate the old buffer yet :D
	Assert(bounds.Height == window->Bounds.Height);
	Assert(bounds.Width == window->Bounds.Width);

	window->Bounds = bounds;

	return true;
}

bool WindowingSystem::GetWindowRect(HWindow handle, Rectangle& bounds)
{
	if (!HandleValid(handle))
		return false;
	
	Window* window = static_cast<Window*>(handle);
	bounds = window->Bounds;
	return true;
}

bool WindowingSystem::ThreadHasWindow(const size_t threadId) const
{
	for (const auto& window : m_windows)
	{
		if (window->Thread->GetId() == threadId)
			return true;
	}

	return false;
}

void WindowingSystem::PostMessage(Message* message)
{
	switch (message->Header.MessageType)
	{
	case MessageType::MouseEvent:
		//Save cursor position
		constexpr uint16_t maxX = std::numeric_limits<int16_t>::max();
		constexpr uint16_t maxY = std::numeric_limits<int16_t>::max();
		uint16_t absX = m_display->GetWidth() * message->MouseEvent.XPosition / maxX;
		uint16_t absY= m_display->GetHeight() * message->MouseEvent.YPosition / maxX;

		//Draw cursor
		Point2D mousePos = {};
		mousePos.X = absX;
		mousePos.Y = absY;

		//Detect click
		if (message->MouseEvent.Buttons.LeftPressed && !m_prevMouseButtons.LeftPressed)
		{
			Window* focus = GetWindow(mousePos);
			if (focus != nullptr)
			{
				//Remove from list, push on top
				m_windows.remove(focus);
				m_windows.push_back(focus);
			}
		}

		//Detect drag
		if (message->MouseEvent.Buttons.LeftPressed)
		{
			if (m_dragWindow == nullptr)
			{
				m_dragWindow = GetWindow(mousePos);
			}

			//Verify we have a window to move
			if (m_dragWindow != nullptr)
			{
				//Calculate difference between old mouse and new mouse
				int deltaX = (int)mousePos.X - m_mousePos.X;
				int deltaY = (int)mousePos.Y - m_mousePos.Y;

				//Update window position
				m_dragWindow->Bounds.X += deltaX;
				m_dragWindow->Bounds.Y += deltaY;
			}
		}
		else
		{
			m_dragWindow = nullptr;
		}

		//Update old mouse position
		m_mousePos = mousePos;
		break;
	}
	
	if (m_windows.size() == 0)
		return;

	Window* top = m_windows.back();
	Assert(top->Thread);
	top->Thread->EnqueueMessage(message);
}

WindowingSystem::Window* WindowingSystem::GetWindow(const Point2D& point) const
{
	//Loop through in reverse, finding first window inside bounds
	auto it = m_windows.rbegin();
	while (it != m_windows.rend())
	{
		Window* window = *it;
		if (window->Bounds.Contains(point))
			return window;

		it++;
	}
	return nullptr;
}

bool WindowingSystem::HandleValid(const HWindow handle) const
{
	auto it = m_windows.rbegin();
	while (it != m_windows.rend())
	{
		Window* window = *it;
		if (window == handle)
			return true;

		it++;
	}
	return false;
}
