#include "Lib/Crc32.h"

void Crc32_test()
{
	//The standard check value: CRC-32 of "123456789"
	AssertEqual(Crc32::Of("123456789", 9), static_cast<uint32_t>(0xCBF43926));

	//Nothing added is zero
	AssertEqual(Crc32::Of("", 0), static_cast<uint32_t>(0));

	//Added in parts, the same as at once
	Crc32 parts;
	parts.Add("1234", 4);
	parts.Add("56789", 5);
	AssertEqual(parts.Value(), Crc32::Of("123456789", 9));
}
