#include "kernel/LoadingScreen.h"

LoadingScreen::LoadingScreen(Graphics::FrameBuffer& frameBuffer) :
	m_frameBuffer(frameBuffer),
	yPos(25)
{

}

void LoadingScreen::Initialize()
{
	//Clear frame and draw border
	Graphics::Draw2D::FillScreen(m_frameBuffer, Background);

	Graphics::Draw2D::DrawFrameBorder(m_frameBuffer, Border, 3);
	Graphics::Draw2D::DrawText(m_frameBuffer, { 9, 9 }, "MetalOS", Border);
	Graphics::Draw2D::DrawRectangle(m_frameBuffer, Border, { 0, 20, m_frameBuffer.Width, 3 });
}

void LoadingScreen::Printf(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	
	char buffer[1024];
	int retval = vsprintf(buffer, format, args);
	buffer[retval] = '\0';
	Write({buffer, static_cast<size_t>(retval)});

	va_end(args);
}

void LoadingScreen::Write(const CString& string)
{
	Graphics::Point2D point = Graphics::Draw2D::DrawText(m_frameBuffer, { 5, static_cast<int32_t>(yPos) }, string, Graphics::Colors::White);
	yPos = static_cast<size_t>(point.Y);
}
