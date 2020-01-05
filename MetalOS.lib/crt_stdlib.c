#include "crt_stdlib.h"
#include "crt_string.h"

size_t mbstowcs(CHAR16* _Dest, char const* _Source, size_t _MaxCount)
{
	const size_t len = strlen(_Source);
	if (_MaxCount < len * sizeof(CHAR16))
		return (size_t)-1;

	for (size_t i = 0; i < len; i++)
	{
		_Dest[i] = (CHAR16)_Source[i];
	}

	return len;
}
