#pragma once

#include "Graphics/Types.h"
#include "Graphics/Theme.h"

//8x8 app icons and 5x5 title-bar glyphs, drawn at 2x or 4x so they sit on the same pixel grid as the system font
namespace Graphics::Icons
{
	constexpr PixelGlyph Terminal =
	{
		8, 8,
		{
			"aaaaaaaa",
			"a......a",
			"a.b....a",
			"a..b...a",
			"a.b.bb.a",
			"a......a",
			"aaaaaaaa",
			"........",
		},
		{ Theme::Steel, Theme::Ember, Theme::Bone },
	};

	constexpr PixelGlyph Calc =
	{
		8, 8,
		{
			".aaaaaa.",
			".abbbba.",
			".abbbba.",
			".a....a.",
			".ac.c.a.",
			".ac.c.a.",
			".a....a.",
			".aaaaaa.",
		},
		{ Theme::Steel, Theme::Ember, Theme::Rgb(0xD9D2C3) },
	};

	constexpr PixelGlyph Flame =
	{
		8, 8,
		{
			"...a....",
			"...aa...",
			"..aaa...",
			"..aaaa..",
			".aabaaa.",
			".aabbaa.",
			".aabbaa.",
			"..aaaa..",
		},
		{ Theme::Ember, Theme::Hot, Theme::Hot },
	};

	constexpr PixelGlyph Skull =
	{
		8, 8,
		{
			".aaaaaa.",
			"aaaaaaaa",
			"abbaabba",
			"abbaabba",
			"aaaaaaaa",
			".aa..aa.",
			".aaaaaa.",
			".a.aa.a.",
		},
		{ Theme::Rgb(0xD9D2C3), Theme::Ember, Theme::Ember },
	};

	constexpr PixelGlyph Window =
	{
		8, 8,
		{
			"aaaaaaaa",
			"abbbbbba",
			"aaaaaaaa",
			"a......a",
			"a......a",
			"a......a",
			"aaaaaaaa",
			"........",
		},
		{ Theme::Steel, Theme::Ember, Theme::Ember },
	};

	constexpr PixelGlyph Network =
	{
		8, 8,
		{
			"........",
			".....a..",
			".....a..",
			"...a.a..",
			"...a.a..",
			".a.a.a..",
			".a.a.a..",
			"........",
		},
		{ Theme::Ember, Theme::Ember, Theme::Ember },
	};

	constexpr PixelGlyph Minimize =
	{
		5, 5,
		{ ".....", ".....", ".....", ".....", "aaaaa" },
		{ Theme::Bone, Theme::Bone, Theme::Bone },
	};

	constexpr PixelGlyph Maximize =
	{
		5, 5,
		{ "aaaaa", "aaaaa", "a...a", "a...a", "aaaaa" },
		{ Theme::Bone, Theme::Bone, Theme::Bone },
	};

	constexpr PixelGlyph Restore =
	{
		5, 5,
		{ ".aaaa", "....a", "aaaaa", "a..a.", "aaaa." },
		{ Theme::Bone, Theme::Bone, Theme::Bone },
	};

	constexpr PixelGlyph Close =
	{
		5, 5,
		{ "a...a", ".a.a.", "..a..", ".a.a.", "a...a" },
		{ Theme::Bone, Theme::Bone, Theme::Bone },
	};
}
