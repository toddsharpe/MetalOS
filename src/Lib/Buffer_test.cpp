#include "Lib/Buffer.h"

void Buffer_test()
{
	uint8_t data[16];
	Buffer storage(data);

	StaticBuffer<32> storage2;
}
