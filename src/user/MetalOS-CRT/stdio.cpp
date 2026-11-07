#include "user/MetalOS-CRT/CRT.h"
#include "user/MetalOS.h"
#include "ToErrno.h"
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

		const SyscallResult result = WriteFile(file, buffer, length, nullptr);
		if (result != SyscallResult::Success)
		{
			errno = ToErrno(result);
			return false;
		}

		return true;
	}

	//A handle that isn't a file (a pipe or the console) can't seek, and RT rejects positions outside the file
	int SeekErrno(const SyscallResult result)
	{
		switch (result)
		{
		case SyscallResult::InvalidObject:
			return ESPIPE;

		case SyscallResult::Failed:
			return EINVAL;

		default:
			return ToErrno(result);
		}
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

	//Value of a digit in any base up to 16, or 16 for a character that isn't one
	unsigned ScanDigit(const char c)
	{
		if (c >= '0' && c <= '9')
			return c - '0';

		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;

		if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;

		return 16;
	}

	//Reads an integer of at most width characters (base 0 takes it from the prefix), returning the characters used or 0 if they aren't a number
	size_t ScanInteger(const char* const input, const size_t width, unsigned base, uint64_t& value)
	{
		size_t length = 0;
		const bool negative = input[0] == '-';
		if (negative || input[0] == '+')
			length++;

		//A 0x that fits in the width is a prefix, and a hex digit has to follow it
		if ((base == 0 || base == 16) && length + 1 < width && input[length] == '0' && (input[length + 1] == 'x' || input[length + 1] == 'X'))
		{
			base = 16;
			length += 2;
		}
		else if (base == 0)
		{
			base = input[length] == '0' ? 8 : 10;
		}

		const size_t digits = length;
		uint64_t result = 0;
		while (length < width && ScanDigit(input[length]) < base)
			result = result * base + ScanDigit(input[length++]);

		if (length == digits)
			return 0;

		value = negative ? 0 - result : result;
		return length;
	}

	//Stores a converted integer into a destination as wide as the length modifier says
	void ScanStore(void* const destination, const size_t bytes, const uint64_t value)
	{
		switch (bytes)
		{
		case sizeof(uint8_t):
			*static_cast<uint8_t*>(destination) = (uint8_t)value;
			break;

		case sizeof(uint16_t):
			*static_cast<uint16_t*>(destination) = (uint16_t)value;
			break;

		case sizeof(uint32_t):
			*static_cast<uint32_t*>(destination) = (uint32_t)value;
			break;

		default:
			*static_cast<uint64_t*>(destination) = value;
			break;
		}
	}

	//Marks the characters the scanset after a %[ matches, returning the format past its ] or nullptr if it isn't closed
	const char* ScanSet(const char* format, bool (&set)[256])
	{
		const bool invert = *format == '^';
		if (invert)
			format++;

		memset(set, 0, sizeof(set));

		//A ] first in the set is a member rather than its end
		if (*format == ']')
			set[(unsigned char)*format++] = true;

		for (; *format != ']'; format++)
		{
			if (*format == '\0')
				return nullptr;

			//A - between two members is a range, and at either end it is a member
			if (format[1] == '-' && format[2] != ']' && format[2] != '\0')
			{
				const unsigned char first = format[0];
				const unsigned char last = format[2];
				const unsigned low = first < last ? first : last;
				const unsigned high = first < last ? last : first;
				for (unsigned c = low; c <= high; c++)
					set[c] = true;

				format += 2;
				continue;
			}

			set[(unsigned char)*format] = true;
		}

		if (invert)
		{
			for (bool& member : set)
				member = !member;
		}

		return format + 1;
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

		//Append needs every write to go to the end, which RT can't ask for yet
		case 'a':
			errno = ENOSYS;
			return nullptr;

		default:
			errno = EINVAL;
			return nullptr;
		}

		if (strchr(_Mode, '+'))
			access = FileAccess::ReadWrite;

		//RT doesn't say why an open failed, and a missing file is the usual reason
		const HFile file = CreateFile(_FileName, access);
		if (!file)
			errno = ENOENT;

		return reinterpret_cast<FILE*>(file);
	}

	size_t fread(void* _Buffer, size_t _ElementSize, size_t _ElementCount, FILE* _Stream)
	{
		if (!_ElementSize || !_ElementCount)
			return 0;

		//A pipe returns what has arrived so far, so keep reading until the count is met or the stream ends
		const size_t total = _ElementSize * _ElementCount;
		size_t bytes = 0;
		while (bytes < total)
		{
			size_t read = 0;
			const SyscallResult result = ReadFile(reinterpret_cast<HFile>(_Stream), static_cast<char*>(_Buffer) + bytes, total - bytes, &read);
			if (result != SyscallResult::Success && result != SyscallResult::BrokenPipe)
			{
				errno = ToErrno(result);
				break;
			}

			//BrokenPipe is an empty pipe whose writer has gone, so it reads as end of file
			bytes += read;
			if (result == SyscallResult::BrokenPipe || read == 0)
				break;
		}

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
			errno = EINVAL;
			return -1;
		}

		const SyscallResult result = SetFilePointer(reinterpret_cast<HFile>(_Stream), _Offset, move, nullptr);
		if (result != SyscallResult::Success)
		{
			errno = SeekErrno(result);
			return -1;
		}

		return 0;
	}

	long ftell(FILE* _Stream)
	{
		size_t position = 0;
		const SyscallResult result = SetFilePointer(reinterpret_cast<HFile>(_Stream), 0, Seek::Current, &position);
		if (result != SyscallResult::Success)
		{
			errno = SeekErrno(result);
			return -1;
		}

		return (long)position;
	}

	int fclose(FILE* _Stream)
	{
		const SyscallResult result = CloseHandle(reinterpret_cast<HFile>(_Stream));
		if (result != SyscallResult::Success)
		{
			errno = ToErrno(result);
			return EOF;
		}

		return 0;
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
		UNUSED(stream);
		//Do nothing
		return 0;
	}

	int rename(char const* _OldFileName, char const* _NewFileName)
	{
		const SyscallResult result = MoveFile(_OldFileName, _NewFileName);
		if (result != SyscallResult::Success)
		{
			errno = ToErrno(result);
			return -1;
		}

		return 0;
	}

	int remove(char const* _FileName)
	{
		const SyscallResult result = DeleteFile(_FileName);
		if (result != SyscallResult::Success)
		{
			errno = ToErrno(result);
			return -1;
		}

		return 0;
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

	int vsscanf(const char* const s, const char* format, va_list arg)
	{
		const char* input = s;
		int assigned = 0;

		//Running out of input before anything was assigned is EOF, any other failure returns what was assigned
		const auto failed = [&](const bool inputFailure) { return inputFailure && assigned == 0 ? EOF : assigned; };

		while (*format != '\0')
		{
			//Whitespace in the format matches any amount of whitespace in the input, including none
			if (isspace(*format))
			{
				while (isspace(*format))
					format++;

				while (isspace(*input))
					input++;

				continue;
			}

			//Anything else but a conversion has to match the input, and %% matches a % after any whitespace
			if (*format != '%' || format[1] == '%')
			{
				if (*format == '%')
				{
					format++;
					while (isspace(*input))
						input++;
				}

				if (*input == '\0')
					return failed(true);

				if (*input != *format)
					return failed(false);

				input++;
				format++;
				continue;
			}

			format++;
			const bool suppress = *format == '*';
			if (suppress)
				format++;

			size_t width = 0;
			while (isdigit(*format))
				width = width * 10 + (*format++ - '0');

			//The l that makes an integer a long makes %c, %s and %[ store wchar_t
			size_t bytes = sizeof(int);
			bool wide = false;
			switch (*format)
			{
			case 'h':
				format++;
				bytes = sizeof(short);
				if (*format == 'h')
				{
					format++;
					bytes = sizeof(char);
				}
				break;

			case 'l':
				format++;
				bytes = sizeof(long);
				wide = true;
				if (*format == 'l')
				{
					format++;
					bytes = sizeof(long long);
					wide = false;
				}
				break;

			case 'j':
				format++;
				bytes = sizeof(intmax_t);
				break;

			case 'z':
				format++;
				bytes = sizeof(size_t);
				break;

			case 't':
				format++;
				bytes = sizeof(ptrdiff_t);
				break;
			}

			const char conversion = *format++;

			//Every conversion but %c, %[ and %n skips leading whitespace
			if (conversion != 'c' && conversion != '[' && conversion != 'n')
			{
				while (isspace(*input))
					input++;
			}

			unsigned base = 10;
			switch (conversion)
			{
			case 'n':
				if (!suppress)
					ScanStore(va_arg(arg, void*), bytes, (uint64_t)(input - s));
				continue;

			case 'c':
			case 's':
			case '[':
			{
				bool set[256];
				if (conversion == '[')
				{
					format = ScanSet(format, set);
					if (!format)
						return failed(false);
				}

				if (*input == '\0')
					return failed(true);

				//%c takes one character unless given a width
				const size_t limit = width ? width : conversion == 'c' ? 1 : (size_t)-1;
				size_t length = 0;
				while (length < limit && input[length] != '\0')
				{
					const unsigned char c = input[length];
					if ((conversion == 's' && isspace(c)) || (conversion == '[' && !set[c]))
						break;

					length++;
				}

				if (length == 0)
					return failed(false);

				if (!suppress)
				{
					//Only %c leaves the characters unterminated
					void* const destination = va_arg(arg, void*);
					const size_t stored = conversion == 'c' ? length : length + 1;
					for (size_t i = 0; i < stored; i++)
					{
						const char c = i < length ? input[i] : '\0';
						if (wide)
							static_cast<wchar_t*>(destination)[i] = (unsigned char)c;
						else
							static_cast<char*>(destination)[i] = c;
					}
					assigned++;
				}

				input += length;
				continue;
			}

			case 'p':
				bytes = sizeof(void*);
				base = 16;
				break;

			case 'x':
			case 'X':
				base = 16;
				break;

			case 'o':
				base = 8;
				break;

			case 'i':
				base = 0;
				break;

			case 'd':
			case 'u':
				break;

			//Floating point isn't supported, so it fails like an unknown conversion
			default:
				return failed(false);
			}

			if (*input == '\0')
				return failed(true);

			uint64_t value = 0;
			const size_t length = ScanInteger(input, width ? width : (size_t)-1, base, value);
			if (length == 0)
				return failed(false);

			if (!suppress)
			{
				ScanStore(va_arg(arg, void*), bytes, value);
				assigned++;
			}

			input += length;
		}

		return assigned;
	}

	int sscanf ( const char * s, const char * format, ...)
	{
		va_list args;
		va_start(args, format);
		const int result = vsscanf(s, format, args);
		va_end(args);
		return result;
	}
}
