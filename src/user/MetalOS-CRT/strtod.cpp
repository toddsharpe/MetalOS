#pragma once

#include "user/MetalOS.h"
#include "user/MetalOS-CRT/errno.cpp"
#include <stdio.h>
#include <stdlib.h>

namespace
{
	//Value of a digit in any base up to 16, or 16 for a character that isn't one
	unsigned ScanDigit(const int c)
	{
		if (c >= '0' && c <= '9')
			return c - '0';

		if (c >= 'a' && c <= 'f')
			return c - 'a' + 10;

		if (c >= 'A' && c <= 'F')
			return c - 'A' + 10;

		return 16;
	}

	//Characters for the scanners, taken from a string
	struct StringSource
	{
		const char* Next;

		int Peek() const
		{
			return *Next != '\0' ? (unsigned char)*Next : EOF;
		}

		void Advance()
		{
			Next++;
		}
	};

	//A double or float and its bits
	union DoubleBits
	{
		double Value;
		uint64_t Bits;
	};

	union FloatBits
	{
		float Value;
		uint32_t Bits;
	};

	double DoubleFromBits(const uint64_t bits)
	{
		DoubleBits value;
		value.Bits = bits;
		return value.Value;
	}

	float FloatFromBits(const uint32_t bits)
	{
		FloatBits value;
		value.Bits = bits;
		return value.Value;
	}

	uint64_t InfinityBits(const bool single)
	{
		return single ? 0x7F800000 : 0x7FF0000000000000;
	}

	//Rounds mantissa * 2^exponent to the nearest double (or with single, float) bits, ties to even; sticky means a little more than that
	uint64_t RoundBits(uint64_t mantissa, int exponent, const bool sticky, const bool single, bool& range)
	{
		const int mantissaBits = single ? 23 : 52;
		const int maxExponent = single ? 127 : 1023;
		const int minExponent = 1 - maxExponent;
		if (mantissa == 0)
			return 0;

		//With the top bit set, top is the exponent of the leading bit
		while ((mantissa >> 63) == 0)
		{
			mantissa <<= 1;
			exponent--;
		}

		int top = exponent + 63;
		if (top > maxExponent)
		{
			range = true;
			return InfinityBits(single);
		}

		//A normal number keeps mantissaBits + 1 bits, a subnormal fewer
		int drop = 63 - mantissaBits;
		if (top < minExponent)
			drop = minExponent - top > 64 ? 65 : drop + minExponent - top;

		uint64_t kept = 0;
		bool half = false;
		bool below = true;
		if (drop <= 64)
		{
			kept = drop < 64 ? mantissa >> drop : 0;
			half = (mantissa >> (drop - 1)) & 1;
			below = sticky || (mantissa & (((uint64_t)1 << (drop - 1)) - 1)) != 0;
		}

		//Underflow is a result below the smallest normal that isn't exact
		if (top < minExponent && (half || below))
			range = true;

		if (half && (below || (kept & 1)))
			kept++;

		//A subnormal is its own bits, and rounding up into the smallest normal sets the exponent's low bit
		if (top < minExponent)
			return kept;

		if (kept >> (mantissaBits + 1))
		{
			kept >>= 1;
			top++;
			if (top > maxExponent)
			{
				range = true;
				return InfinityBits(single);
			}
		}

		return ((uint64_t)(top + maxExponent) << mantissaBits) | (kept & (((uint64_t)1 << mantissaBits) - 1));
	}

	//Decimal digits of a number being converted, enough to round any double correctly
	struct Decimal
	{
		static constexpr int Capacity = 800;

		//The value is 0.Digits * 10^Point, and a left shift writes up to 21 digits past the capacity before cutting back
		uint8_t Digits[Capacity + 24];
		int Count;
		int Point;
		bool Truncated;
	};

	//Adds the next digit read, before or after the radix point
	void AppendDigit(Decimal& decimal, const uint8_t digit, const bool fraction)
	{
		//Leading zeros only move the point
		if (decimal.Count == 0 && digit == 0)
		{
			if (fraction)
				decimal.Point--;
			return;
		}

		if (!fraction)
			decimal.Point++;

		if (decimal.Count < Decimal::Capacity)
			decimal.Digits[decimal.Count++] = digit;
		else if (digit != 0)
			decimal.Truncated = true;
	}

	//Trailing zeros don't change the value
	void Trim(Decimal& decimal)
	{
		while (decimal.Count > 0 && decimal.Digits[decimal.Count - 1] == 0)
			decimal.Count--;
	}

