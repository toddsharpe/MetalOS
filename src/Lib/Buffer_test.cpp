#include "Lib/Buffer.h"

void Buffer_test()
{
	uint8_t data[16];
	Buffer storage(data);

	StaticBuffer<32> storage2;

	//Copy writes the source's bytes and size, whatever the destination held before
	{
		const uint8_t source[] = { 1, 2, 3, 4, 5 };
		StaticBuffer<16> to;
		Copy(to, CBuffer(source));
		AssertEqual(to.Count, sizeof(source));
		Assert(to.Ref() == CBuffer(source));

		const uint8_t shorter[] = { 9, 8 };
		Copy(to, CBuffer(shorter));
		AssertEqual(to.Count, sizeof(shorter));
		Assert(to.Ref() == CBuffer(shorter));
		AssertEqual(to.Data[2], 3);
	}
}
