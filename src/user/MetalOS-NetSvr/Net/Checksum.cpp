#include "Net/Net.h"

namespace Net
{
	// From: https://tools.ietf.org/html/rfc1071
	uint16_t checksum(const void* const buffer, const size_t length)
	{
		return checksum_finalize(checksum_add(0, buffer, length));
	}

	uint32_t checksum_add(const uint32_t current, const void* const buffer, const size_t length)
	{
		uint32_t sum = current;

		const uint8_t* bytes = static_cast<const uint8_t*>(buffer);
		size_t count = length;
		while (count > 1)
		{
			sum += static_cast<uint32_t>((bytes[0] << 8) | bytes[1]);
			bytes += 2;
			count -= 2;
		}

		//A left-over byte is the high half of a word padded with zero
		if (count > 0)
			sum += static_cast<uint32_t>(bytes[0]) << 8;

		return sum;
	}

	uint16_t checksum_finalize(const uint32_t current)
	{
		uint32_t sum = current;
		while (sum >> 16)
			sum = (sum & 0xffff) + (sum >> 16);

		return static_cast<uint16_t>(~sum);
	}

	bool checksum_valid(const void* const buffer, const size_t length)
	{
		return checksum(buffer, length) == 0;
	}
}
