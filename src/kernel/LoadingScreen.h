#pragma once

#include "core_crt/stdarg.h"
#include "core_crt/stdio.h"
#include "Lib/System.h"
#include "Lib/String.h"
#include "Graphics/Types.h"
#include "Graphics/Draw2D.h"

class LoadingScreen
{
public:
	LoadingScreen(Graphics::FrameBuffer& frameBuffer);

	void Initialize();

	void Printf(const char* format, ...);
	void Write(const CString& string);

private:
	static constexpr Graphics::Color Foreground = Graphics::Colors::White;
	static constexpr Graphics::Color Background = Graphics::Colors::Black;
	static constexpr Graphics::Color Border = Graphics::Colors::Blue;

	Graphics::FrameBuffer& m_frameBuffer;
	size_t yPos;
};

