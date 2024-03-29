#include "stdio.h"
#include <MetalOS.h>
#include "Debug.h"
#include <string.h>

FILE* fopen(char const* _FileName, char const* _Mode)
{
	//inspect mode
	return CreateFile(_FileName, GenericAccess::Read);
}

size_t fread(void* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream)
{
	if (!_ElementSize || !_ElementCount)
		return 0;
	
	size_t bytes;
	SystemCallResult result = (SystemCallResult)ReadFile(_Stream, _Buffer, _ElementSize * _ElementCount, &bytes);
	Assert(result == SystemCallResult::Success);

	return bytes / _ElementSize;
}

LIBRARY size_t fwrite(void const* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream)
{
	if (!_ElementSize || !_ElementCount)
		return 0;

	size_t bytes;
	SystemCallResult result = (SystemCallResult)WriteFile(_Stream, _Buffer, _ElementSize * _ElementCount, &bytes);
	Assert(result == SystemCallResult::Success);

	return bytes / _ElementSize;
}

int fseek(FILE* _Stream, long _Offset, int _Origin)
{
	enum FilePointerMove move;
	switch (_Origin)
	{
	case SEEK_SET:
		move = FilePointerMove::Begin;
		break;

	case SEEK_CUR:
		move = FilePointerMove::Current;
		break;

	case SEEK_END:
		move = FilePointerMove::End;
		break;

	default:
		return -1;
	}
	
	size_t newPos;

	SystemCallResult result = (SystemCallResult)SetFilePointer(_Stream, _Offset, move, &newPos);
	return newPos;
}

LIBRARY int ftell(FILE* _Stream)
{
	return fseek(_Stream, 0, SEEK_CUR);
}

int fclose(FILE* _Stream)
{
	return static_cast<int>(CloseFile(_Stream));
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

int rename(char const* _OldFileName, char const* _NewFileName)
{
	return static_cast<int>(MoveFile(_OldFileName, _NewFileName));
}

int remove(char const* _FileName)
{
	return static_cast<int>(DeleteFile(_FileName));
}
