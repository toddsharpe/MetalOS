#include "Kernel.h"
#include "Assert.h"

#include "Font.h"

const char* Font::GetCharacterMap(char c) const
{
	Assert(c <= 0x7F);
	
	return (*this->FontMap)[c];
}
