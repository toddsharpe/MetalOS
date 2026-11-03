#pragma once

#include "user/MetalOS.h"
#include <stdio.h>

typedef void* FILE;
/* Seek method constants */

#define SEEK_CUR    1
#define SEEK_END    2
#define SEEK_SET    0

/* Buffered I/O macros */

#define BUFSIZ  512

//The core_crt formatter, which hands each character to a callback
int kvprintf(char const* fmt, void (*func)(int, void*), void* arg, int radix, va_list ap);

namespace
{
	//Formatted output collects here and goes to the stream a chunk at a time
	struct StreamBuffer
	{
		HFile File;
		size_t Count;
		bool Failed;
		char Data[BUFSIZ];
	};

	//WriteFile writes everything or fails, and rejects zero-length writes
	bool StreamWrite(const HFile file, const void* const buffer, const size_t length)
	{
		if (length == 0)
			return true;

		return WriteFile(file, buffer, length, nullptr) == SyscallResult::Success;
	}

	void StreamFlush(StreamBuffer& stream)
	{
		if (!StreamWrite(stream.File, stream.Data, stream.Count))
			stream.Failed = true;
		stream.Count = 0;
	}

	void StreamPut(int ch, void* arg)
	{
		StreamBuffer& stream = *static_cast<StreamBuffer*>(arg);
		stream.Data[stream.Count++] = (char)ch;
		if (stream.Count == sizeof(stream.Data))
			StreamFlush(stream);
	}
}

extern "C"
{
	FILE* fopen(char const* _FileName, char const* _Mode)
	{
		FileAccess access;
		switch (_Mode[0])
		{
		case 'r':
			access = FileAccess::Read;
			break;

		case 'w':
			access = FileAccess::Write;
			break;

		//"a" lands here too: append needs every write to go to the end, which RT can't ask for yet
		default:
			return nullptr;
		}

		if (strchr(_Mode, '+'))
			access = FileAccess::ReadWrite;

		return reinterpret_cast<FILE*>(CreateFile(_FileName, access));
	}

	size_t fread(void* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream)
	{
		if (!_ElementSize || !_ElementCount)
			return 0;

		size_t bytes = 0;
		if (ReadFile(reinterpret_cast<HFile>(_Stream), _Buffer, _ElementSize * _ElementCount, &bytes) != SyscallResult::Success)
			return 0;

		return bytes / _ElementSize;
	}

	size_t fwrite(void const* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream)
	{
		if (!_ElementSize || !_ElementCount)
			return 0;

		if (!StreamWrite(reinterpret_cast<HFile>(_Stream), _Buffer, _ElementSize * _ElementCount))
			return 0;

		return _ElementCount;
	}

	int fseek(FILE* _Stream, long _Offset, int _Origin)
	{
		enum Seek move;
		switch (_Origin)
		{
		case SEEK_SET:
			move = Seek::Begin;
			break;

		case SEEK_CUR:
			move = Seek::Current;
			break;

		case SEEK_END:
			move = Seek::End;
			break;

		default:
			return -1;
		}

		if (SetFilePointer(reinterpret_cast<HFile>(_Stream), _Offset, move, nullptr) != SyscallResult::Success)
			return -1;

		return 0;
	}

	long ftell(FILE* _Stream)
	{
		size_t position = 0;
		if (SetFilePointer(reinterpret_cast<HFile>(_Stream), 0, Seek::Current, &position) != SyscallResult::Success)
			return -1;

		return (long)position;
	}

	int fclose(FILE* _Stream)
	{
		return static_cast<int>(CloseHandle(reinterpret_cast<HFile>(_Stream)));
	}

	int vfprintf(FILE* const _Stream, char const* const _Format, va_list _ArgList)
	{
		StreamBuffer stream = {};
		stream.File = reinterpret_cast<HFile>(_Stream);

		const int count = kvprintf(_Format, StreamPut, &stream, 10, _ArgList);
		StreamFlush(stream);
		return stream.Failed ? EOF : count;
	}

	int fprintf(FILE* const _Stream, char const* const _Format, ...)
	{
		va_list args;
		va_start(args, _Format);
		int result = vfprintf(_Stream, _Format, args);
		va_end(args);
		return result;
	}

	int fflush(FILE *stream)
	{
		//Do nothing
		return 0;
	}

	int rename(char const* _OldFileName, char const* _NewFileName)
	{
		return static_cast<int>(MoveFile(_OldFileName, _NewFileName));
	}

	int remove(char const* _FileName)
	{
		return static_cast<int>(DeleteFile(_FileName));
	}

	int printf(char const* const _Format, ...)
	{
		va_list args;
		va_start(args, _Format);
		int result = vfprintf((FILE*)stdout, _Format, args);
		va_end(args);
		return result;
	}

	int puts ( const char * str )
	{
		const HFile out = reinterpret_cast<HFile>(stdout);
		if (!StreamWrite(out, str, strlen(str)) || !StreamWrite(out, "\n", 1))
			return EOF;

		return 0;
	}

	int putchar ( int character )
	{
		const char c = (char)character;
		if (!StreamWrite(reinterpret_cast<HFile>(stdout), &c, 1))
			return EOF;

		return (unsigned char)c;
	}

	int sscanf ( const char * s, const char * format, ...)
	{
		//Not implemented
		return -1;
	}
}
