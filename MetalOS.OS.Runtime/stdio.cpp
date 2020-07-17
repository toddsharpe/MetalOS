#include "stdio.h"
#include <MetalOS.h>
#include "Debug.h"
#include <string.h>

FILE* fopen(char const* _FileName, char const* _Mode)
{
	//inspect mode
	return CreateFile(_FileName, Read);
}

size_t fread(void* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream)
{
	if (!_ElementSize || !_ElementCount)
		return 0;
	
	size_t bytes;
	SystemCallResult result = (SystemCallResult)ReadFile(_Stream, _Buffer, _ElementSize * _ElementCount, &bytes);
	Assert(result == Success);

	return bytes / _ElementSize;
}

int fseek(FILE* _Stream, long _Offset, int _Origin)
{
	enum FilePointerMove move;
	switch (_Origin)
	{
	case SEEK_SET:
		move = Begin;
		break;

	case SEEK_CUR:
		move = Current;
		break;

	case SEEK_END:
		move = End;
		break;

	default:
		return -1;
	}
	
	return SetFilePointer(_Stream, _Offset, move);
}

int fclose(FILE* _Stream)
{
	return CloseFile(_Stream);
}

//TODO: make this real, it just prints to debug stream
int fprintf(FILE* const _Stream, char const* const _Format, ...)
{
	va_list args;
	va_start(args, _Format);

	char buffer[255];
	int retval = vsprintf(buffer, _Format, args);
	buffer[retval] = '\0';
	va_end(args);

	DebugPrint(buffer);
	return strlen(buffer);
}
