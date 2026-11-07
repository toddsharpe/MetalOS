#include "Lib/Serializer.h"
#include "Lib/Deserializer.h"

void Serializer_test()
{
	//A write that ends exactly at the buffer's end fits, in network order
	{
		uint8_t bytes[2] = {};
		Serializer ser(bytes);
		ser.Write<uint16_t>(0x0102);
		AssertEqual(ser.Bytes(), sizeof(bytes));
		AssertEqual(bytes[0], static_cast<uint8_t>(0x01));
		AssertEqual(bytes[1], static_cast<uint8_t>(0x02));
	}

	//Mixed writes fill a buffer to the last byte, as a UDP header with no payload does
	{
		uint8_t bytes[8] = {};
		Serializer ser(bytes);
		ser.Write<uint16_t>(1024);
		ser.Write<uint16_t>(53);
		ser.Write<uint16_t>(8);
		ser.Write<uint16_t>(0);
		AssertEqual(ser.Bytes(), sizeof(bytes));
	}

	//Byte arrays and raw bytes fit exactly too
	{
		uint8_t bytes[9] = {};
		Serializer ser(bytes);
		const uint8_t mac[6] = { 1, 2, 3, 4, 5, 6 };
		ser.Write(mac);
		const uint8_t raw[3] = { 7, 8, 9 };
		ser.WriteBytes(raw, sizeof(raw));
		AssertEqual(ser.Bytes(), sizeof(bytes));
		AssertEqual(bytes[0], static_cast<uint8_t>(6));
		AssertEqual(bytes[8], static_cast<uint8_t>(9));
	}

	//Reads take a buffer to its last byte, and nothing remains
	{
		const uint8_t bytes[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07 };
		Deserializer deser(bytes, sizeof(bytes));
		AssertEqual(deser.ReadU8(), static_cast<uint8_t>(0x01));
		AssertEqual(deser.ReadU16(), static_cast<uint16_t>(0x0203));
		AssertEqual(deser.ReadU32(), static_cast<uint32_t>(0x04050607));
		AssertEqual(deser.Remaining(), static_cast<size_t>(0));
	}

	//A read that is the whole buffer fits
	{
		const uint8_t bytes[] = { 0xAA, 0xBB, 0xCC };
		Deserializer deser(bytes, sizeof(bytes));
		uint8_t out[3] = {};
		deser.ReadBytes(out, sizeof(out));
		AssertEqual(out[2], static_cast<uint8_t>(0xCC));
		AssertEqual(deser.Remaining(), static_cast<size_t>(0));
	}
}
