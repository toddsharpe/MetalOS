#pragma once

#include "Lib/StaticVector.h"
#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"
#include "UI/Control.h"
#include "user/MetalOS.h"
#include "Assert.h"

class GUI;

typedef bool (*MessageCallback)(GUI &window, Message &message);

class GUI
{
public:
	GUI(const CString &title, const Graphics::Rectangle &rect, const MessageCallback callback) :
		Frame(nullptr, rect.Height, rect.Width),
		Background(Graphics::Colors::White),
		Children(),
		Title(title),
		m_callback(callback),
		m_point({rect.X, rect.Y}),
		m_prevMouseButtons(),
		m_pressed(),
		m_handle()
	{
	}

	void Initialize()
	{
		// Allocate
		Graphics::Rectangle rect = {m_point.X, m_point.Y, Frame.Width, Frame.Height};
		AssertSuccess(AllocWindow(&m_handle, &rect, Title.c_str()));

		// Draw directly into the WM-provided shared surface (zero-copy)
		Frame.Buffer = (Graphics::Color *)GetWindowSurface(m_handle);
		Assert(Frame.Buffer);
	}

	void Run()
	{
		Message message = {};
		while (GetMessage(&message) == SyscallResult::Success)
		{
			if (message.Header.MessageType == MessageType::CloseEvent)
				break; //WM asked us to close; exit the message loop so main() can return
			if (!m_callback(*this, message))
				DefaultCallback(message);
		}
	}

	void DefaultCallback(Message &message)
	{
		switch (message.Header.MessageType)
		{
		case MessageType::PaintEvent:
			//Draw straight into the WM-provided shared surface; the WM composites it.
			Draw();
			break;

		case MessageType::MouseEvent:
			OnMouse(message.MouseEvent);
			break;
		}
	}

	Graphics::FrameBuffer Frame;
	Graphics::Color Background;
	StaticVector<UI::Control*, 32> Children;
	StaticString<32> Title;

private:
	static constexpr Graphics::Color BorderColor = Graphics::Colors::Blue;

	void Draw()
	{
		//Clear frame
		Graphics::Draw2D::FillScreen(Frame, Background);

		//Draw border
		Graphics::Draw2D::DrawFrameBorder(Frame, BorderColor, 3);
		Graphics::Draw2D::DrawText(Frame, { 9, 9 }, Title, BorderColor);
		Graphics::Draw2D::DrawRectangle(Frame, BorderColor, { 0, 20, Frame.Width, 3 });

		//Draw children
		for (const auto& child : Children)
		{
			child->Draw(Frame);
		}
	}

	//The child drawn last under a point in window coordinates, since it is the one on top
	UI::Control* HitTest(const Graphics::Point2D& point) const
	{
		for (size_t i = Children.Count(); i > 0; i--)
		{
			UI::Control* const child = Children[i - 1];
			if (child->Bounds.Contains(point))
				return child;
		}

		return nullptr;
	}

	//A click is the left button going down and coming back up over the same child
	void OnMouse(const MouseEvent& mouse)
	{
		const Graphics::Point2D point = { mouse.XPosition, mouse.YPosition };
		if (mouse.Buttons.LeftPressed && !m_prevMouseButtons.LeftPressed)
		{
			m_pressed = HitTest(point);
		}
		else if (!mouse.Buttons.LeftPressed && m_prevMouseButtons.LeftPressed)
		{
			if (m_pressed && HitTest(point) == m_pressed)
				m_pressed->OnClick();
			m_pressed = nullptr;
		}

		m_prevMouseButtons = mouse.Buttons;
	}

	MessageCallback m_callback;

	Graphics::Point2D m_point;
	MouseButtonState m_prevMouseButtons;
	UI::Control* m_pressed;
	HWindow m_handle;
};
