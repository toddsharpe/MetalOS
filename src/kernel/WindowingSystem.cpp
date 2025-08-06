#include "kernel/WindowingSystem.h"
#include "kernel/Api.h"
#include "Graphics/Draw2D.h"
#include "new.h"
#include "Assert.h"
#include "core_crt/stdint.h"

using namespace Graphics;

uint32_t WindowingSystem::ThreadLoop(void* arg)
{
	return static_cast<WindowingSystem*>(arg)->ThreadLoop();
};

WindowingSystem::WindowingSystem(Graphics::FrameBuffer& display) :
	Enabled(),
	m_display(display),
	m_backBuffer(nullptr, display.Height, display.Width),
	m_windows(),
	m_windowPool(),
	m_messages(),
	m_mousePos(),
	m_prevMouseButtons(),
	m_dragWindow(),
	m_focusWindow(),
	m_postCount(),
	m_dropCount()
{

}

void WindowingSystem::Initialize()
{
	//Initialize back buffer
	m_backBuffer.Buffer = (Color*)KeVirtualAlloc(m_backBuffer.Size());

	//Create thread
	KeCreateThread(&WindowingSystem::ThreadLoop, this, "WindowingSystem::ThreadLoop");

	m_windows.Initialize();
}

UWindow* WindowingSystem::CreateWindow(UThread& owner)
{
	//Allocate window
	UWindow* window = m_windowPool.Allocate(owner);
	Assert(window);
	
	//Add to list
	m_windows.InsertHead(*window);

	//Set it as focus
	m_focusWindow = window;

	return window;
}

bool WindowingSystem::Delete(UWindow* window)
{
	//If window is focused or dragged clear it
	if (m_dragWindow == window)
		m_dragWindow = nullptr;
	if (m_focusWindow == window)
		m_focusWindow = nullptr;

	//Remove from list
	m_windows.Remove(*window);

	//Deallocate
	m_windowPool.Deallocate(window);

	return true;
}

void WindowingSystem::PostMessage(Message& message)
{
	const cpu_flags_t flags = m_messagesLock.Acquire();
	if (!m_messages.Enqueue(message))
		m_dropCount++;
	else
		m_postCount++;
	m_messagesLock.Release(flags);
}

uint32_t WindowingSystem::ThreadLoop()
{
	while (true)
	{
		if (Enabled)
		{
			//Clear screen
			Draw2D::FillScreen(m_backBuffer, Colors::Black);

			//Process current messages
			ProcessMessages();

			//Draw windows back to front
			m_windows.ForEachReverse<WindowingSystem>([](const UWindow& window, WindowingSystem& context)
			{
				if (&window == context.m_focusWindow)
				{
					Draw2D::WriteFrame(context.m_backBuffer, window.GetBounds(), window.Frame.Buffer);
				}
				else
				{
					Draw2D::WriteFrameGrayscale(context.m_backBuffer, window.GetBounds(), window.Frame.Buffer);
				}
			}, *this);

			//Write to graphics device
			memcpy(m_display.Buffer, m_backBuffer.Buffer, m_backBuffer.Size());

			//Request repaints
			//TODO(tsharpe): Only changed regions
			m_windows.ForEach<WindowingSystem&>([](const UWindow& window, WindowingSystem& context)
			{
				Message message = {};
				message.Header.MessageType = MessageType::PaintEvent;
				message.PaintEvent.Region = window.GetBounds();
				window.Thread.Enqueue(message);
			}, *this);
		}

		KeSleepThread(Second / 30);
	}

	return 0;
}

void WindowingSystem::ProcessMessages()
{
	// Drain the queue under a single brief lock window so interrupt context
	// (PostMessage) never spins waiting for us to finish processing a message.
	Message pending[16] = {};
	size_t pendingCount = 0;

	const cpu_flags_t flags = m_messagesLock.Acquire();
	while (!m_messages.IsEmpty())
		pending[pendingCount++] = m_messages.Dequeue();
	m_messagesLock.Release(flags);

	for (size_t i = 0; i < pendingCount; i++)
	{
		const Message& message = pending[i];
		switch (message.Header.MessageType)
		{
		case MessageType::MouseEvent:
			//Save cursor position
			const uint16_t absX = m_display.Width * message.MouseEvent.XPosition / INT16_MAX;
			const uint16_t absY = m_display.Height * message.MouseEvent.YPosition / INT16_MAX;
			const Point2D mousePos = { absX , absY };

			const bool clicked = message.MouseEvent.Buttons.LeftPressed && !m_prevMouseButtons.LeftPressed;

			//Detect click
			if (clicked)
			{
				m_focusWindow = GetWindow(mousePos);
				m_dragWindow = m_focusWindow;

				if (m_focusWindow != nullptr)
				{
					//Remove from list, push on top
					m_windows.Remove(*m_focusWindow);
					m_windows.InsertHead(*m_focusWindow);

				}
			}
			//Detect drag
			else if (message.MouseEvent.Buttons.LeftPressed)
			{
				if (m_dragWindow)
				{
					//Calculate difference between old mouse and new mouse
					const int deltaX = (int)(mousePos.X - m_mousePos.X);
					const int deltaY = (int)(mousePos.Y - m_mousePos.Y);

					//Update window position
					m_dragWindow->Point.X += deltaX;
					m_dragWindow->Point.Y += deltaY;
				}
			}
			else
			{
				m_dragWindow = nullptr;
			}

			//Save previous mouse
			m_mousePos = mousePos;
			m_prevMouseButtons = message.MouseEvent.Buttons;
			break;
		}
		
		if (m_focusWindow)
		{
			m_focusWindow->Thread.Enqueue(message);
		}
	}
}

UWindow* WindowingSystem::GetWindow(const Point2D& point) const
{
	//Printf("Get: X: %d, Y: %d\n", point.X, point.Y);
	UWindow* found = m_windows.First<const Point2D>([](const UWindow& window, const Point2D& point)
	{
		//Printf("--X: %d, Y: %d, W: %d, H:%d\n", window.GetBounds().X, window.GetBounds().Y, window.GetBounds().Width, window.GetBounds().Height);
		const bool r = window.GetBounds().Contains(point);
		//Printf("----R: %d\n", r);
		return r;
	}, point);
	return found;
}
