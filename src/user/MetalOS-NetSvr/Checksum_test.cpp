//The internet checksum against known answers
#include "Net/Net.h"

void Checksum_test()
{
	//RFC 1071's worked example sums to 0xDDF2, so its checksum is the complement
	{
		const uint8_t bytes[] = { 0x00, 0x01, 0xF2, 0x03, 0xF4, 0xF5, 0xF6, 0xF7 };
		AssertEqual(Net::checksum(bytes, sizeof(bytes)), static_cast<uint16_t>(0x220D));
	}

	//A left-over byte is the high half of its word
	{
		const uint8_t one[] = { 0x01 };
		AssertEqual(Net::checksum(one, sizeof(one)), static_cast<uint16_t>(0xFEFF));
		const uint8_t three[] = { 0x00, 0x01, 0x02 };
		AssertEqual(Net::checksum(three, sizeof(three)), static_cast<uint16_t>(0xFDFE));
	}

	//Summed in parts, the same as at once
	{
		const uint8_t bytes[] = { 0x45, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00 };
		const uint32_t parts = Net::checksum_add(Net::checksum_add(0, bytes, 4), bytes + 4, 4);
		AssertEqual(Net::checksum_finalize(parts), Net::checksum(bytes, sizeof(bytes)));
	}

	//A header with its checksum in it is valid, and one changed byte isn't (Wireshark: 0x66BC, correct)
	{
		uint8_t header[] = { 0x45, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x40, 0x11, 0x66, 0xBC, 0x0A, 0x08, 0x00, 0x01, 0x0A, 0x08, 0x00, 0x02 };
		Assert(Net::checksum_valid(header, sizeof(header)));
		header[8] = 0x3F;
		Assert(!Net::checksum_valid(header, sizeof(header)));
	}
}
