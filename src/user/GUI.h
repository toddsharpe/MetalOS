#pragma once

#include "Graphics/Types.h"
#include "UI/Window.h"
#include "user/MetalOS.h"
#include "Assert.h"

class GUI;

typedef bool (*MessageCallback)(GUI &window, Message &message);

class GUI : public UI::Window
{
public:
	GUI(const CString &title, const Graphics::Rectangle &rect, UI::WindowStyle &style, const MessageCallback &callback) : Window(title, style),
																														  Frame(nullptr, rect.Height, rect.Width),
																														  m_callback(callback),
																														  m_point({rect.X, rect.Y}),
																														  m_prevMouseButtons(),
																														  m_handle()
	{
	}

	void Initialize()
	{
		// Allocate
		Graphics::Rectangle rect = {m_point.X, m_point.Y, Frame.Width, Frame.Height};
		AssertSuccess(AllocWindow(&m_handle, &rect));

		Frame.Buffer = (Graphics::Color *)VirtualAlloc(nullptr, Frame.Size());
		Assert(Frame.Buffer);
	}

	void Run()
	{
		Message message = {};
		while (GetMessage(&message) == SyscallResult::Success)
		{
			if (!m_callback(*this, message))
				DefaultCallback(message);
		}
	}

	void DefaultCallback(Message &message)
	{
		switch (message.Header.MessageType)
		{
		case MessageType::PaintEvent:
		{
			Draw(Frame);
			Buffer buff((uint8_t *)Frame.Buffer, Frame.Size());
			AssertSuccess(PaintWindow(m_handle, &buff));
		}
		break;

		case MessageType::MouseEvent:
			m_prevMouseButtons = message.MouseEvent.Buttons;
			break;
		}
	}

	void GetRectangle(Graphics::Rectangle &frame)
	{
		// GetWindowRect?
		frame = Frame.GetBounds();
	}

	void Move(const Graphics::Rectangle &frame)
	{
		// Implement these later
		Assert(frame.Height == Frame.Height);
		Assert(frame.Width == Frame.Width);

		MoveWindow(m_handle, &frame);
	}

	HWindow GetHandle() const
	{
		return m_handle;
	}

	Graphics::FrameBuffer Frame;

private:
	const MessageCallback &m_callback;

	Graphics::Point2D m_point;
	MouseButtonState m_prevMouseButtons;
	HWindow m_handle;
};
