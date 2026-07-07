#include "Net/Net.h"

namespace Net
{
	//TODO(tsharpe): Implement rand for this method
	uint16_t rand_port()
	{
		static uint16_t port = 1024;
		port++;
		return (port % UINT16_MAX);
	}
}
