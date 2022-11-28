#include "LoadingScreen.h"

using namespace Graphics;

LoadingScreen::LoadingScreen(Graphics::FrameBuffer& frameBuffer) :
	StringPrinter(),
	m_frameBuffer(frameBuffer),
	yPos(25)
{

}

void LoadingScreen::Initialize()
{
	//Clear frame and draw border
	m_frameBuffer.FillScreen(Background);

	m_frameBuffer.DrawFrameBorder(Border, 3);
	m_frameBuffer.DrawText({ 9, 9 }, "MetalOS", Border);
	m_frameBuffer.DrawRectangle(Border, { 0, 20, m_frameBuffer.GetWidth(), 3 });
}

void LoadingScreen::Write(const std::string& string)
{
	Point2D point = m_frameBuffer.DrawText({ 5, yPos }, string, Colors::White);
	yPos = point.Y;
}
