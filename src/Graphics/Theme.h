#pragma once

#include "Graphics/Types.h"

//The Forge look, shared by the window manager and the apps: gunmetal chrome, bone text, and one ember accent kept for focus and action
namespace Graphics::Theme
{
	constexpr Color Rgb(const uint32_t rgb)
	{
		return Color((uint8_t)rgb, (uint8_t)(rgb >> 8), (uint8_t)(rgb >> 16));
	}

	//Surfaces
	constexpr Color Void = Rgb(0x0B0B0D);
	constexpr Color Iron = Rgb(0x131417);
	constexpr Color IronDeep = Rgb(0x0E0F11);
	constexpr Color Well = Rgb(0x08090A);
	constexpr Color GunmetalTop = Rgb(0x2E3036);
	constexpr Color GunmetalBottom = Rgb(0x1F2024);
	constexpr Color GunmetalDimTop = Rgb(0x25272B);
	constexpr Color GunmetalDimBottom = Rgb(0x1B1C1F);

	//Edges
	constexpr Color Outline = Rgb(0x050506);
	constexpr Color BevelLight = Rgb(0x3D4047);
	constexpr Color BevelDark = Rgb(0x0C0D0F);
	constexpr Color Divider = Rgb(0x2B2D32);
	constexpr Color AccentOff = Rgb(0x26282C);

	//Text
	constexpr Color Bone = Rgb(0xE9E3D6);
	constexpr Color Text = Rgb(0xDDD6C8);
	constexpr Color TextDim = Rgb(0x858178);
	constexpr Color Steel = Rgb(0x8A9099);
	constexpr Color Hint = Rgb(0x7C818A);
	constexpr Color Muted = Rgb(0x6F747C);

	//Accent and status
	constexpr Color Ember = Rgb(0xFF6A1F);
	constexpr Color EmberText = Rgb(0xFF7A2E);
	constexpr Color Molten = Rgb(0xFF9A3C);
	constexpr Color Hot = Rgb(0xFFCF5A);
	constexpr Color Blood = Rgb(0xB3200F);
	constexpr Color ErrorText = Rgb(0xEF5A3C);
	constexpr Color Selection = Rgb(0x2A1710);
	constexpr Color SelectionEdge = Rgb(0x5A2A14);

	//Raised controls: face gradient, then the light and dark bevel
	struct Face
	{
		Color Top;
		Color Bottom;
		Color Light;
		Color Dark;
		Color Label;
	};

	constexpr Face Raised = { Rgb(0x3A3D44), Rgb(0x2B2D32), Rgb(0x50545C), Rgb(0x17181B), Bone };
	constexpr Face RaisedHover = { Rgb(0x4A4E56), Rgb(0x383B41), Rgb(0x5C6068), Rgb(0x17181B), Rgb(0xFFFFFF) };
	constexpr Face Sunken = { Rgb(0x1C1D20), Rgb(0x1C1D20), Rgb(0x070708), Rgb(0x3A3D44), Bone };
	constexpr Face Operator = { Rgb(0x26282C), Rgb(0x1C1D20), Rgb(0x3A3D44), Rgb(0x111214), EmberText };
	constexpr Face OperatorHover = { Rgb(0x2E3035), Rgb(0x222327), Rgb(0x44474E), Rgb(0x111214), Molten };
	constexpr Face OperatorSunken = { Rgb(0x18191C), Rgb(0x18191C), Rgb(0x070708), Rgb(0x34373D), EmberText };
	constexpr Face Primary = { Rgb(0xF0662A), Rgb(0xC2410F), Rgb(0xFF9A5C), Rgb(0x7A2408), Rgb(0x1A0B05) };
	constexpr Face PrimaryHover = { Rgb(0xFF7838), Rgb(0xD04C14), Rgb(0xFFAE78), Rgb(0x7A2408), Rgb(0x1A0B05) };
	constexpr Face PrimarySunken = { Rgb(0xA8380C), Rgb(0xC84A14), Rgb(0x6A1F06), Rgb(0xE0703A), Rgb(0x1A0B05) };
	constexpr Face Danger = { Rgb(0x6A1D13), Rgb(0x4E140C), Rgb(0x8E3224), Rgb(0x2A0904), Rgb(0xF3DCD2) };
	constexpr Face DangerLift = { Rgb(0x7A2418), Rgb(0x5A170E), Rgb(0x9E3A2A), Rgb(0x2A0904), Rgb(0xFFFFFF) };
	constexpr Face DangerHover = { Rgb(0xC22A14), Rgb(0x8F1A0E), Rgb(0xE0503A), Rgb(0x4A0C05), Rgb(0xFFFFFF) };
	constexpr Face DangerSunken = { Rgb(0x5A1009), Rgb(0x5A1009), Rgb(0x2A0604), Rgb(0x8F1A0E), Bone };

	//Text is the 8x8 system font scaled up; UI text is 2x
	constexpr uint32_t TextScale = 2;
	constexpr int32_t Glyph = 8 * TextScale;

	//Window frame around an app's client area: a 1px outline and 1px bevel, with the title bar on top
	constexpr int32_t FrameEdge = 2;
	constexpr int32_t TitleHeight = 28;
	constexpr int32_t AccentHeight = 2;
	constexpr int32_t FrameTop = FrameEdge + TitleHeight + AccentHeight;

	//Title-bar buttons, laid out from the right edge
	constexpr int32_t ButtonWidth = 26;
	constexpr int32_t ButtonHeight = 22;
	constexpr int32_t ButtonGap = 4;
	constexpr int32_t ButtonMargin = 6;

	constexpr int32_t TaskbarHeight = 40;
}
