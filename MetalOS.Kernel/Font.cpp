#include "Font.h"

#include "Main.h"

const char* Font::GetCharacterMap(char c) const
{
	Assert(c <= 0x7F);
	
	return (*this->FontMap)[c];
}
