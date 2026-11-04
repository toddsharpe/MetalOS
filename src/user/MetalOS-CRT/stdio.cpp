#pragma once

#include "user/MetalOS.h"
#include "user/MetalOS-CRT/errno.cpp"
#include "user/MetalOS-CRT/strtod.cpp"
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

	//A character pushed back onto a stream, which the next read takes first; Key is the handle plus one, so a zeroed slot is free (stdin is handle 0)
	struct Pushback
	{
		volatile __int64 Key;
		int Character;
	};

	//Handles count up without reuse, so they can't index a table: the few streams holding a character share these slots
	Pushback Pushbacks[16];

	//The stream's pushback slot, or with claim a free one for it that holds nothing yet (nullptr if none is left)
	Pushback* FindPushback(const HFile file, const bool claim)
	{
		const __int64 key = (__int64)file + 1;
		for (Pushback& slot : Pushbacks)
		{
			if (slot.Key == key)
				return &slot;
		}

		if (!claim)
			return nullptr;

		//Threads using other streams can be claiming slots at the same time
		for (Pushback& slot : Pushbacks)
		{
			if (slot.Key == 0 && _InterlockedCompareExchange64(&slot.Key, key, 0) == 0)
			{
				slot.Character = EOF;
				return &slot;
			}
		}

		return nullptr;
	}

	bool HasPushback(const HFile file)
	{
		const Pushback* const slot = FindPushback(file, false);
		return slot && slot->Character != EOF;
	}

	//Takes the character pushed back onto a stream and frees its slot, or returns EOF if nothing is there
	int TakePushback(const HFile file)
	{
		Pushback* const slot = FindPushback(file, false);
		if (!slot || slot->Character == EOF)
			return EOF;

		const int character = slot->Character;
		slot->Key = 0;
		return character;
	}

	//Reads a character from the handle, or EOF at the end or on an error (which sets errno)
	int ReadChar(const HFile file)
	{
		//BrokenPipe comes with nothing read: it is the end of a pipe
		unsigned char character = 0;
		size_t bytes = 0;
		const SyscallResult result = ReadFile(file, &character, 1, &bytes);
		if (bytes == 1)
			return character;

		if (result != SyscallResult::Success && result != SyscallResult::BrokenPipe)
			errno = ToErrno(result);

		return EOF;
	}

	//Characters for the scanners, read from a stream; the one looked at but not taken waits in the stream's pushback slot
	struct StreamSource
	{
		HFile File;
		Pushback* Slot;
		bool Ended;

		int Peek()
		{
			if (Slot->Character == EOF && !Ended)
			{
				Slot->Character = ReadChar(File);
				Ended = Slot->Character == EOF;
			}

			return Slot->Character;
		}

		void Advance()
		{
			Slot->Character = EOF;
		}
	};

	//Reads an integer of at most width characters (base 0 takes it from the prefix), returning the characters read or 0 if they aren't a number
	template<typename Source>
	size_t ScanInteger(Source& source, const size_t width, unsigned base, uint64_t& value)
	{
		size_t length = 0;
		const auto peek = [&]() { return length < width ? source.Peek() : EOF; };
		const auto next = [&]()
		{
			source.Advance();
			length++;
		};

		const bool negative = peek() == '-';
		if (negative || peek() == '+')
			next();

		//A 0x is a prefix where the base allows one, and a hex digit has to follow it
		bool digits = false;
		if ((base == 0 || base == 16) && peek() == '0')
		{
			next();
			if (tolower(peek()) == 'x')
			{
				next();
				base = 16;
			}
			else
			{
				digits = true;
				if (base == 0)
					base = 8;
			}
		}
		else if (base == 0)
		{
			base = 10;
		}

		uint64_t result = 0;
		while (ScanDigit(peek()) < base)
		{
			result = result * base + ScanDigit(peek());
			next();
			digits = true;
		}

		if (!digits)
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

		//A character pushed back by ungetc or a scanf comes first
		const HFile file = reinterpret_cast<HFile>(_Stream);
		const size_t total = _ElementSize * _ElementCount;
		const int pushed = TakePushback(file);
		size_t bytes = 0;
		if (pushed != EOF)
			static_cast<unsigned char*>(_Buffer)[bytes++] = (unsigned char)pushed;

		//A pipe returns what has arrived so far, so keep reading until the count is met or the stream ends
		while (bytes < total)
		{
			size_t read = 0;
			const SyscallResult result = ReadFile(file, static_cast<unsigned char*>(_Buffer) + bytes, total - bytes, &read);
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

		//A pushed back character sits one before the handle's position, and a seek that works drops it
		const HFile file = reinterpret_cast<HFile>(_Stream);
		const long offset = _Origin == SEEK_CUR && HasPushback(file) ? _Offset - 1 : _Offset;
		const SyscallResult result = SetFilePointer(file, offset, move, nullptr);
		if (result != SyscallResult::Success)
		{
			errno = SeekErrno(result);
			return -1;
		}

		TakePushback(file);
		return 0;
	}

	long ftell(FILE* _Stream)
	{
		size_t position = 0;
		const HFile file = reinterpret_cast<HFile>(_Stream);
		const SyscallResult result = SetFilePointer(file, 0, Seek::Current, &position);
		if (result != SyscallResult::Success)
		{
			errno = SeekErrno(result);
			return -1;
		}

		//The caller hasn't read a character that is pushed back
		if (position > 0 && HasPushback(file))
			position--;

		return (long)position;
	}

	int fclose(FILE* _Stream)
	{
		//The stream goes away whether or not the close works, and anything pushed back onto it with it
		TakePushback(reinterpret_cast<HFile>(_Stream));
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
}

namespace
{
	//The scanf engine: reads from a string or a stream as format directs, storing through arg
	template<typename Source>
	int Scan(Source& source, const char* format, va_list arg)
	{
		size_t count = 0;
		int assigned = 0;

		//Running out of input before anything was assigned is EOF, any other failure returns what was assigned
		const auto failed = [&](const bool inputFailure) { return inputFailure && assigned == 0 ? EOF : assigned; };
		const auto next = [&]()
		{
			source.Advance();
			count++;
		};
		const auto skipSpace = [&]()
		{
			while (isspace(source.Peek()))
				next();
		};

		while (*format != '\0')
		{
			//Whitespace in the format matches any amount of whitespace in the input, including none
			if (isspace(*format))
			{
				while (isspace(*format))
					format++;

				skipSpace();
				continue;
			}

			//Anything else but a conversion has to match the input, and %% matches a % after any whitespace
			if (*format != '%' || format[1] == '%')
			{
				if (*format == '%')
				{
					format++;
					skipSpace();
				}

				const int c = source.Peek();
				if (c == EOF)
					return failed(true);

				if (c != (unsigned char)*format)
					return failed(false);

				next();
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

			//The l that makes an integer a long makes %c, %s and %[ store wchar_t, and it or L makes a float a double
			size_t bytes = sizeof(int);
			bool wide = false;
			bool isDouble = false;
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
				isDouble = true;
				if (*format == 'l')
				{
					format++;
					bytes = sizeof(long long);
					wide = false;
				}
				break;

			case 'L':
				format++;
				isDouble = true;
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
				skipSpace();

			unsigned base = 10;
			switch (conversion)
			{
			case 'n':
				if (!suppress)
					ScanStore(va_arg(arg, void*), bytes, (uint64_t)count);
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

				if (source.Peek() == EOF)
					return failed(true);

				//Characters go straight into the destination as they are read
				void* const destination = suppress ? nullptr : va_arg(arg, void*);
				const auto store = [&](const size_t index, const int c)
				{
					if (wide)
						static_cast<wchar_t*>(destination)[index] = (wchar_t)c;
					else
						static_cast<char*>(destination)[index] = (char)c;
				};

				//%c takes one character unless given a width
				const size_t limit = width ? width : conversion == 'c' ? 1 : (size_t)-1;
				size_t length = 0;
				for (; length < limit; length++)
				{
					const int c = source.Peek();
					if (c == EOF || (conversion == 's' && isspace(c)) || (conversion == '[' && !set[c]))
						break;

					if (destination)
						store(length, c);
					next();
				}

				if (length == 0)
					return failed(false);

				//Only %c leaves the characters unterminated
				if (destination)
				{
					if (conversion != 'c')
						store(length, '\0');
					assigned++;
				}

				continue;
			}

			case 'a':
			case 'A':
			case 'e':
			case 'E':
			case 'f':
			case 'F':
			case 'g':
			case 'G':
			{
				if (source.Peek() == EOF)
					return failed(true);

				//All of what was read has to be the number, so a 1e that stops short is a matching failure
				const ScannedFloat number = ScanFloat(source, width ? width : (size_t)-1, !isDouble);
				count += number.Read;
				if (number.Length == 0 || number.Length < number.Read)
					return failed(false);

				if (!suppress)
				{
					if (isDouble)
						*va_arg(arg, double*) = DoubleFromBits(number.Bits);
					else
						*va_arg(arg, float*) = FloatFromBits((uint32_t)number.Bits);
					assigned++;
				}

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

			//An unknown conversion stops the scan
			default:
				return failed(false);
			}

			if (source.Peek() == EOF)
				return failed(true);

			uint64_t value = 0;
			const size_t length = ScanInteger(source, width ? width : (size_t)-1, base, value);
			if (length == 0)
				return failed(false);

			count += length;
			if (!suppress)
			{
				ScanStore(va_arg(arg, void*), bytes, value);
				assigned++;
			}
		}

		return assigned;
	}
}

extern "C"
{
	int vsscanf(const char* const s, const char* format, va_list arg)
	{
		StringSource source = { s };
		return Scan(source, format, arg);
	}

	int sscanf ( const char * s, const char * format, ...)
	{
		va_list args;
		va_start(args, format);
		const int result = vsscanf(s, format, args);
		va_end(args);
		return result;
	}

	int vfscanf(FILE* const _Stream, char const* const _Format, va_list _ArgList)
	{
		//The character the scan stops at stays with the stream, so the scan holds a pushback slot throughout
		StreamSource source = {};
		source.File = reinterpret_cast<HFile>(_Stream);
		source.Slot = FindPushback(source.File, true);
		if (!source.Slot)
		{
			errno = EMFILE;
			return EOF;
		}

		const int result = Scan(source, _Format, _ArgList);
		if (source.Slot->Character == EOF)
			source.Slot->Key = 0;

		return result;
	}

	int fscanf(FILE* const _Stream, char const* const _Format, ...)
	{
		va_list args;
		va_start(args, _Format);
		const int result = vfscanf(_Stream, _Format, args);
		va_end(args);
		return result;
	}

	int vscanf(char const* const _Format, va_list _ArgList)
	{
		return vfscanf(stdin, _Format, _ArgList);
	}

	int scanf(char const* const _Format, ...)
	{
		va_list args;
		va_start(args, _Format);
		const int result = vfscanf(stdin, _Format, args);
		va_end(args);
		return result;
	}

	int fgetc(FILE* _Stream)
	{
		const HFile file = reinterpret_cast<HFile>(_Stream);
		const int pushed = TakePushback(file);
		return pushed != EOF ? pushed : ReadChar(file);
	}

	int getc(FILE* _Stream)
	{
		return fgetc(_Stream);
	}

	int getchar(void)
	{
		return fgetc(stdin);
	}

	int ungetc(int _Character, FILE* _Stream)
	{
		//One character can wait on each stream, while a slot is free for it
		Pushback* const slot = _Character == EOF ? nullptr : FindPushback(reinterpret_cast<HFile>(_Stream), true);
		if (!slot || slot->Character != EOF)
			return EOF;

		slot->Character = (unsigned char)_Character;
		return slot->Character;
	}
}
