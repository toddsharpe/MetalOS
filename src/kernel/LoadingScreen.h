#pragma once

#include "UI/Window.h"
#include "UI/Label.h"
#include "StringPrinter.h"

//This method is called early on during boot (before a proper heap) and therefore
//writes to the screen directly (versus buffering in UI constructs like window or control).
class LoadingScreen : public StringPrinter
{
public:
	LoadingScreen(Graphics::FrameBuffer& frameBuffer);

	void Initialize();
	virtual void Write(const std::string& string) override;

private:
	static constexpr Graphics::Color Foreground = Graphics::Colors::White;
	static constexpr Graphics::Color Background = Graphics::Colors::Black;
	static constexpr Graphics::Color Border = Graphics::Colors::Blue;

	Graphics::FrameBuffer& m_frameBuffer;
	size_t yPos;
};

