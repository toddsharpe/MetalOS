#include "Font.h"

#include "MetalOS.Kernel.h"
#include "Main.h"

const char* Font::GetCharacterMap(char c)
{
	Assert(c <= 0x7F);
	
	return (*this->FontMap)[c];
}
