#pragma once

#include "Lib/Buffer.h"
#include "Net/Net.h"
#include "Assert.h"
#include "core_crt/stdint.h"
#include "core_crt/stddef.h"
#include "core_crt/string.h"

namespace Net
{
	struct Packet
	{
		constexpr Packet() : backing(), src(), dst(), proto(), offset(), count()
		{

		}

		Packet(const Buffer& _backing) : backing(_backing), src(), dst(), proto(), offset(), count(_backing.Capacity)
		{

		}

		Packet(const Buffer& _backing, const size_t _count) : backing(_backing), src(), dst(), proto(), offset(), count(_count)
		{

		}

		uint8_t* buffer()
		{
			return &backing.Data[offset];
		}

		const uint8_t* buffer() const
		{
			return &backing.Data[offset];
		}

		size_t headroom() const
		{
			return offset;
		}

		size_t length() const
		{
			return count;
		}

		void trim(const size_t size)
		{
			count = size;
		}

		void mask(const size_t size)
		{
			AssertOp(count, >, size);
			offset += size;
			count -= size;
		}

		void unmask(const size_t size)
		{
			Assert(offset >= size);
			offset -= size;
			count += size;
		}

#if 0
		void display()
		{
			printf("Count: 0x%lx, offset: 0x%lx\n", count, offset);
			printf("\t: %s(%d): %d.%d.%d.%d:%d -> %d.%d.%d.%d:%d\n",
				ToString(proto),
				(int)proto,
				src.addr.bytes[3],
				src.addr.bytes[2],
				src.addr.bytes[1],
				src.addr.bytes[0],
				src.port,
				dst.addr.bytes[3],
				dst.addr.bytes[2],
				dst.addr.bytes[1],
				dst.addr.bytes[0],
				dst.port
			);
		}
#endif

		Buffer backing;

		//Layer specific data
		endpoint_t src;
		endpoint_t dst;
		proto_t proto;

	private:
		//Packet data
		size_t offset;
		size_t count;
	};
}
