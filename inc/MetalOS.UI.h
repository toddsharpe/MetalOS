#pragma once

#include <MetalOS.h>
#include <Debug.h>
#include <vector>

/*********************************/
/*MetalOS UI Public Facing Header*/
/*********************************/

//System Call interface is kept as C-compatible despite this header not being C-friendly
//This is to allow potential C# interop to be clean

#if !defined(EXPORT)
#define UICALL(type) extern "C" __declspec(dllimport) type
#else
#define UICALL(type) extern "C" __declspec(dllexport) type
#endif

class Window;

typedef bool(*MessageCallback)(Window& window, Message& message);

struct WindowStyle
{
	bool IsBordered;

};

constexpr static WindowStyle Default = { true };


//Process/Thread
//UICALL(SystemCallResult) CreateWindow(const char* name, const WindowStyle &style);

// Constant: font8x8_basic
// Contains an 8x8 font map for unicode points U+0000 - U+007F (basic latin)
#pragma warning(push)
#pragma warning(disable: 4838)
#pragma warning(disable: 4309)
const char font8x8_basic[128][8] = {
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0000 (nul)
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0001
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0002
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0003
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0004
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0005
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0006
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0007
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0008
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0009
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000A
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000B
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000C
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000D
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000E
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+000F
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0010
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0011
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0012
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0013
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0014
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0015
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0016
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0017
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0018
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0019
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001A
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001B
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001C
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001D
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001E
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+001F
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0020 (space)
	{ 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00},   // U+0021 (!)
	{ 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0022 (")
	{ 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00},   // U+0023 (#)
	{ 0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00},   // U+0024 ($)
	{ 0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00},   // U+0025 (%)
	{ 0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00},   // U+0026 (&)
	{ 0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0027 (')
	{ 0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00},   // U+0028 (()
	{ 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00},   // U+0029 ())
	{ 0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},   // U+002A (*)
	{ 0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00},   // U+002B (+)
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+002C (,)
	{ 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00},   // U+002D (-)
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+002E (.)
	{ 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00},   // U+002F (/)
	{ 0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00},   // U+0030 (0)
	{ 0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00},   // U+0031 (1)
	{ 0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00},   // U+0032 (2)
	{ 0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00},   // U+0033 (3)
	{ 0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00},   // U+0034 (4)
	{ 0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00},   // U+0035 (5)
	{ 0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00},   // U+0036 (6)
	{ 0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00},   // U+0037 (7)
	{ 0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00},   // U+0038 (8)
	{ 0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00},   // U+0039 (9)
	{ 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00},   // U+003A (:)
	{ 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x06},   // U+003B (;)
	{ 0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00},   // U+003C (<)
	{ 0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00},   // U+003D (=)
	{ 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00},   // U+003E (>)
	{ 0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00},   // U+003F (?)
	{ 0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00},   // U+0040 (@)
	{ 0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00},   // U+0041 (A)
	{ 0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00},   // U+0042 (B)
	{ 0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00},   // U+0043 (C)
	{ 0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00},   // U+0044 (D)
	{ 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00},   // U+0045 (E)
	{ 0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00},   // U+0046 (F)
	{ 0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00},   // U+0047 (G)
	{ 0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00},   // U+0048 (H)
	{ 0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0049 (I)
	{ 0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00},   // U+004A (J)
	{ 0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00},   // U+004B (K)
	{ 0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00},   // U+004C (L)
	{ 0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00},   // U+004D (M)
	{ 0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00},   // U+004E (N)
	{ 0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00},   // U+004F (O)
	{ 0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00},   // U+0050 (P)
	{ 0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00},   // U+0051 (Q)
	{ 0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00},   // U+0052 (R)
	{ 0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00},   // U+0053 (S)
	{ 0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0054 (T)
	{ 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00},   // U+0055 (U)
	{ 0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0056 (V)
	{ 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},   // U+0057 (W)
	{ 0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00},   // U+0058 (X)
	{ 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00},   // U+0059 (Y)
	{ 0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00},   // U+005A (Z)
	{ 0x1E, 0x06, 0x06, 0x06, 0x06, 0x06, 0x1E, 0x00},   // U+005B ([)
	{ 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},   // U+005C (\)
	{ 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E, 0x00},   // U+005D (])
	{ 0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00},   // U+005E (^)
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},   // U+005F (_)
	{ 0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+0060 (`)
	{ 0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00},   // U+0061 (a)
	{ 0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00},   // U+0062 (b)
	{ 0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00},   // U+0063 (c)
	{ 0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00},   // U+0064 (d)
	{ 0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00},   // U+0065 (e)
	{ 0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00},   // U+0066 (f)
	{ 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0067 (g)
	{ 0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00},   // U+0068 (h)
	{ 0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+0069 (i)
	{ 0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E},   // U+006A (j)
	{ 0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00},   // U+006B (k)
	{ 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00},   // U+006C (l)
	{ 0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00},   // U+006D (m)
	{ 0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00},   // U+006E (n)
	{ 0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00},   // U+006F (o)
	{ 0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F},   // U+0070 (p)
	{ 0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78},   // U+0071 (q)
	{ 0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00},   // U+0072 (r)
	{ 0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00},   // U+0073 (s)
	{ 0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00},   // U+0074 (t)
	{ 0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00},   // U+0075 (u)
	{ 0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00},   // U+0076 (v)
	{ 0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00},   // U+0077 (w)
	{ 0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00},   // U+0078 (x)
	{ 0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F},   // U+0079 (y)
	{ 0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00},   // U+007A (z)
	{ 0x38, 0x0C, 0x0C, 0x07, 0x0C, 0x0C, 0x38, 0x00},   // U+007B ({)
	{ 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00},   // U+007C (|)
	{ 0x07, 0x0C, 0x0C, 0x38, 0x0C, 0x0C, 0x07, 0x00},   // U+007D (})
	{ 0x6E, 0x3B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},   // U+007E (~)
	{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}    // U+007F
};
#pragma warning(pop)

class Font
{
public:
	const char* GetCharacterMap(const char c) const
	{
		//Assert(c <= 0x7F);

		return (*this->FontMap)[c];
	}

private:
	const char(*FontMap)[128][8] = &font8x8_basic;
};

class Frame
{
public:
	Frame(const Rectangle& rect) :
		m_font(),
		m_rect(rect),
		m_frameBuffer()
	{

	}

	void Initialize()
	{
		m_frameBuffer.Length = m_rect.Height * m_rect.Width * sizeof(Color);
		m_frameBuffer.Data = VirtualAlloc(nullptr, m_frameBuffer.Length, MemoryAllocationType::CommitReserve, MemoryProtection::PageReadWrite);
	}

	void FillFrame(const Color color)
	{
		setpixels(m_frameBuffer.Data, color, m_rect.Height * m_rect.Width);
	}

//#define MAX(x, y) (x > y ? x : y)
#define MIN(x, y) (x < y ? x : y)
#define ABS(N) ((N<0)?(-N):(N))


	void DrawRectangle(const Color color, const Rectangle& region)
	{
		for (size_t y = 0; y < region.Height; y++)
		{
			if (y + region.Y >= m_rect.Height)
				break;

			//Write row
			uint32_t* dst = (uint32_t*)m_frameBuffer.Data + (region.Y + y) * m_rect.Width + region.X;
			size_t count = MIN(region.Width, m_rect.Width - region.X);
			setpixels(dst, color, count);
		}
	}

	//TODO: edge cases
	void DrawFrameBorder(const Color color, const size_t thickness)
	{
		//Top
		setpixels(m_frameBuffer.Data, color, m_rect.Width * thickness);

		//Bottom
		Color* dst = (Color*)m_frameBuffer.Data + (m_rect.Height - thickness) * m_rect.Width;
		setpixels(dst, color, m_rect.Width * thickness);

		//Sides
		for (size_t y = thickness; y < m_rect.Height - thickness; y++)
		{
			Color* row = (Color*)m_frameBuffer.Data + y * m_rect.Width;

			//Left side
			setpixels(row, color, thickness);

			//Right side
			setpixels(row + (m_rect.Width - thickness), color, thickness);
		}
	}

	void DrawBorder(const Color color, const Rectangle& rect, const size_t thickness)
	{
		//Top
		DrawRectangle(color, { rect.X, rect.Y, rect.Width, thickness });

		//Left
		DrawRectangle(color, { rect.X, rect.Y, thickness, rect.Height });

		//Right
		DrawRectangle(color, { rect.X + rect.Width, rect.Y, thickness, rect.Height });

		//Bottom
		DrawRectangle(color, { rect.X, rect.Y + rect.Height, rect.Width, thickness });
	}

	//void DrawTitle

	void DrawText(Point2D position, const char* text)
	{
		while (*text != '\0')
		{
			if (*text == '\n')
			{
				position.Y = (position.Y += PixelScale);
				position.X = 0;
			}
			else if (*text != '\r')
			{
				DrawCharacter(position, *text);
				position.X = (position.X += PixelScale);
			}

			text++;
		}
	}

	void DrawCharacter(Point2D position, const char c)
	{
		const char* map = m_font.GetCharacterMap(c);
		uint8_t size = 8; // TODO: get from font

		for (size_t y = 0; y < size; y++)
		{
			//Line is reversed
			char line = map[y];

			size_t x = 0;
			unsigned char mask = 0x80;
			while (mask > 0)
			{
				if ((line & mask) != 0)
				{
					Rectangle rect;
					rect.X = (position.X + (8 - x - 1)) * FontScale;
					rect.Y = (position.Y + y) * FontScale;
					rect.Width = FontScale;
					rect.Height = FontScale;
					DrawRectangle(Colors::Red, rect);
				}

				mask = mask >> 1;
				x++;
			}
		}
	}

	ReadOnlyBuffer GetRef()
	{
		return m_frameBuffer.GetRef();
	}

private:
	static constexpr size_t PixelScale = 10;
	static constexpr size_t FontScale = 1;

	void* setpixels(void* _Dst, Color _Val, size_t _Size)
	{
		Color* dest_32 = (Color*)_Dst;
		for (size_t i = 0; i < _Size; i++)
		{
			*dest_32 = (Color)_Val;
			dest_32++;
		}

		return _Dst;
	}

	const Font m_font;
	const Rectangle& m_rect;
	Buffer m_frameBuffer;
	//TODO: maybe have a member for the readonly buffer?
};

class Control
{
public:
	Control() :
		Foreground(),
		Background()
	{ }
	virtual void Draw(Frame& frame) = 0;

	Color Foreground;
	Color Background;
};

class Button;
typedef void(*ButtonClick)(Button& button);

class Button : public Control
{
public:
	Button(const char* text, const Rectangle& rect) :
		Control(),
		m_text(text),
		m_rect(rect),
		Click()
	{

	}

	virtual void Draw(Frame& frame) override
	{
		frame.DrawBorder(Foreground, m_rect, 3);
		frame.DrawText({ m_rect.X + 5, m_rect.Y + 5 }, m_text);
	}

	ButtonClick Click;

private:
	const char* m_text;
	const Rectangle& m_rect;
};

class Window
{
public:
	Window(const char* name, const Rectangle& rect, WindowStyle& style, const MessageCallback& callback) :
		m_title(name),
		m_rect(rect),
		m_style(style),
		Children(),
		m_callback(callback),
		m_frame(rect),
		m_handle()
	{

	}

	void Initialize()
	{
		SystemCallResult r = AllocWindow(m_handle, m_rect);
		m_frame.Initialize();
	}

	void Draw()
	{
		m_frame.FillFrame(Colors::White);

		if (m_style.IsBordered)
		{
			DrawBorder();
		}

		for (const auto& child : Children)
		{
			child->Draw(m_frame);
		}
	}

	void DrawBorder()
	{
		m_frame.DrawFrameBorder(Colors::Blue, 3);
		m_frame.DrawText({ 9, 9 }, m_title);
		m_frame.DrawRectangle(Colors::Blue, { 0, 20, m_rect.Width, 3 });
	}

	void Run()
	{
		Message message = {};
		while (GetMessage(message) == SystemCallResult::Success)
		{
			if (!m_callback(*this, message))
				DefaultCallback(message);
		}
	}

	void DefaultCallback(Message& message)
	{
		switch (message.Header.MessageType)
		{
		case MessageType::PaintEvent:
			Draw();
			PaintWindow(m_handle, m_frame.GetRef());
			break;
		case MessageType::MouseEvent:
			//todo: clicked?

			break;
		}
	}

	void Move(const Rectangle& frame)
	{
		//Implement these later
		Assert(frame.Height == m_rect.Height);
		Assert(frame.Width == m_rect.Width);

		MoveWindow(m_handle, frame);
		this->m_rect = frame;
	}

	void GetRectangle(Rectangle& frame)
	{
		//GetWindowRect?
		frame = m_rect;
	}

	void Paint(const ReadOnlyBuffer& buffer)
	{
		PaintWindow(m_handle, buffer);
	}

	std::vector<Control*> Children;

private:
	Frame m_frame;
	Rectangle m_rect;//TODO: keep? or use GetWindowRect?
	WindowStyle m_style;

	const char* m_title;
	const MessageCallback& m_callback;
	HWindow m_handle;
};
