#include "Net/Net.h"
#include "core_crt/stdint.h"

namespace Net
{
	// From: https://tools.ietf.org/html/rfc1071
	uint16_t checksum(const void* const buffer, const size_t length)
	{
		const uint32_t current = checksum_add(0, buffer, length);
		return checksum_finalize(current);
	}

	uint32_t checksum_add(const uint32_t current, const void* const buffer, const size_t length)
	{
		uint32_t sum = current;

		const uint16_t* ptr = reinterpret_cast<const uint16_t*>(buffer);
		size_t count = length;

		while (count > 1) {
			/*  This is the inner loop */
			sum += htons(*ptr++);
			count -= 2;
		}

		/*  Add left-over byte, if any */
		if (count > 0)
			sum += *(uint8_t*)ptr;

		return sum;
	}

	uint16_t checksum_finalize(const uint32_t current)
	{
		uint32_t sum = current;

		/*  Fold 32-bit sum to 16 bits */
		while (sum >> 16)
			sum = (sum & 0xffff) + (sum >> 16);

		return static_cast<uint16_t>(~sum);
	}

	bool checksum_valid(const void* const buffer, const size_t length)
	{
		const uint16_t check = checksum(buffer, length);
		return check == 0;
	}
}