	//Divides by 2^shift, which is at most 60 so that ten times the remainder fits in 64 bits
	void RightShift(Decimal& decimal, const unsigned shift)
	{
		//Take enough leading digits for the first digit of the quotient
		int read = 0;
		uint64_t n = 0;
		while ((n >> shift) == 0)
		{
			n = n * 10 + (read < decimal.Count ? decimal.Digits[read] : 0);
			read++;
		}
		decimal.Point -= read - 1;

		const uint64_t mask = ((uint64_t)1 << shift) - 1;
		int write = 0;
		for (; read < decimal.Count; read++)
		{
			decimal.Digits[write++] = (uint8_t)(n >> shift);
			n = (n & mask) * 10 + decimal.Digits[read];
		}

		//The remainder adds digits at the end, as many as fit
		while (n > 0)
		{
			const uint8_t digit = (uint8_t)(n >> shift);
			if (write < Decimal::Capacity)
				decimal.Digits[write++] = digit;
			else if (digit != 0)
				decimal.Truncated = true;
			n = (n & mask) * 10;
		}

		decimal.Count = write;
		Trim(decimal);
	}

	//Multiplies by 2^shift, which is at most 60
	void LeftShift(Decimal& decimal, const unsigned shift)
	{
		//2^shift has at most shift / 3 + 1 digits, so the product has at most that many more
		const int end = decimal.Count + shift / 3 + 1;
		int read = decimal.Count;
		int write = end;
		uint64_t n = 0;
		while (read > 0)
		{
			n += (uint64_t)decimal.Digits[--read] << shift;
			const uint64_t quotient = n / 10;
			decimal.Digits[--write] = (uint8_t)(n - quotient * 10);
			n = quotient;
		}

		while (n > 0)
		{
			const uint64_t quotient = n / 10;
			decimal.Digits[--write] = (uint8_t)(n - quotient * 10);
			n = quotient;
		}

		//Move the product to the front and keep what fits
		decimal.Point += end - write - decimal.Count;
		decimal.Count = end - write;
		memmove(decimal.Digits, decimal.Digits + write, decimal.Count);
		for (; decimal.Count > Decimal::Capacity; decimal.Count--)
		{
			if (decimal.Digits[decimal.Count - 1] != 0)
				decimal.Truncated = true;
		}

		Trim(decimal);
	}

	//Rounds a decimal to the nearest double (or with single, float) bits, without the sign
	uint64_t DecimalBits(Decimal& decimal, const bool single, bool& range)
	{
		if (decimal.Count == 0)
			return 0;

		//An exact mantissa and power of ten need only one rounded multiply or divide
		const int power = decimal.Point - decimal.Count;
		const int magnitude = power < 0 ? -power : power;
		if (decimal.Count <= 19)
		{
			uint64_t mantissa = 0;
			for (int i = 0; i < decimal.Count; i++)
				mantissa = mantissa * 10 + decimal.Digits[i];

			if (single && mantissa <= ((uint64_t)1 << 24) && magnitude <= 10)
			{
				float scale = 1;
				for (int i = 0; i < magnitude; i++)
					scale *= 10;

				FloatBits value;
				value.Value = power < 0 ? (float)(int64_t)mantissa / scale : (float)(int64_t)mantissa * scale;
				return value.Bits;
			}

			if (!single && mantissa <= ((uint64_t)1 << 53) && magnitude <= 22)
			{
				double scale = 1;
				for (int i = 0; i < magnitude; i++)
					scale *= 10;

				DoubleBits value;
				value.Value = power < 0 ? (double)(int64_t)mantissa / scale : (double)(int64_t)mantissa * scale;
				return value.Bits;
			}
		}

		//Beyond these no double is anywhere near
		if (decimal.Point > 310)
		{
			range = true;
			return InfinityBits(single);
		}

		if (decimal.Point < -330)
		{
			range = true;
			return 0;
		}

		//Scale by powers of two into [1/2, 1), counting them in exponent
		int exponent = 0;
		while (decimal.Point > 0)
		{
			const unsigned shift = decimal.Point >= 20 ? 60 : decimal.Point * 3;
			RightShift(decimal, shift);
			exponent += shift;
		}

		while (decimal.Point < 0 || decimal.Digits[0] < 5)
		{
			const unsigned shift = decimal.Point <= -20 ? 60 : decimal.Point < 0 ? -decimal.Point * 3 : 1;
			LeftShift(decimal, shift);
			exponent -= shift;
		}

		//The integer part of the value times 2^64 is then its top 64 bits, and any fraction left is below them
		LeftShift(decimal, 60);
		LeftShift(decimal, 4);
		uint64_t mantissa = 0;
		for (int i = 0; i < decimal.Point; i++)
			mantissa = mantissa * 10 + (i < decimal.Count ? decimal.Digits[i] : 0);

		const bool sticky = decimal.Truncated || decimal.Count > decimal.Point;
		return RoundBits(mantissa, exponent - 64, sticky, single, range);
	}

	//What ScanFloat read, with the value as double (or with single, float) bits
	struct ScannedFloat
	{
		uint64_t Bits;
		size_t Read;
		size_t Length;
		bool Range;
	};

	//Reads at most width characters that begin a number as strtod takes it; Length is how many of them make the longest number, 0 if none
	template<typename Source>
	ScannedFloat ScanFloat(Source& source, const size_t width, const bool single)
	{
		ScannedFloat result = {};
		const auto peek = [&]() { return result.Read < width ? source.Peek() : EOF; };
		const auto next = [&]()
		{
			source.Advance();
			result.Read++;
		};

		//An exponent counts once a digit follows its sign, and saturates long before it could overflow an int
		const auto exponent = [&]()
		{
			next();
			const bool negative = peek() == '-';
			if (negative || peek() == '+')
				next();

			int value = 0;
			while (isdigit(peek()))
			{
				if (value < 100000000)
					value = value * 10 + (peek() - '0');
				next();
				result.Length = result.Read;
			}

			return negative ? -value : value;
		};

		const bool negative = peek() == '-';
		if (negative || peek() == '+')
			next();

		const uint64_t sign = negative ? (uint64_t)1 << (single ? 31 : 63) : 0;

		//Inf, infinity, nan and nan(chars), in any case
		const int first = tolower(peek());
		if (first == 'i' || first == 'n')
		{
			const char* const word = first == 'i' ? "infinity" : "nan";
			size_t matched = 0;
			while (word[matched] != '\0' && tolower(peek()) == word[matched])
			{
				next();
				matched++;
			}

			if (matched < 3)
				return result;

			//Part of infinity is still inf
			result.Length = result.Read - (matched == 8 ? 0 : matched - 3);
			if (first == 'i')
			{
				result.Bits = sign | InfinityBits(single);
				return result;
			}

			result.Bits = sign | (single ? 0x7FC00000 : 0x7FF8000000000000);
			if (peek() == '(')
			{
				next();
				while (isdigit(peek()) || (tolower(peek()) >= 'a' && tolower(peek()) <= 'z') || peek() == '_')
					next();

				if (peek() == ')')
				{
					next();
					result.Length = result.Read;
				}
			}

			return result;
		}

		bool digits = false;
		if (peek() == '0')
		{
			next();
			digits = true;
			result.Length = result.Read;
		}

		//Hex digits past the 16 that fill the mantissa only matter as a bit below it, and without any the number is the 0 before the x
		if (digits && tolower(peek()) == 'x')
		{
			next();
			uint64_t mantissa = 0;
			int scale = 0;
			int stored = 0;
			bool sticky = false;
			bool hex = false;
			bool point = false;
			for (;;)
			{
				const int c = peek();
				const unsigned digit = ScanDigit(c);
				if (digit < 16)
				{
					next();
					hex = true;
					result.Length = result.Read;
					if (mantissa == 0 && digit == 0)
					{
						if (point)
							scale -= 4;
					}
					else if (stored < 16)
					{
						mantissa = mantissa * 16 + digit;
						stored++;
						if (point)
							scale -= 4;
					}
					else
					{
						sticky |= digit != 0;
						if (!point)
							scale += 4;
					}
				}
				else if (c == '.' && !point)
				{
					next();
					point = true;
					if (hex)
						result.Length = result.Read;
				}
				else
				{
					break;
				}
			}

			if (hex && tolower(peek()) == 'p')
				scale += exponent();

			result.Bits = sign | RoundBits(mantissa, scale, sticky, single, result.Range);
			return result;
		}

		Decimal decimal;
		decimal.Count = 0;
		decimal.Point = 0;
		decimal.Truncated = false;
		bool point = false;
		for (;;)
		{
			const int c = peek();
			if (isdigit(c))
			{
				next();
				AppendDigit(decimal, (uint8_t)(c - '0'), point);
				digits = true;
				result.Length = result.Read;
			}
			else if (c == '.' && !point)
			{
				next();
				point = true;
				if (digits)
					result.Length = result.Read;
			}
			else
			{
				break;
			}
		}

		if (!digits)
			return result;

		if (tolower(peek()) == 'e')
			decimal.Point += exponent();

		Trim(decimal);
		result.Bits = sign | DecimalBits(decimal, single, result.Range);
		return result;
	}
}

extern "C"
{
	//The compiler makes code that uses floating point reference this, so apps that do link against it here
	int _fltused = 0;

	double strtod(const char* _String, char** _EndPtr)
	{
		const char* input = _String;
		while (isspace(*input))
			input++;

		StringSource source = { input };
		const ScannedFloat number = ScanFloat(source, (size_t)-1, false);
		if (_EndPtr)
			*_EndPtr = const_cast<char*>(number.Length ? input + number.Length : _String);

		if (number.Range)
			errno = ERANGE;

		return DoubleFromBits(number.Bits);
	}

	float strtof(const char* _String, char** _EndPtr)
	{
		const char* input = _String;
		while (isspace(*input))
			input++;

		StringSource source = { input };
		const ScannedFloat number = ScanFloat(source, (size_t)-1, true);
		if (_EndPtr)
			*_EndPtr = const_cast<char*>(number.Length ? input + number.Length : _String);

		if (number.Range)
			errno = ERANGE;

		return FloatFromBits((uint32_t)number.Bits);
	}

	double atof(const char* _String)
	{
		return strtod(_String, nullptr);
	}
}
